#include "Juego.h"

#include "Configuracion.h"
#include "SistemaColisiones.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>

//std::min() recibe dos valores y devuelve el menor.

namespace {
constexpr float VELOCIDAD_PROYECTIL = 800.0f;
constexpr float ALCANCE_ARMA = 2210.0f;
constexpr float ANCHO_PROYECTIL_PISTOLA = 28.0f;
constexpr float ALTO_PROYECTIL_PISTOLA = 14.0f;
constexpr float ANCHO_PROYECTIL_METRALLETA = 14.0f;
constexpr float ALTO_PROYECTIL_METRALLETA = 6.0f;
const sf::Color COLOR_PROYECTIL_PISTOLA(255, 225, 70);
const sf::Color COLOR_PROYECTIL_METRALLETA(255, 140, 50);
constexpr float SEGUNDOS_REAPARICION = 3.0f;
constexpr float SEGUNDOS_ENTRE_DANOS_CONTACTO = 0.75f;
constexpr float DURACION_INDICADOR_DANIO = 0.8f;
constexpr float VELOCIDAD_INDICADOR_DANIO = 45.0f;
constexpr float RADIO_RECOLECCION = 90.0f;
constexpr float TAMANIO_RECURSO = 16.0f;
constexpr float SEPARACION_RECURSOS = 12.0f;
constexpr float DURACION_INDICADOR_RECURSO = 1.0f;
constexpr float VELOCIDAD_INDICADOR_RECURSO = 40.0f;
constexpr std::array<Posicion, 3> POSICIONES_INICIALES_ENEMIGOS{{
    {1440.0f, 200.0f},
    {1440.0f, 512.0f},
    {1440.0f, 824.0f}
}};
constexpr std::array<TipoEnemigo, 3> TIPOS_ENEMIGOS{{
    TipoEnemigo::BASICO,
    TipoEnemigo::RAPIDO,
    TipoEnemigo::RESISTENTE
}};

Enemigo crearEnemigoParaIndice(std::size_t indice) {
    const Posicion posicion = POSICIONES_INICIALES_ENEMIGOS[indice];
    return crearEnemigo(
        TIPOS_ENEMIGOS[indice], posicion.x, posicion.y);
}

sf::Color obtenerColorEnemigo(TipoEnemigo tipo) {
    switch (tipo) {
        case TipoEnemigo::RAPIDO:
            return sf::Color(235, 190, 60);

        case TipoEnemigo::RESISTENTE:
            return sf::Color(130, 115, 220);

        case TipoEnemigo::BASICO:
        default:
            return sf::Color(205, 70, 70);
    }
}

void configurarFiguraEnemigo(
        sf::RectangleShape& figura, const Enemigo& enemigo,
        TipoEnemigo tipo) {
    const FormaColision forma = enemigo.obtenerFormaColision();
    figura.setSize({forma.ancho, forma.alto});
    figura.setPosition({forma.x, forma.y});
    figura.setFillColor(obtenerColorEnemigo(tipo));
}

bool cargarFuenteDanio(sf::Font& fuente) {
    constexpr std::array<const char*, 2> RUTAS_FUENTE{{
        "recursos/Tuffy.ttf",
        "../../recursos/Tuffy.ttf"
    }};

    for (const char* ruta : RUTAS_FUENTE) {
        std::ifstream archivo(ruta, std::ios::binary);
        if (archivo.good() && fuente.openFromFile(ruta)) {
            return true;
        }
    }

    return false;
}
}

Juego::Juego()
    : ventana(
          sf::VideoMode({Configuracion::ANCHO_VENTANA, Configuracion::ALTO_VENTANA}),
          "WASD: mover | Disparo automatico | K: dano | E: experiencia"),
      enemigosPrueba{
          crearEnemigoParaIndice(0),
          crearEnemigoParaIndice(1),
          crearEnemigoParaIndice(2)
      },
      figurasEnemigosPrueba{
          sf::RectangleShape({0.0f, 0.0f}),
          sf::RectangleShape({0.0f, 0.0f}),
          sf::RectangleShape({0.0f, 0.0f})
      },
      tiemposReaparicion{},
      figuraProyectilPrueba({28.0f, 14.0f}),
      _proyectiles{},
      armaPrueba(1, "Pistola basica", 25, 2.0f, ALCANCE_ARMA, 5),
      _metralletaPrueba(2, "Metralleta", 6, 8.0f, ALCANCE_ARMA, 5),
      _armaSeleccionada(TipoArmaPrueba::Pistola),
      fuenteDanioDisponible(cargarFuenteDanio(fuenteDanio)),
      indicadoresDanio{},
      recursosSuelo{},
      indicadoresRecurso{},
      tiempoRestanteDanoContacto(0.0f) {
    ventana.setFramerateLimit(Configuracion::LIMITE_FPS);

    figuraProyectilPrueba.setFillColor(sf::Color(255, 225, 70));
    reiniciarPrueba();
}

void Juego::ejecutar() {
    while (ventana.isOpen()) {
        procesarEventos();
        actualizar();
        dibujar();
    }
}

void Juego::procesarEventos() {
    while (const auto evento = ventana.pollEvent()) {
        if (evento->is<sf::Event::Closed>()) {
            ventana.close();
        }

        if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
            if (tecla->scancode == sf::Keyboard::Scancode::Escape) {
                ventana.close();
            } else if (tecla->scancode == sf::Keyboard::Scancode::R) {
                reiniciarPrueba();
            } else if (tecla->scancode == sf::Keyboard::Scancode::K) {
                jugador.recibirDano(20);
            } else if (tecla->scancode == sf::Keyboard::Scancode::E) {
                jugador.ganarExperiencia(40);
            }
            else if (tecla->scancode == sf::Keyboard::Scancode::Num1 ||
                     tecla->scancode == sf::Keyboard::Scancode::Numpad1)
            {
                seleccionarArmaPrueba(TipoArmaPrueba::Pistola);
            }
            else if (tecla->scancode == sf::Keyboard::Scancode::Num2 ||
                     tecla->scancode == sf::Keyboard::Scancode::Numpad2)
            {
                seleccionarArmaPrueba(TipoArmaPrueba::Metralleta);
            }
        }
    }
}

void Juego::actualizar() {
    // Limitar el salto reduce el riesgo de atravesar al enemigo si la ventana se pausa.
    const float segundos = std::min(relojFotogramas.restart().asSeconds(), 0.05f);
    actualizarArmas(segundos);
    jugador.procesarEntrada();
    jugador.actualizar();
    actualizarEnemigos(segundos);
    actualizarDanoContacto(segundos);
    actualizarIndicadoresDanio(segundos);
    actualizarProyectiles(segundos);
    actualizarRecursos(segundos);
    actualizarDisparoAutomatico();
}

void Juego::actualizarEnemigos(float segundos) {
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (!enemigosPrueba[i].estaVivo()) {
            tiemposReaparicion[i] =
                std::max(0.0f, tiemposReaparicion[i] - segundos);
            if (tiemposReaparicion[i] <= 0.0f) {
                enemigosPrueba[i] = crearEnemigoParaIndice(i);
                configurarFiguraEnemigo(
                    figurasEnemigosPrueba[i], enemigosPrueba[i],
                    TIPOS_ENEMIGOS[i]);
            }
            continue;
        }

        const Posicion posicionJugador = jugador.getPosicion();
        const FormaColision formaEnemigo =
            enemigosPrueba[i].obtenerFormaColision();
        const Posicion objetivoEnemigo{
            posicionJugador.x - formaEnemigo.ancho / 2.0f,
            posicionJugador.y - formaEnemigo.alto / 2.0f
        };
        enemigosPrueba[i].actualizar(objetivoEnemigo, segundos);

        const Posicion posicionEnemigo = enemigosPrueba[i].obtenerPosicion();
        figurasEnemigosPrueba[i].setPosition(
            {posicionEnemigo.x, posicionEnemigo.y});
    }
}

void Juego::actualizarDanoContacto(float segundos) {
    tiempoRestanteDanoContacto =
        std::max(0.0f, tiempoRestanteDanoContacto - segundos);

    if (!jugador.estaVivo() || tiempoRestanteDanoContacto > 0.0f) {
        return;
    }

    const FormaColision formaJugador = jugador.getFormaColision();
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (!enemigosPrueba[i].estaVivo() ||
            !SistemaColisiones::hayInterseccion(
                formaJugador, enemigosPrueba[i].obtenerFormaColision())) {
            continue;
        }

        const int vidaAnterior = jugador.getVida();
        jugador.recibirDano(enemigosPrueba[i].obtenerDano());
        const int danoRecibido = vidaAnterior - jugador.getVida();
        if (danoRecibido > 0) {
            indicadoresDanio.push_back({
                danoRecibido,
                {formaJugador.x + formaJugador.ancho / 2.0f,
                 formaJugador.y - 12.0f},
                DURACION_INDICADOR_DANIO
            });
            tiempoRestanteDanoContacto = SEGUNDOS_ENTRE_DANOS_CONTACTO;
        }
        break;
    }
}

void Juego::actualizarIndicadoresDanio(float segundos) {
    for (IndicadorDanio& indicador : indicadoresDanio) {
        indicador.posicion.y -= VELOCIDAD_INDICADOR_DANIO * segundos;
        indicador.tiempoRestante -= segundos;
    }

    indicadoresDanio.erase(
        std::remove_if(
            indicadoresDanio.begin(), indicadoresDanio.end(),
            [](const IndicadorDanio& indicador) {
                return indicador.tiempoRestante <= 0.0f;
            }),
        indicadoresDanio.end());
}

void Juego::actualizarRecursos(float segundos) {
    for (IndicadorRecurso& indicador : indicadoresRecurso) {
        indicador.posicion.y -= VELOCIDAD_INDICADOR_RECURSO * segundos;
        indicador.tiempoRestante -= segundos;
    }

    indicadoresRecurso.erase(
        std::remove_if(
            indicadoresRecurso.begin(), indicadoresRecurso.end(),
            [](const IndicadorRecurso& indicador) {
                return indicador.tiempoRestante <= 0.0f;
            }),
        indicadoresRecurso.end());

    if (!jugador.estaVivo()) {
        return;
    }

    const Posicion posicionJugador = jugador.getPosicion();
    auto recurso = recursosSuelo.begin();
    while (recurso != recursosSuelo.end()) {
        if (!SistemaColisiones::estaDentroDeRadio(
                posicionJugador, recurso->posicion, RADIO_RECOLECCION)) {
            ++recurso;
            continue;
        }

        if (recurso->tipo == TipoRecurso::Experiencia) {
            jugador.ganarExperiencia(recurso->cantidad);
        } else {
            jugador.ganarChatarra(recurso->cantidad);
        }

        indicadoresRecurso.push_back({
            recurso->tipo,
            recurso->cantidad,
            {posicionJugador.x + 24.0f, posicionJugador.y - 38.0f},
            DURACION_INDICADOR_RECURSO
        });
        recurso = recursosSuelo.erase(recurso);
    }
}

void Juego::actualizarProyectiles(float segundos)
{
    const FormaColision escenario{
        0.0f, 0.0f,
        static_cast<float>(Configuracion::ANCHO_VENTANA),
        static_cast<float>(Configuracion::ALTO_VENTANA)
    };

    for (Proyectil& proyectil : _proyectiles)
    {
        if (!proyectil.estaActivo())
        {
            continue;
        }

        proyectil.actualizar(segundos);
        for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++)
        {
            if (enemigosPrueba[i].estaVivo() &&
                SistemaColisiones::hayInterseccion(
                    proyectil.obtenerFormaColision(),
                    enemigosPrueba[i].obtenerFormaColision()))
            {
                enemigosPrueba[i].recibirDanio(proyectil.obtenerDano());
                proyectil.desactivar();
                if (!enemigosPrueba[i].estaVivo())
                {
                    generarRecompensa(enemigosPrueba[i]);
                    tiemposReaparicion[i] = SEGUNDOS_REAPARICION;
                }
                break;
            }
        }

        if (proyectil.estaActivo() &&
            !SistemaColisiones::estaDentroDe(
                proyectil.obtenerFormaColision(), escenario))
        {
            proyectil.desactivar();
        }
    }

    _proyectiles.erase(
        std::remove_if(
            _proyectiles.begin(), _proyectiles.end(),
            [](const Proyectil& proyectil)
            {
                return !proyectil.estaActivo();
            }),
        _proyectiles.end());
}

void Juego::dibujar() {
    ventana.clear(sf::Color(28, 31, 36));
    dibujarRecursos();
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (enemigosPrueba[i].estaVivo()) {
            ventana.draw(figurasEnemigosPrueba[i]);
        }
    }
    jugador.dibujar(ventana);
    for (const Proyectil& proyectil : _proyectiles)
    {
        if (proyectil.estaActivo())
        {
            const FormaColision forma = proyectil.obtenerFormaColision();
            figuraProyectilPrueba.setSize({forma.ancho, forma.alto});
            figuraProyectilPrueba.setPosition({forma.x, forma.y});
            // Los dos formatos de prueba conservan su apariencia al cambiar de arma.
            const bool esMetralleta =
                forma.ancho == ANCHO_PROYECTIL_METRALLETA &&
                forma.alto == ALTO_PROYECTIL_METRALLETA;
            figuraProyectilPrueba.setFillColor(
                esMetralleta ? COLOR_PROYECTIL_METRALLETA : COLOR_PROYECTIL_PISTOLA);
            ventana.draw(figuraProyectilPrueba);
        }
    }
    dibujarIndicadoresDanio();
    dibujarIndicadoresRecurso();
    dibujarInterfaz();
    ventana.display();
}

void Juego::dibujarRecursos() {
    for (const RecursoSuelo& recurso : recursosSuelo) {
        sf::RectangleShape figura({TAMANIO_RECURSO, TAMANIO_RECURSO});
        figura.setOrigin({TAMANIO_RECURSO / 2.0f, TAMANIO_RECURSO / 2.0f});
        figura.setPosition({recurso.posicion.x, recurso.posicion.y});
        if (recurso.tipo == TipoRecurso::Experiencia) {
            figura.setFillColor(sf::Color(70, 220, 255));
        } else {
            figura.setFillColor(sf::Color(255, 190, 55));
        }
        figura.setOutlineColor(sf::Color(240, 240, 240));
        figura.setOutlineThickness(1.0f);
        ventana.draw(figura);
    }
}

void Juego::dibujarIndicadoresDanio() {
    if (!fuenteDanioDisponible) {
        return;
    }

    for (const IndicadorDanio& indicador : indicadoresDanio) {
        const float proporcion =
            std::clamp(indicador.tiempoRestante / DURACION_INDICADOR_DANIO,
                       0.0f, 1.0f);
        const auto transparencia =
            static_cast<std::uint8_t>(255.0f * proporcion);
        sf::Text texto(
            fuenteDanio, "-" + std::to_string(indicador.cantidad), 22);
        texto.setFillColor(sf::Color(255, 95, 95, transparencia));
        texto.setOutlineColor(sf::Color(45, 12, 12, transparencia));
        texto.setOutlineThickness(2.0f);
        texto.setPosition({indicador.posicion.x, indicador.posicion.y});
        ventana.draw(texto);
    }
}

void Juego::dibujarIndicadoresRecurso() {
    if (!fuenteDanioDisponible) {
        return;
    }

    for (const IndicadorRecurso& indicador : indicadoresRecurso) {
        const float proporcion =
            std::clamp(indicador.tiempoRestante /
                           DURACION_INDICADOR_RECURSO,
                       0.0f, 1.0f);
        const auto transparencia =
            static_cast<std::uint8_t>(255.0f * proporcion);
        const std::string etiqueta =
            indicador.tipo == TipoRecurso::Experiencia
                ? " EXP"
                : " CHATARRA";
        sf::Text texto(
            fuenteDanio,
            "+" + std::to_string(indicador.cantidad) + etiqueta,
            20);
        if (indicador.tipo == TipoRecurso::Experiencia) {
            texto.setFillColor(sf::Color(70, 220, 255, transparencia));
        } else {
            texto.setFillColor(sf::Color(255, 190, 55, transparencia));
        }
        texto.setOutlineColor(sf::Color(20, 20, 20, transparencia));
        texto.setOutlineThickness(2.0f);
        texto.setPosition({indicador.posicion.x, indicador.posicion.y});
        ventana.draw(texto);
    }
}

void Juego::dibujarInterfaz() {
    if (!fuenteDanioDisponible) {
        return;
    }

    sf::RectangleShape fondo({850.0f, 44.0f});
    fondo.setPosition({14.0f, 14.0f});
    fondo.setFillColor(sf::Color(8, 10, 14, 190));
    fondo.setOutlineColor(sf::Color(110, 120, 135));
    fondo.setOutlineThickness(1.0f);
    ventana.draw(fondo);

    sf::Text nivel(
        fuenteDanio, "NIVEL: " + std::to_string(jugador.getNivel()), 22);
    nivel.setPosition({26.0f, 22.0f});
    nivel.setFillColor(sf::Color(235, 235, 235));
    ventana.draw(nivel);

    sf::Text experiencia(
        fuenteDanio, "EXP: " + std::to_string(jugador.getExperiencia()), 22);
    experiencia.setPosition({170.0f, 22.0f});
    experiencia.setFillColor(sf::Color(70, 220, 255));
    ventana.draw(experiencia);

    sf::Text chatarra(
        fuenteDanio, "CHATARRA: " + std::to_string(jugador.getChatarra()), 22);
    chatarra.setPosition({330.0f, 22.0f});
    chatarra.setFillColor(sf::Color(255, 190, 55));
    ventana.draw(chatarra);

    sf::Text nombreArma(
        fuenteDanio,
        "ARMA: " + std::string(obtenerArmaSeleccionada().obtenerNombre()), 22);
    nombreArma.setPosition({580.0f, 22.0f});
    nombreArma.setFillColor(sf::Color(235, 235, 235));
    ventana.draw(nombreArma);
}

void Juego::generarRecompensa(const Enemigo& enemigo) {
    const Recompensa recompensa = enemigo.obtenerRecompensa();
    const FormaColision forma = enemigo.obtenerFormaColision();
    const Posicion centro{
        forma.x + forma.ancho / 2.0f,
        forma.y + forma.alto / 2.0f
    };

    if (recompensa.experiencia > 0) {
        recursosSuelo.push_back({
            TipoRecurso::Experiencia,
            recompensa.experiencia,
            {centro.x - SEPARACION_RECURSOS, centro.y}
        });
    }
    if (recompensa.chatarra > 0) {
        recursosSuelo.push_back({
            TipoRecurso::Chatarra,
            recompensa.chatarra,
            {centro.x + SEPARACION_RECURSOS, centro.y}
        });
    }
}

void Juego::actualizarDisparoAutomatico()
{
    Arma& arma = obtenerArmaSeleccionada();
    if (!jugador.estaVivo() || !arma.puedeDisparar())
    {
        return;
    }

    const FormaColision formaJugador = jugador.getFormaColision();
    const Posicion centroJugador{
        formaJugador.x + formaJugador.ancho / 2.0f,
        formaJugador.y + formaJugador.alto / 2.0f
    };

    int indiceObjetivo = -1;
    float distanciaMinima = std::numeric_limits<float>::max();
    Posicion centroObjetivo{0.0f, 0.0f};
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (!enemigosPrueba[i].estaVivo()) {
            continue;
        }

        const FormaColision formaEnemigo =
            enemigosPrueba[i].obtenerFormaColision();
        const Posicion centroEnemigo{
            formaEnemigo.x + formaEnemigo.ancho / 2.0f,
            formaEnemigo.y + formaEnemigo.alto / 2.0f
        };
        const float diferenciaX = centroEnemigo.x - centroJugador.x;
        const float diferenciaY = centroEnemigo.y - centroJugador.y;
        const float distanciaCuadrada =
            diferenciaX * diferenciaX + diferenciaY * diferenciaY;
        if (distanciaCuadrada < distanciaMinima) {
            distanciaMinima = distanciaCuadrada;
            indiceObjetivo = static_cast<int>(i);
            centroObjetivo = centroEnemigo;
        }
    }

    if (indiceObjetivo < 0) {
        return;
    }

    Proyectil nuevoProyectil({0.0f, 0.0f}, 0.0f, 0.0f, 0.0f, 0.0f, 0);
    const bool esMetralleta = _armaSeleccionada == TipoArmaPrueba::Metralleta;
    const float ancho = esMetralleta ? ANCHO_PROYECTIL_METRALLETA : ANCHO_PROYECTIL_PISTOLA;
    const float alto = esMetralleta ? ALTO_PROYECTIL_METRALLETA : ALTO_PROYECTIL_PISTOLA;
    if (arma.intentarDisparar(
            nuevoProyectil, centroJugador, centroObjetivo, VELOCIDAD_PROYECTIL,
            ancho, alto))
    {
        _proyectiles.push_back(nuevoProyectil);
    }
}

void Juego::actualizarArmas(float segundos)
{
    armaPrueba.actualizar(segundos);
    _metralletaPrueba.actualizar(segundos);
}

void Juego::seleccionarArmaPrueba(TipoArmaPrueba tipo)
{
    _armaSeleccionada = tipo;
}

Arma& Juego::obtenerArmaSeleccionada()
{
    if (_armaSeleccionada == TipoArmaPrueba::Metralleta)
    {
        return _metralletaPrueba;
    }
    return armaPrueba;
}

void Juego::reiniciarPrueba() {
    jugador.reiniciar();
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        enemigosPrueba[i] = crearEnemigoParaIndice(i);
        configurarFiguraEnemigo(
            figurasEnemigosPrueba[i], enemigosPrueba[i],
            TIPOS_ENEMIGOS[i]);
        tiemposReaparicion[i] = 0.0f;
    }
    _proyectiles.clear();
    armaPrueba.reiniciar();
    _metralletaPrueba.reiniciar();
    seleccionarArmaPrueba(TipoArmaPrueba::Pistola);
    indicadoresDanio.clear();
    recursosSuelo.clear();
    indicadoresRecurso.clear();
    tiempoRestanteDanoContacto = 0.0f;
    ventana.setTitle(
        "WASD: mover | PRUEBA: 1 pistola / 2 metralleta | R: reiniciar | K: dano | E: experiencia");
    relojFotogramas.restart();
}
