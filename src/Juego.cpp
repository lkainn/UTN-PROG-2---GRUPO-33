#include "Juego.h"

#include "Configuracion.h"
#include "SistemaColisiones.h"
#include "ArchivoJugadores.h"
#include "ArchivoMejoras.h"
#include "ArchivoPartidas.h"
#include "ArchivoPartidaEnCurso.h"
#include "CarpetaDatos.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <process.h>
#include <string>
#include <cmath>

//std::min() recibe dos valores y devuelve el menor.

namespace {
constexpr const char* ARCHIVO_PARTIDA_EN_CURSO = "PartidaEnCurso.dat";
constexpr std::uint32_t FIRMA_PARTIDA = 0x54504953u;
constexpr std::uint32_t VERSION_PARTIDA = 1;
constexpr float VELOCIDAD_PROYECTIL = 800.0f;
constexpr float ANCHO_PROYECTIL_PISTOLA = 28.0f;
constexpr float ALTO_PROYECTIL_PISTOLA = 14.0f;
constexpr float ANCHO_PROYECTIL_METRALLETA = 14.0f;
constexpr float ALTO_PROYECTIL_METRALLETA = 6.0f;
constexpr float DISPERSION_ESCOPETA = 0.6981317f;
constexpr float PASO_ENERGIA = 8.0f;
constexpr float SEGUNDOS_ENTRE_DANOS_CONTACTO = 0.75f;
constexpr float DURACION_INDICADOR_DANIO = 0.8f;
constexpr float VELOCIDAD_INDICADOR_DANIO = 45.0f;
constexpr float RADIO_RECOLECCION = 90.0f;
constexpr float TAMANIO_RECURSO = 16.0f;
constexpr float SEPARACION_RECURSOS = 12.0f;
constexpr float DURACION_INDICADOR_RECURSO = 1.0f;
constexpr float VELOCIDAD_INDICADOR_RECURSO = 40.0f;
sf::Color obtenerColorRareza(RarezaMejora rareza)
{
    switch (rareza)
    {
        case RarezaMejora::Rara:
            return sf::Color(85, 170, 255);
        case RarezaMejora::Epica:
            return sf::Color(195, 105, 255);
        case RarezaMejora::Legendaria:
            return sf::Color(255, 190, 55);
        case RarezaMejora::Comun:
        default:
            return sf::Color(205, 215, 225);
    }
}
sf::Color obtenerColorEnemigo(int idEnemigo)
{
    switch (idEnemigo)
    {
        case 2:
            return sf::Color(235, 190, 60);

        case 3:
            return sf::Color(130, 115, 220);

        case 4:
            return sf::Color(255, 125, 25);

        case 1:
        default:
            return sf::Color(205, 70, 70);
    }
}

void configurarFiguraEnemigo(
        sf::RectangleShape& figura, const Enemigo& enemigo,
        int idEnemigo)
{
    const FormaColision forma = enemigo.obtenerFormaColision();
    figura.setSize({forma.ancho, forma.alto});
    figura.setPosition({forma.x, forma.y});
    figura.setFillColor(obtenerColorEnemigo(idEnemigo));
}

Arma crearArma(const RegistroArma& registro, int cantidadProyectiles = 1,
               float dispersion = 0.0f, int maximoImpactos = 1)
{
    return Arma(registro.idArma, registro.nombre, registro.dano, registro.velocidadAtaque,
                registro.alcance, registro.nivelMaximo, cantidadProyectiles, dispersion,
                maximoImpactos);
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
    : Juego(ArchivoArmas{}.leerCatalogo())
{
}

Juego::Juego(const ArchivoArmas::Catalogo& catalogoArmas)
    : ventana(
          sf::VideoMode({Configuracion::ANCHO_VENTANA, Configuracion::ALTO_VENTANA}),
          U"El \u00DAltimo Recolector"),
      _escenario{},
      _inventarioArmas(std::array<bool, 4>{{true, catalogoArmas[1].eliminado == 0,
                        catalogoArmas[2].eliminado == 0, catalogoArmas[3].eliminado == 0}}),
      _gestorOleadas{},
      _aspectoProyectiles{},
      _proyectiles{},
      armaPrueba(crearArma(catalogoArmas[0])),
      _metralletaPrueba(crearArma(catalogoArmas[1])),
      _escopetaPrueba(crearArma(catalogoArmas[2], 5, DISPERSION_ESCOPETA)),
      _canonEnergiaPrueba(crearArma(catalogoArmas[3], 1, 0.0f, 3)),
      _armaSeleccionada(TipoArmaPrueba::Pistola),
      fuenteDanioDisponible(cargarFuenteDanio(fuenteDanio)),
      indicadoresDanio{},
      recursosSuelo{},
      indicadoresRecurso{},
      tiempoRestanteDanoContacto(0.0f) {
    ventana.setFramerateLimit(Configuracion::LIMITE_FPS);
    ventana.setKeyRepeatEnabled(false);

    reiniciarPrueba();
    std::error_code error;
    _continuacionDisponible = std::filesystem::is_regular_file(
        obtenerRutaDatos(ARCHIVO_PARTIDA_EN_CURSO), error);
}

void Juego::ejecutar() {
    while (ventana.isOpen()) {
        procesarEventos();
        if (!ventana.isOpen())
        {
            break;
        }
        actualizar();
        dibujar();
    }
}

void Juego::procesarEventos() {
    while (const auto evento = ventana.pollEvent()) {
        if (evento->is<sf::Event::Closed>()) {
            salirDelJuego();
            continue;
        }

        if (const auto* clic = evento->getIf<sf::Event::MouseButtonPressed>())
        {
            if (clic->button == sf::Mouse::Button::Left)
            {
                procesarClicResultado(clic->position);
            }
        }

        if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
            if (_estado != EstadoJuego::Jugando)
            {
                procesarEntradaMenu(tecla->scancode);
                continue;
            }
            if (tecla->scancode == sf::Keyboard::Scancode::Escape) {
                manejarEscape();
            } else if (tecla->scancode == sf::Keyboard::Scancode::R) {
                reiniciarPrueba();
            } else if (_estacionMejoras.menuAbierto()) {
                if (_opcionReemplazo < 3)
                {
                    if (tecla->scancode == sf::Keyboard::Scancode::Num1 ||
                        tecla->scancode == sf::Keyboard::Scancode::Numpad1)
                    {
                        confirmarReemplazoPistola();
                    }
                    else if (tecla->scancode == sf::Keyboard::Scancode::Num2 ||
                             tecla->scancode == sf::Keyboard::Scancode::Numpad2)
                    {
                        _opcionReemplazo = 3;
                    }
                }
                else if (tecla->scancode == sf::Keyboard::Scancode::Num1 ||
                    tecla->scancode == sf::Keyboard::Scancode::Numpad1)
                {
                    elegirMejoraEstacion(0);
                }
                else if (tecla->scancode == sf::Keyboard::Scancode::Num2 ||
                         tecla->scancode == sf::Keyboard::Scancode::Numpad2)
                {
                    elegirMejoraEstacion(1);
                }
                else if (tecla->scancode == sf::Keyboard::Scancode::Num3 ||
                         tecla->scancode == sf::Keyboard::Scancode::Numpad3)
                {
                    elegirMejoraEstacion(2);
                }
            } else if (tecla->scancode == sf::Keyboard::Scancode::F) {
                interactuarEncuentro();
            } else if (tecla->scancode == sf::Keyboard::Scancode::K) {
                jugador.recibirDano(20);
            } else if (tecla->scancode == sf::Keyboard::Scancode::E) {
                jugador.ganarExperiencia(40);
            } else if (tecla->scancode == sf::Keyboard::Scancode::U) {
                ventana.setTitle(comprarMejoraPermanente() ?
                    "MEJORA PERMANENTE: Vida maxima aumentada" : "MEJORA NO DISPONIBLE");
            }
        }
    }
}

void Juego::actualizar() {
    if (_estado != EstadoJuego::Jugando)
    {
        relojFotogramas.restart();
        return;
    }
    // Limitar el salto reduce el riesgo de atravesar al enemigo si la ventana se pausa.
    const float segundos = std::min(relojFotogramas.restart().asSeconds(), 0.05f);
    if (_estacionMejoras.menuAbierto())
    {
        return;
    }
    if (!jugador.estaVivo())
    {
        actualizarPartida(0.0f);
        return;
    }
    if (jugador.requiereElegirMejora())
    {
        prepararCartas();
        return;
    }
    actualizarArmas(segundos);
    jugador.procesarEntrada();
    jugador.actualizar(false);
    _aspectoJugador.actualizar(jugador.getPosicion());
    _escenario.actualizar(jugador.getPosicion());
    actualizarEncuentros(segundos);
    actualizarEnemigos(segundos);
    actualizarDanoContacto(segundos);
    actualizarIndicadoresDanio(segundos);
    actualizarProyectiles(segundos);
    _aspectoEnemigos.actualizar(_gestorOleadas.obtenerEnemigos(),
                               jugador.getFormaColision(), segundos);
    actualizarRecursos(segundos);
    actualizarPartida(segundos);
    if (_estado == EstadoJuego::Jugando)
    {
        actualizarDisparoAutomatico();
    }
}

void Juego::actualizarEnemigos(float segundos)
{
    if (jugador.estaVivo())
    {
        _gestorOleadas.actualizar(jugador.getPosicion(), segundos);
    }
}

void Juego::actualizarPartida(float segundos)
{
    if (_modoInfinito ? _estado != EstadoJuego::Jugando : !_partida.estaEnCurso())
    {
        return;
    }
    _partida.actualizar(segundos);
    int eliminados = _gestorOleadas.enemigosEliminados();
    bool jefeMuerto = _gestorOleadas.jefeFueDerrotado();
    // Incluir las muertes de este fotograma, aun pendientes de limpieza.
    for (const Enemigo& enemigo : _gestorOleadas.obtenerEnemigos())
    {
        if (!enemigo.estaVivo())
        {
            ++eliminados;
            jefeMuerto = jefeMuerto || enemigo.obtenerIdEnemigo() == 4;
        }
    }
    _partida.registrarEliminados(eliminados - _eliminadosRegistrados);
    _eliminadosRegistrados = eliminados;
    _partida.registrarNivel(jugador.getNivel());
    if (_modoInfinito)
    {
        if (!jugador.estaVivo())
        {
            _estado = EstadoJuego::Resultado;
            _opcionMenu = 0;
        }
        return;
    }

    if (!jugador.estaVivo())
    {
        _partida.marcarDerrota();
    }
    else if (jefeMuerto)
    {
        _partida.marcarVictoria();
    }
    if (!_partida.estaEnCurso())
    {
        ArchivoPartidas historial;
        _partidaGuardada = historial.agregar(_partida);
        ArchivoJugadores archivoJugador;
        RegistroJugador perfil = archivoJugador.leer();
        perfil.puntajeMaximo = std::max(perfil.puntajeMaximo, _partida.obtenerPuntaje());
        perfil.nivelMaximo = std::max(perfil.nivelMaximo, jugador.getNivel());
        perfil.chatarraTotal = jugador.getChatarra();
        archivoJugador.guardar(perfil);
        _estado = EstadoJuego::Resultado;
        _opcionMenu = 0;
    }
}

void Juego::actualizarDanoContacto(float segundos) {
    tiempoRestanteDanoContacto =
        std::max(0.0f, tiempoRestanteDanoContacto - segundos);

    if (!jugador.estaVivo() || tiempoRestanteDanoContacto > 0.0f) {
        return;
    }

    const FormaColision formaJugador = jugador.getFormaColision();
    for (const Enemigo& enemigo : _gestorOleadas.obtenerEnemigos())
    {
        if (!enemigo.estaVivo() ||
            !SistemaColisiones::hayInterseccion(
                formaJugador, enemigo.obtenerFormaColision())) {
            continue;
        }

        const int vidaAnterior = jugador.getVida();
        jugador.recibirDano(enemigo.obtenerDano());
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
            _partida.registrarChatarra(recurso->cantidad);
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
    if (!std::isfinite(segundos) || segundos <= 0.0f)
    {
        return;
    }
    const FormaColision escenario = _escenario.obtenerAreaVisible();
    auto& enemigos = _gestorOleadas.obtenerEnemigos();
    for (Proyectil& proyectil : _proyectiles)
    {
        float tiempoRestante = segundos;
        while (proyectil.estaActivo() && tiempoRestante > 0.0f)
        {
            float paso = tiempoRestante;
            if (proyectil.obtenerIdArma() == 4 && proyectil.obtenerVelocidad() > 0.0f)
            {
                paso = std::min(paso, PASO_ENERGIA / proyectil.obtenerVelocidad());
            }
            proyectil.actualizar(paso);
            tiempoRestante -= paso;
            if (!proyectil.estaActivo())
            {
                break;
            }
            for (std::size_t indice = 0; indice < enemigos.size(); ++indice)
            {
                Enemigo& enemigo = enemigos[indice];
                if (enemigo.estaVivo() &&
                    SistemaColisiones::hayInterseccion(
                        proyectil.obtenerFormaColision(), enemigo.obtenerFormaColision()) &&
                    proyectil.registrarImpacto(enemigo.obtenerIdInstancia()))
                {
                    enemigo.recibirDanio(proyectil.obtenerDano());
                    if (!enemigo.estaVivo())
                    {
                        generarRecompensa(enemigo);
                    }
                    if (proyectil.puedeRebotar() && intentarRebote(proyectil, enemigos))
                    {
                        break;
                    }
                    proyectil.terminarImpacto();
                    if (!proyectil.estaActivo())
                    {
                        break;
                    }
                }
            }
            if (proyectil.estaActivo() &&
                !SistemaColisiones::estaDentroDe(proyectil.obtenerFormaColision(), escenario))
            {
                proyectil.desactivar();
            }
        }
    }
    _proyectiles.erase(
        std::remove_if(_proyectiles.begin(), _proyectiles.end(),
            [](const Proyectil& proyectil)
            {
                return !proyectil.estaActivo();
            }),
        _proyectiles.end());
}

bool Juego::intentarRebote(Proyectil& proyectil, const std::vector<Enemigo>& enemigos)
{
    const Posicion origen = proyectil.obtenerPosicion();
    const float radio = std::min(450.0f, proyectil.obtenerDistanciaRestante());
    float distanciaMinima = radio * radio;
    Posicion objetivo{};
    bool encontrado = false;
    for (const Enemigo& enemigo : enemigos)
    {
        if (!enemigo.estaVivo() || proyectil.yaImpacto(enemigo.obtenerIdInstancia()))
        {
            continue;
        }
        const FormaColision forma = enemigo.obtenerFormaColision();
        const Posicion centro{forma.x + forma.ancho / 2.0f, forma.y + forma.alto / 2.0f};
        const float dx = centro.x - origen.x;
        const float dy = centro.y - origen.y;
        const float distancia = dx * dx + dy * dy;
        if (distancia > 0.0f && distancia < distanciaMinima)
        {
            distanciaMinima = distancia;
            objetivo = centro;
            encontrado = true;
        }
    }
    return encontrado && proyectil.rebotarHacia(objetivo);
}

void Juego::dibujar() {
    if (_estado == EstadoJuego::Ranking)
    {
        dibujarRanking();
        return;
    }
    if (_estado == EstadoJuego::Resultado)
    {
        dibujarResultado();
        return;
    }
    if (_estado != EstadoJuego::Jugando && _estado != EstadoJuego::SubiendoNivel)
    {
        dibujarMenuPrincipal();
        return;
    }
    ventana.setView(_escenario.obtenerVista());
    ventana.clear(sf::Color(28, 31, 36));
    _escenario.dibujar(ventana);
    dibujarEncuentros();
    dibujarRecursos();
    std::vector<const Enemigo*> enemigosOrdenados;
    for (const Enemigo& enemigo : _gestorOleadas.obtenerEnemigos())
    {
        if (enemigo.estaVivo())
        {
            enemigosOrdenados.push_back(&enemigo);
        }
    }
    std::stable_sort(enemigosOrdenados.begin(), enemigosOrdenados.end(),
        [](const Enemigo* primero, const Enemigo* segundo)
        {
            const FormaColision a = primero->obtenerFormaColision();
            const FormaColision b = segundo->obtenerFormaColision();
            return a.y + a.alto / 2.0f < b.y + b.alto / 2.0f;
        });
    const auto dibujarEnemigo = [this](const Enemigo& enemigo)
    {
        if (!_aspectoEnemigos.dibujar(ventana, enemigo))
        {
            sf::RectangleShape figura;
            configurarFiguraEnemigo(figura, enemigo, enemigo.obtenerIdEnemigo());
            ventana.draw(figura);
        }
    };
    std::size_t siguienteEnemigo = 0;
    while (siguienteEnemigo < enemigosOrdenados.size())
    {
        const Enemigo& enemigo = *enemigosOrdenados[siguienteEnemigo];
        const FormaColision forma = enemigo.obtenerFormaColision();
        if (forma.y + forma.alto / 2.0f > jugador.getPosicion().y)
        {
            break;
        }
        dibujarEnemigo(enemigo);
        ++siguienteEnemigo;
    }
    const bool herido = tiempoRestanteDanoContacto >
        SEGUNDOS_ENTRE_DANOS_CONTACTO - 0.12f;
    const int armaLejana = _inventarioArmas.obtenerIdSlot(1);
    if (armaLejana != 0)
    {
        _aspectoArmas.dibujarEnMano(ventana, obtenerAnclajeArma(1), _angulosArmas[1],
                                  armaLejana, _armasReflejadas[1]);
    }
    _aspectoJugador.dibujar(ventana, jugador.getPosicion(), jugador.estaVivo(), herido);
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        const int idArma = _inventarioArmas.obtenerIdSlot(slot);
        if (idArma != 0)
        {
            if (slot == 0)
            {
                _aspectoArmas.dibujarEnMano(ventana, obtenerAnclajeArma(slot), _angulosArmas[slot],
                                          idArma, _armasReflejadas[slot]);
            }
            _aspectoJugador.dibujarAgarre(ventana, jugador.getPosicion(), slot, jugador.estaVivo(), herido);
        }
    }
    while (siguienteEnemigo < enemigosOrdenados.size())
    {
        dibujarEnemigo(*enemigosOrdenados[siguienteEnemigo]);
        ++siguienteEnemigo;
    }
    for (const Proyectil& proyectil : _proyectiles)
    {
        _aspectoProyectiles.dibujar(ventana, proyectil);
    }
    dibujarIndicadoresDanio();
    dibujarIndicadoresRecurso();
    ventana.setView(ventana.getDefaultView());
    dibujarInterfaz();
    if (_estado == EstadoJuego::SubiendoNivel)
    {
        dibujarCartas();
    }
    else
    {
        dibujarMenuEstacion();
    }
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

    sf::RectangleShape fondo({1250.0f, 58.0f});
    fondo.setPosition({14.0f, 12.0f});
    fondo.setFillColor(sf::Color(8, 10, 14, 215));
    fondo.setOutlineColor(sf::Color(110, 120, 135));
    fondo.setOutlineThickness(1.0f);
    ventana.draw(fondo);

    // barra de vida grafica
    float anchoBarraVida = 200.0f;
    sf::RectangleShape fondoVida({anchoBarraVida, 18.0f});
    fondoVida.setPosition({26.0f, 18.0f});
    fondoVida.setFillColor(sf::Color(100, 20, 20));
    ventana.draw(fondoVida);

    sf::RectangleShape rellenoVida({anchoBarraVida * jugador.getPorcentajeVida(), 18.0f});
    rellenoVida.setPosition({26.0f, 18.0f});
    rellenoVida.setFillColor(sf::Color(50, 205, 50));
    ventana.draw(rellenoVida);

    // Texto sobre la barra de vida
    sf::Text textoVida(fuenteDanio, "VIDA: " + std::to_string(jugador.getVida()) + "/" + std::to_string(jugador.getVidaMaxima()), 13);
    textoVida.setPosition({32.0f, 19.0f});
    textoVida.setFillColor(sf::Color::White);
    ventana.draw(textoVida);

    // Barra de experiencia grafica
    float anchoBarraExp = 200.0f;
    sf::RectangleShape fondoExp({anchoBarraExp, 18.0f});
    fondoExp.setPosition({26.0f, 42.0f});
    fondoExp.setFillColor(sf::Color(35, 40, 55));
    ventana.draw(fondoExp);

    sf::RectangleShape rellenoExp({anchoBarraExp * jugador.getPorcentajeExperiencia(), 18.0f});
    rellenoExp.setPosition({26.0f, 42.0f});
    rellenoExp.setFillColor(sf::Color(60, 180, 240));
    ventana.draw(rellenoExp);

    sf::Text textoNivel(fuenteDanio, "NIVEL: " + std::to_string(jugador.getNivel()), 17);
    textoNivel.setPosition({1185.0f, 17.0f});
    textoNivel.setFillColor(sf::Color(255, 235, 100));
    ventana.draw(textoNivel);

    sf::Text textoExp(fuenteDanio, "EXP: " + std::to_string(jugador.getExperiencia()) + " / " + std::to_string(jugador.getExperienciaSiguienteNivel()), 13);
    textoExp.setPosition({32.0f, 43.0f});
    textoExp.setFillColor(sf::Color::White);
    ventana.draw(textoExp);

    sf::Text chatarra(fuenteDanio, "CHATARRA: " + std::to_string(jugador.getChatarra()), 17);
    chatarra.setPosition({295.0f, 17.0f});
    chatarra.setFillColor(sf::Color(255, 190, 55));
    ventana.draw(chatarra);

    const std::string nombresArmas = std::string(obtenerNombreArma(_inventarioArmas.obtenerIdSlot(0))) +
        (_inventarioArmas.obtenerIdSlot(1) == 0 ? "" :
            " + " + std::string(obtenerNombreArma(_inventarioArmas.obtenerIdSlot(1))));
    sf::Text nombreArma(fuenteDanio, "ARMAS: " + nombresArmas, 14);
    nombreArma.setPosition({480.0f, 17.0f});
    nombreArma.setFillColor(sf::Color(235, 235, 235));
    ventana.draw(nombreArma);

    sf::Text oleada(
        fuenteDanio, "OLEADA: " + std::to_string(_gestorOleadas.oleadaActual()) +
        " | TIEMPO: " + std::to_string(static_cast<int>(_gestorOleadas.tiempoDePartida())) +
        " s", 17);
    oleada.setPosition({810.0f, 17.0f});
    ventana.draw(oleada);

std::string slots = "1: " + std::string(obtenerNombreArma(_inventarioArmas.obtenerIdSlot(0)));
if (_inventarioArmas.obtenerIdSlot(0) == 1) {
        slots += " (reemplazable)";
} else {
    slots += " (fija)"; }
    slots += " | 2: " + std::string(obtenerNombreArma(_inventarioArmas.obtenerIdSlot(1)));
    if (_inventarioArmas.obtenerIdSlot(1) != 0) {
            slots += " (fija)";
    }

    sf::Text inventario(fuenteDanio, slots, 15);
    inventario.setPosition({480.0f, 41.0f});
    inventario.setFillColor(sf::Color(200, 220, 245));
    ventana.draw(inventario);

    std::string estadoJefe;
    if (_modoInfinito)
    {
        estadoJefe = "MODO INFINITO";
    }
    else if (_gestorOleadas.jefeEnCamino())
    {
        estadoJefe = "JEFE EN " + std::to_string(static_cast<int>(std::ceil(_gestorOleadas.segundosParaElJefe()))) + " s";
    }
    else if (_gestorOleadas.jefeEstaVivo())
    {
        estadoJefe = "JEFE CHATARRERO EN EL MAPA";
    }
    sf::Text resumen(fuenteDanio, "BAJAS: " + std::to_string(_partida.obtenerEnemigosEliminados()) +
        " | PUNTAJE: " + std::to_string(_partida.obtenerPuntaje()) + "   " + estadoJefe, 20);
    resumen.setPosition({26.0f, 110.0f});
    resumen.setFillColor(sf::Color(255, 165, 70));
    resumen.setOutlineColor(sf::Color::Black);
    resumen.setOutlineThickness(2.0f);
    ventana.draw(resumen);
    if (_tiempoMensajeCofre > 0.0f)
    {
        sf::Text mensaje(fuenteDanio, _mensajeCofre, 20);
        mensaje.setPosition({26.0f, 78.0f});
        mensaje.setFillColor(sf::Color(255, 215, 90));
        mensaje.setOutlineThickness(2.0f);
        mensaje.setOutlineColor(sf::Color::Black);
        ventana.draw(mensaje);
    }
}

void Juego::actualizarEncuentros(float segundos)
{
    _tiempoMensajeCofre = std::max(0.0f, _tiempoMensajeCofre - segundos);
    if (jugador.estaVivo())
    {
        _encuentros.generarAlrededor(jugador.getPosicion());
    }
}

void Juego::interactuarEncuentro()
{
    if (!jugador.estaVivo() || _estacionMejoras.menuAbierto())
    {
        return;
    }
    const Posicion posicion = jugador.getPosicion();
    ObjetoEncuentro* cofre = _encuentros.buscarCercano(posicion, TipoEncuentro::Cofre);
    ObjetoEncuentro* estacion = _encuentros.buscarCercano(posicion, TipoEncuentro::Estacion);
    if (cofre != nullptr && (estacion == nullptr ||
        std::hypot(posicion.x - cofre->posicion.x, posicion.y - cofre->posicion.y) <
        std::hypot(posicion.x - estacion->posicion.x, posicion.y - estacion->posicion.y)))
    {
        intentarAbrirCofre();
    }
    else
    {
        intentarComprarMejora();
    }
}
bool Juego::hayRecompensaCofre() const
{
    return jugador.getVidaMaxima() <= std::numeric_limits<int>::max() - 20;
}

void Juego::intentarAbrirCofre()
{
    ObjetoEncuentro* cofre = _encuentros.buscarCercano(jugador.getPosicion(), TipoEncuentro::Cofre);

    if (cofre == nullptr || !jugador.estaVivo() ||
        _estacionMejoras.menuAbierto() || !hayRecompensaCofre() ||
        jugador.getChatarra() < _costoCofre)
    {
        return;
    }
    if (!jugador.gastarChatarra(_costoCofre))
    {
        return;
    }
    const long long metaExperiencia = 100LL + (jugador.getNivel() - 1LL) * 50;
    const bool experienciaDisponible = jugador.getNivel() >= 1 &&
        jugador.getNivel() < (std::numeric_limits<int>::max() - 100) / 50 &&
        jugador.getExperiencia() < metaExperiencia &&
        jugador.getExperiencia() <= std::numeric_limits<int>::max() - 40;
    if (experienciaDisponible && std::uniform_int_distribution<int>(0, 1)(_generador) == 0)
    {
        jugador.ganarExperiencia(40);
        _mensajeCofre = "CONTENEDOR: +40 experiencia para el personaje";
    }
    else
    {
        jugador.aumentarVidaMaxima(20);
        _mensajeCofre = "CONTENEDOR: Blindaje +20 vida maxima y actual";
    }
    cofre->usado = true;
    const long long siguienteCosto = (static_cast<long long>(_costoCofre) * 8 + 4) / 5;
    _costoCofre = static_cast<int>(std::min(siguienteCosto,
        static_cast<long long>(std::numeric_limits<int>::max())));
    _tiempoMensajeCofre = 5.0f;
    ventana.setTitle(_mensajeCofre);
}

void Juego::aplicarBonificacionesArmas()
{
    const std::array<Arma*, 4> armas{{&armaPrueba, &_metralletaPrueba,
        &_escopetaPrueba, &_canonEnergiaPrueba}};
    for (Arma* arma : armas)
    {
        const BonificacionesArma bonos = _inventarioArmas.obtenerBonificaciones(arma->obtenerId());
        arma->configurarMejoras(bonos.porcentajeDano, bonos.porcentajeCadencia, bonos.especiales);
    }
}

void Juego::dibujarEncuentros()
{
    const FormaColision area = _escenario.obtenerAreaVisible();
    for (const auto& entrada : _encuentros.obtenerZonas())
    {
        for (const ObjetoEncuentro& objeto : entrada.second)
        {
            if (!objeto.presente)
            {
                continue;
            }
            if (objeto.posicion.x < area.x - 300.0f ||
                objeto.posicion.x > area.x + area.ancho + 300.0f ||
                objeto.posicion.y < area.y - 100.0f ||
                objeto.posicion.y > area.y + area.alto + 100.0f)
            {
                continue;
            }
            if (objeto.tipo == TipoEncuentro::Estacion)
            {
                dibujarEstacionMejoras(objeto);
            }
            else
            {
                dibujarCofre(objeto);
            }
        }
    }
}

void Juego::dibujarCofre(const ObjetoEncuentro& objeto)
{
    const Posicion posicion = objeto.posicion;
    const bool cerca = SistemaColisiones::estaDentroDeRadio(
        jugador.getPosicion(), posicion, 100.0f);
    const bool disponible = !objeto.usado && hayRecompensaCofre();
    const bool puedeAbrir = disponible && jugador.estaVivo() && jugador.getChatarra() >= _costoCofre;
    _aspectoEncuentros.dibujar(ventana, objeto, cerca, puedeAbrir);
    if (fuenteDanioDisponible)
    {
        std::string etiqueta = objeto.usado ? "CONTENEDOR VACIO" :
            "CONTENEDOR: " + std::to_string(_costoCofre) + " CHATARRA";
        if (cerca && !objeto.usado)
        {
            etiqueta += !disponible ? " | Sin recompensa disponible" :
                (puedeAbrir ? " | F: abrir" : " | Falta chatarra");
        }
        sf::Text texto(fuenteDanio, etiqueta, 18);
        texto.setPosition({posicion.x - 100.0f, posicion.y + 40.0f});
        texto.setOutlineThickness(1.0f);
        texto.setOutlineColor(sf::Color::Black);
        ventana.draw(texto);
    }
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

Posicion Juego::obtenerAnclajeArma(std::size_t slot) const
{
    return _aspectoJugador.obtenerMano(jugador.getPosicion(), slot, jugador.estaVivo());
}

void Juego::actualizarDisparoAutomatico()
{
    if (!jugador.estaVivo() || _estacionMejoras.menuAbierto())
    {
        return;
    }
    float alcance = 0.0f;
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        const int idArma = _inventarioArmas.obtenerIdSlot(slot);
        if (idArma != 0)
        {
            alcance = std::max(alcance, obtenerArmaPorId(idArma).obtenerAlcance());
        }
    }
    Posicion objetivo{};
    float distanciaCuadrada = 0.0f;
    if (buscarObjetivo(alcance, objetivo, distanciaCuadrada))
    {
        // Ambos disparos usan la misma pose y los anclajes de ese cuadro.
        _aspectoJugador.orientarHacia(jugador.getPosicion(), objetivo);
    }
    else
    {
        const Posicion direccion = _aspectoJugador.obtenerDireccionVista();
        const Posicion centro = jugador.getPosicion();
        objetivo = {centro.x + direccion.x * 1000.0f, centro.y + direccion.y * 1000.0f};
    }
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        if (_inventarioArmas.obtenerIdSlot(slot) != 0)
        {
            actualizarApuntadoArma(slot, objetivo);
            dispararArma(slot);
        }
    }
}

bool Juego::buscarObjetivo(float alcance, Posicion& centroObjetivo, float& distanciaMinima) const
{
    const FormaColision formaJugador = jugador.getFormaColision();
    const Posicion centroJugador{
        formaJugador.x + formaJugador.ancho / 2.0f,
        formaJugador.y + formaJugador.alto / 2.0f
    };

    bool hayObjetivo = false;
    distanciaMinima = std::numeric_limits<float>::max();
    const FormaColision areaVisible = _escenario.obtenerAreaVisible();
    for (const Enemigo& enemigo : _gestorOleadas.obtenerEnemigos())
    {
        if (!enemigo.estaVivo()) {
            continue;
        }

        const FormaColision formaEnemigo =
            enemigo.obtenerFormaColision();
        const Posicion centroEnemigo{
            formaEnemigo.x + formaEnemigo.ancho / 2.0f,
            formaEnemigo.y + formaEnemigo.alto / 2.0f
        };
        const float diferenciaX = centroEnemigo.x - centroJugador.x;
        const float diferenciaY = centroEnemigo.y - centroJugador.y;
        const float distanciaCuadrada =
            diferenciaX * diferenciaX + diferenciaY * diferenciaY;
        if (!std::isfinite(distanciaCuadrada) ||
            distanciaCuadrada > alcance * alcance ||
            centroEnemigo.x < areaVisible.x ||
            centroEnemigo.y < areaVisible.y ||
            centroEnemigo.x >= areaVisible.x + areaVisible.ancho ||
            centroEnemigo.y >= areaVisible.y + areaVisible.alto)
        {
            continue;
        }
        if (distanciaCuadrada < distanciaMinima) {
            distanciaMinima = distanciaCuadrada;
            hayObjetivo = true;
            centroObjetivo = centroEnemigo;
        }
    }

    return hayObjetivo;
}

void Juego::actualizarApuntadoArma(std::size_t slot, Posicion objetivo)
{
    const Posicion anclaje = obtenerAnclajeArma(slot);
    if (objetivo.x != anclaje.x || objetivo.y != anclaje.y)
    {
        _armasReflejadas[slot] = objetivo.x < anclaje.x;
        _angulosArmas[slot] = _aspectoArmas.obtenerAnguloDesdeMano(
            anclaje, objetivo, _inventarioArmas.obtenerIdSlot(slot), _armasReflejadas[slot]);
    }
}

void Juego::dispararArma(std::size_t slot)
{
    Arma& arma = obtenerArmaPorId(_inventarioArmas.obtenerIdSlot(slot));
    if (!jugador.estaVivo() || _estacionMejoras.menuAbierto())
    {
        return;
    }
    Posicion centroObjetivo{};
    float distanciaMinima = 0.0f;
    if (!buscarObjetivo(arma.obtenerAlcance(), centroObjetivo, distanciaMinima))
    {
        return;
    }

    actualizarApuntadoArma(slot, centroObjetivo);
    if (!arma.puedeDisparar())
    {
        return;
    }

    const bool esMetralleta = arma.obtenerId() == 2;
    float ancho = esMetralleta ? ANCHO_PROYECTIL_METRALLETA : ANCHO_PROYECTIL_PISTOLA;
    float alto = esMetralleta ? ALTO_PROYECTIL_METRALLETA : ALTO_PROYECTIL_PISTOLA;
    float velocidad = VELOCIDAD_PROYECTIL;
    if (arma.obtenerId() == 3)
    {
        ancho = 8.0f;
        alto = 8.0f;
        velocidad = 650.0f;
    }
    else if (arma.obtenerId() == 4)
    {
        ancho = 40.0f;
        alto = 10.0f;
        velocidad = 1000.0f;
    }
    const Posicion punta = _aspectoArmas.obtenerPuntaEnMano(obtenerAnclajeArma(slot),
        _angulosArmas[slot], arma.obtenerId(), _armasReflejadas[slot]);
    // La direccion sigue el eje del canon incluso si el objetivo esta pegado al jugador.
    const float distanciaDisparo = std::min(arma.obtenerAlcance() * 0.9999f, std::max(1.0f, std::sqrt(distanciaMinima)));
    const Posicion destino{punta.x + std::cos(_angulosArmas[slot]) * distanciaDisparo,
                           punta.y + std::sin(_angulosArmas[slot]) * distanciaDisparo};
    const std::size_t cantidadAnterior = _proyectiles.size();
    arma.intentarDispararRafaga(_proyectiles, punta, destino, velocidad, ancho, alto);
    for (std::size_t indice = cantidadAnterior; indice < _proyectiles.size(); ++indice)
    {
        _proyectiles[indice].configurarRebotes(jugador.getRebotes());
    }
}

void Juego::actualizarArmas(float segundos)
{
    armaPrueba.actualizar(segundos);
    _metralletaPrueba.actualizar(segundos);
    _escopetaPrueba.actualizar(segundos);
    _canonEnergiaPrueba.actualizar(segundos);
}

void Juego::seleccionarArmaPrueba(TipoArmaPrueba tipo)
{
    if (!_inventarioArmas.poseeArma(static_cast<int>(tipo) + 1))
    {
        return;
    }
    _armaSeleccionada = tipo;
}

void Juego::seleccionarSlotArma(std::size_t slot)
{
    const int idArma = _inventarioArmas.obtenerIdSlot(slot);
    if (idArma != 0)
    {
        seleccionarArmaPrueba(static_cast<TipoArmaPrueba>(idArma - 1));
    }
}

Arma& Juego::obtenerArmaSeleccionada()
{
    return obtenerArmaPorId(static_cast<int>(_armaSeleccionada) + 1);
}

Arma& Juego::obtenerArmaPorId(int idArma)
{
    switch (static_cast<TipoArmaPrueba>(idArma - 1))
    {
        case TipoArmaPrueba::Metralleta:
            return _metralletaPrueba;
        case TipoArmaPrueba::Escopeta:
            return _escopetaPrueba;
        case TipoArmaPrueba::CanonEnergia:
            return _canonEnergiaPrueba;
        case TipoArmaPrueba::Pistola:
        default:
            return armaPrueba;
    }
}

const char* Juego::obtenerNombreArma(int idArma)
{
    return idArma == 0 ? "Vacio" : obtenerArmaPorId(idArma).obtenerNombre();
}

void Juego::reiniciarPrueba() {
    cancelarCompraEstacion();
    jugador.reiniciar();
    _aspectoJugador.reiniciar(jugador.getPosicion());
    _aspectoEnemigos.reiniciar();
    _estacionSeleccionada = nullptr;
    _opcionReemplazo = 3;
    _encuentros.generarAlrededor(jugador.getPosicion());
    _mensajeCofre.clear();
    _tiempoMensajeCofre = 0.0f;
    _escenario.actualizar(jugador.getPosicion());
    _gestorOleadas.reiniciar();
    _partida = Partida(1);
    _partidaGuardada = false;
    _modoInfinito = false;
    _eliminadosRegistrados = 0;
    _proyectiles.clear();
    armaPrueba.reiniciar();
    _metralletaPrueba.reiniciar();
    _escopetaPrueba.reiniciar();
    _canonEnergiaPrueba.reiniciar();
    seleccionarSlotArma(0);
    _angulosArmas.fill(0.0f);
    _armasReflejadas.fill(false);
    indicadoresDanio.clear();
    recursosSuelo.clear();
    indicadoresRecurso.clear();
    tiempoRestanteDanoContacto = 0.0f;
    ventana.setTitle(
        "WASD: mover | F: estacion/contenedor | dos armas automaticas | R: reiniciar prueba");
    relojFotogramas.restart();
}

void Juego::intentarComprarMejora()
{
    if (!jugador.estaVivo() || _estacionMejoras.menuAbierto())
    {
        return;
    }
    ObjetoEncuentro* estacion = _encuentros.buscarCercano(jugador.getPosicion(), TipoEncuentro::Estacion);
    if (estacion == nullptr)
    {
        return;
    }
    _estacionMejoras.establecerPosicion(estacion->posicion);
    if (_estacionMejoras.puedeComprar(jugador.getPosicion(), jugador.getChatarra()) &&
        jugador.gastarChatarra(_estacionMejoras.obtenerCosto()))
    {
        _estacionMejoras.abrirMenu(_inventarioArmas, _generador);
        if (!_estacionMejoras.menuAbierto())
        {
            jugador.ganarChatarra(_estacionMejoras.obtenerCosto());
            return;
        }
        _estacionSeleccionada = estacion;
        _opcionReemplazo = 3;
        ventana.setTitle("ESTACION: 1/2/3 elegir oferta | Escape: cancelar y devolver pago");
    }
}
void Juego::elegirMejoraEstacion(std::size_t indice)
{
    if (_estacionSeleccionada == nullptr || _estacionSeleccionada->usado ||
        !jugador.estaVivo() || !_estacionMejoras.menuAbierto() || indice >= 3 ||
        _opcionReemplazo < 3)
    {
        return;
    }
    const OfertaArma oferta = _estacionMejoras.obtenerOferta(indice);
    if (!_inventarioArmas.puedeAplicarOferta(oferta))
    {
        return;
    }
    if (_inventarioArmas.reemplazaPistola(oferta.idArma))
    {
        _opcionReemplazo = indice;
        return;
    }
    finalizarCompraEstacion(indice, false);
}

void Juego::confirmarReemplazoPistola()
{
    if (_opcionReemplazo < 3)
    {
        finalizarCompraEstacion(_opcionReemplazo, true);
    }
}

void Juego::finalizarCompraEstacion(std::size_t indice, bool confirmarReemplazo)
{
    if (_estacionSeleccionada == nullptr || _estacionSeleccionada->usado ||
        !jugador.estaVivo() || !_estacionMejoras.confirmarEleccion(indice, _inventarioArmas, confirmarReemplazo))
    {
        return;
    }
    aplicarBonificacionesArmas();
    seleccionarArmaPrueba(static_cast<TipoArmaPrueba>(_estacionMejoras.obtenerOferta(indice).idArma - 1));
    _estacionSeleccionada->usado = true;
    _estacionSeleccionada = nullptr;
    _opcionReemplazo = 3;
    ventana.setTitle("WASD: mover | F: estacion/contenedor | dos armas automaticas | R: reiniciar prueba");
    relojFotogramas.restart();
}

void Juego::cancelarCompraEstacion()
{
    if (!_estacionMejoras.menuAbierto())
    {
        return;
    }
    jugador.ganarChatarra(_estacionMejoras.obtenerCosto());
    _estacionMejoras.cancelarCompra();
    _estacionSeleccionada = nullptr;
    _opcionReemplazo = 3;
    ventana.setTitle("WASD: mover | F: estacion/contenedor | dos armas automaticas | R: reiniciar prueba");
    relojFotogramas.restart();
}

void Juego::dibujarEstacionMejoras(const ObjetoEncuentro& objeto)
{
    const Posicion posicion = objeto.posicion;
    const bool cerca = SistemaColisiones::estaDentroDeRadio(jugador.getPosicion(), posicion, 100.0f);
    const bool puedeComprar = !objeto.usado && cerca && jugador.estaVivo() &&
        !_estacionMejoras.menuAbierto() && jugador.getChatarra() >= _estacionMejoras.obtenerCosto();
    _aspectoEncuentros.dibujar(ventana, objeto, cerca, puedeComprar);
    if (fuenteDanioDisponible)
    {
        std::string etiqueta = objeto.usado ? "ESTACION APAGADA - USADA" :
            "MEJORAS: " + std::to_string(_estacionMejoras.obtenerCosto()) + " CHATARRA";
        if (cerca && !objeto.usado)
        {
            etiqueta += puedeComprar ? " | F: comprar" : " | Falta chatarra";
        }
        sf::Text texto(fuenteDanio, etiqueta, 18);
        texto.setPosition({posicion.x - 100.0f, posicion.y + 45.0f});
        texto.setFillColor(puedeComprar ? sf::Color(110, 255, 185) : sf::Color(255, 205, 125));
        texto.setOutlineThickness(1.0f);
        texto.setOutlineColor(sf::Color::Black);
        ventana.draw(texto);
    }
}

void Juego::dibujarMenuEstacion()
{
    if (!_estacionMejoras.menuAbierto() || !fuenteDanioDisponible)
    {
        return;
    }
    sf::RectangleShape velo({1920.0f, 1080.0f});
    velo.setFillColor(sf::Color(0, 0, 0, 210));
    ventana.draw(velo);
    sf::Text titulo(fuenteDanio, "ESTACION DE ARMAS - ELEGI UNA OFERTA", 34);
    titulo.setPosition({300.0f, 220.0f});
    ventana.draw(titulo);
    sf::Text costo(fuenteDanio, "Pagado: " + std::to_string(_estacionMejoras.obtenerCosto()) +
                    " de chatarra | Proxima compra: +60%", 22);
    costo.setPosition({300.0f, 270.0f});
    ventana.draw(costo);
    const std::string slots = "Slot 1: " + std::string(obtenerNombreArma(_inventarioArmas.obtenerIdSlot(0))) +
        (_inventarioArmas.obtenerIdSlot(0) == 1 ? " (solo la pistola se puede reemplazar)" : " (fijo)") +
        " | Slot 2: " + obtenerNombreArma(_inventarioArmas.obtenerIdSlot(1)) +
        (_inventarioArmas.obtenerIdSlot(1) == 0 ? "" : " (fijo)");
    sf::Text inventario(fuenteDanio, slots, 22);
    inventario.setPosition({300.0f, 315.0f});
    ventana.draw(inventario);
    if (_opcionReemplazo < 3)
    {
        const BonificacionesArma bonos = _inventarioArmas.obtenerBonificaciones(1);
        const OfertaArma oferta = _estacionMejoras.obtenerOferta(_opcionReemplazo);
        sf::Text advertencia(fuenteDanio,
            "REEMPLAZAR PISTOLA: ELECCION DEFINITIVA\n\n" +
            _estacionMejoras.describirOpcion(_opcionReemplazo, obtenerNombreArma(oferta.idArma)) +
            "\n\nPerdes la pistola y sus mejoras: +" + std::to_string(bonos.porcentajeDano) +
            "% dano, +" + std::to_string(bonos.porcentajeCadencia) + "% cadencia, " +
            std::to_string(bonos.especiales) + " proyectiles extra.\n" +
            "El slot 2 se conserva. Los dos slots quedan fijos.\n\n" +
            "1: confirmar reemplazo | 2: volver a las mismas ofertas", 25);
        advertencia.setPosition({300.0f, 405.0f});
        advertencia.setFillColor(sf::Color(255, 205, 125));
        ventana.draw(advertencia);
        return;
    }
    for (std::size_t i = 0; i < 3; ++i)
    {
        const OfertaArma oferta = _estacionMejoras.obtenerOferta(i);
        sf::RectangleShape tarjeta({1320.0f, 110.0f});
        tarjeta.setPosition({300.0f, 385.0f + i * 135.0f});
        tarjeta.setFillColor(sf::Color(35, 52, 57));
        tarjeta.setOutlineThickness(2.0f);
        tarjeta.setOutlineColor(obtenerColorRareza(oferta.rareza));
        ventana.draw(tarjeta);
        sf::Text texto(fuenteDanio, std::to_string(i + 1) + "   " +
                        _estacionMejoras.describirOpcion(i, obtenerNombreArma(oferta.idArma)), 24);
        texto.setPosition({325.0f, 399.0f + i * 135.0f});
        texto.setFillColor(obtenerColorRareza(oferta.rareza));
        ventana.draw(texto);
        const char* destino = _inventarioArmas.poseeArma(oferta.idArma) ? "Mejora tu arma actual" :
            (_inventarioArmas.reemplazaPistola(oferta.idArma) ?
                "Reemplaza la pistola: requiere confirmacion y fija el slot 1" : "Adquiere el arma con estos beneficios y fija el slot 2");
        sf::Text detalle(fuenteDanio, destino, 20);
        detalle.setPosition({325.0f, 445.0f + i * 135.0f});
        ventana.draw(detalle);
        _aspectoArmas.dibujar(ventana, {1530.0f, 457.0f + i * 135.0f}, 0.0f, oferta.idArma);
    }
    sf::Text ayuda(fuenteDanio,
        "Partida en pausa | 1 / 2 / 3: elegir | Escape: devolver pago | R: reiniciar prueba", 22);
    ayuda.setPosition({300.0f, 810.0f});
    ventana.draw(ayuda);
}

void Juego::manejarEscape()
{
    if (_estado == EstadoJuego::Pausa)
    {
        _estado = EstadoJuego::Jugando;
        relojFotogramas.restart();
    }
    else if (_estado == EstadoJuego::Jugando)
    {
        if (_estacionMejoras.menuAbierto())
        {
            cancelarCompraEstacion();
        }
        else
        {
            _estado = EstadoJuego::Pausa;
            _opcionMenu = 0;
        }
    }
}

void Juego::procesarEntradaMenu(sf::Keyboard::Scancode tecla)
{
    if (_estado == EstadoJuego::ConfirmarReinicio)
    {
        if (tecla == sf::Keyboard::Scancode::Escape)
        {
            _estado = EstadoJuego::Menu;
        }
        else if (tecla == sf::Keyboard::Scancode::Enter && !confirmarNuevaPartida())
        {
            _estado = EstadoJuego::Menu;
            _mensajeMenu = "No se pudo iniciar una nueva partida.";
        }
        return;
    }
    if (_estado == EstadoJuego::SubiendoNivel)
    {
        using Tecla = sf::Keyboard::Scancode;
        int opcion = -1;
        if (tecla == Tecla::Num1 || tecla == Tecla::Numpad1)
        {
            opcion = 0;
        }
        else if (tecla == Tecla::Num2 || tecla == Tecla::Numpad2)
        {
            opcion = 1;
        }
        else if (tecla == Tecla::Num3 || tecla == Tecla::Numpad3)
        {
            opcion = 2;
        }
        else if (tecla == Tecla::Escape)
        {
            jugador.omitirMejora();
            prepararCartas();
            return;
        }
        if (opcion >= 0 && _cartasDisponibles[opcion])
        {
            if (jugador.aplicarMejoraElegida(opcion + 1))
            {
                prepararCartas();
            }
            else
            {
                _errorGuardadoCarta = true;
            }
        }
        return;
    }

    if (_estado == EstadoJuego::Resultado)
    {
        if (tecla == sf::Keyboard::Scancode::I)
        {
            continuarEnInfinito();
        }
        else if (tecla == sf::Keyboard::Scancode::Enter)
        {
            iniciarNuevaPartida();
        }
        else if (tecla == sf::Keyboard::Scancode::Escape)
        {
            _estado = EstadoJuego::Menu;
            _opcionMenu = 0;
        }
        return;
    }
    if (_estado == EstadoJuego::Controles || _estado == EstadoJuego::Ranking)
    {
        if (tecla == sf::Keyboard::Scancode::Escape ||
            tecla == sf::Keyboard::Scancode::Enter)
        {
            _estado = EstadoJuego::Menu;
        }
        return;
    }
    const int cantidadOpciones = _estado == EstadoJuego::Pausa ? 2 : 5;
    if (tecla == sf::Keyboard::Scancode::Up)
    {
        _mensajeMenu.clear();
        _opcionMenu = (_opcionMenu + cantidadOpciones - 1) % cantidadOpciones;
    }
    else if (tecla == sf::Keyboard::Scancode::Down)
    {
        _mensajeMenu.clear();
        _opcionMenu = (_opcionMenu + 1) % cantidadOpciones;
    }
    else if (tecla == sf::Keyboard::Scancode::Escape)
    {
        if (_estado == EstadoJuego::Pausa)
        {
            manejarEscape();
        }
        else
        {
            ventana.close();
        }
    }
    else if (tecla == sf::Keyboard::Scancode::Enter)
    {
        if (_estado == EstadoJuego::Pausa)
        {
            if (_opcionMenu == 0)
            {
                manejarEscape();
            }
            else
            {
                salirDelJuego();
            }
        }
        else if (_opcionMenu == 0)
        {
            if (_continuacionDisponible)
            {
                if (!reanudarPartida())
                {
                    _mensajeMenu = "No se pudo cargar la partida guardada.";
                }
            }
            else
            {
                _mensajeMenu = "No hay una partida guardada.";
            }
        }
        else if (_opcionMenu == 1)
        {
            _estado = EstadoJuego::ConfirmarReinicio;
        }
        else if (_opcionMenu == 2)
        {
            _estado = EstadoJuego::Controles;
        }
        else if (_opcionMenu == 3)
        {
            cargarRanking();
            _estado = EstadoJuego::Ranking;
        }
        else
        {
            ventana.close();
        }
    }
}

void Juego::dibujarMenuPrincipal()
{
    ventana.setView(ventana.getDefaultView());
    ventana.clear(sf::Color(18, 23, 29));
    const sf::Vector2f tamanio = ventana.getDefaultView().getSize();
    const float centroX = tamanio.x / 2.0f;
    sf::RectangleShape panel({760.0f, 760.0f});
    panel.setPosition({centroX - 380.0f, tamanio.y / 2.0f - 380.0f});
    panel.setFillColor(sf::Color(28, 35, 43));
    panel.setOutlineThickness(2.0f);
    panel.setOutlineColor(sf::Color(75, 92, 103));
    ventana.draw(panel);
    if (fuenteDanioDisponible)
    {
        const bool esPausa = _estado == EstadoJuego::Pausa;
        sf::Text texto(fuenteDanio, esPausa ? U"PAUSA" : U"EL \u00DALTIMO RECOLECTOR", 56);
        texto.setFillColor(sf::Color(255, 190, 55));
        const sf::FloatRect limites = texto.getLocalBounds();
        texto.setOrigin({limites.position.x + limites.size.x / 2.0f,
                         limites.position.y});
        texto.setPosition({centroX, tamanio.y / 2.0f - 300.0f});
        ventana.draw(texto);
        if (_estado == EstadoJuego::Menu || esPausa)
        {
            const std::array<const char*, 5> OPCIONES = esPausa ?
                std::array<const char*, 5>{{"CONTINUAR", "SALIR", "", "", ""}} :
                std::array<const char*, 5>{{"CONTINUAR", "NUEVA PARTIDA", "CONTROLES", "RANKING", "SALIR"}};
            const int cantidadOpciones = esPausa ? 2 : 5;
            for (int indice = 0; indice < cantidadOpciones; ++indice)
            {
                const float altoBoton = esPausa ? 80.0f : 60.0f;
                const float alto = tamanio.y / 2.0f + (esPausa ? -100.0f : -170.0f) +
                    indice * (esPausa ? 110.0f : 80.0f);
                const bool seleccionada = indice == _opcionMenu;
                const bool disponible = esPausa || indice != 0 || _continuacionDisponible;
                sf::RectangleShape boton({560.0f, altoBoton});
                boton.setPosition({centroX - 280.0f, alto});
                boton.setFillColor(seleccionada && disponible ? sf::Color(255, 190, 55) :
                                                 sf::Color(40, 50, 61));
                if (seleccionada && !disponible)
                {
                    boton.setOutlineThickness(2.0f);
                    boton.setOutlineColor(sf::Color(150, 170, 183));
                }
                ventana.draw(boton);
                texto.setString(OPCIONES[indice]);
                texto.setCharacterSize(32);
                texto.setFillColor(seleccionada ? sf::Color(18, 23, 29) :
                                                 sf::Color(220, 230, 235));
                if (!disponible)
                {
                    texto.setFillColor(sf::Color(150, 170, 183));
                }
                const sf::FloatRect area = texto.getLocalBounds();
                texto.setOrigin({area.position.x + area.size.x / 2.0f,
                                 area.position.y + area.size.y / 2.0f});
                texto.setPosition({centroX, alto + altoBoton / 2.0f});
                ventana.draw(texto);
            }
        }
        else if (_estado == EstadoJuego::ConfirmarReinicio)
        {
            texto.setString(_continuacionDisponible ?
                "Se descartara la partida guardada y se reiniciara el perfil.\n\n"
                "Enter: nueva partida | Escape: cancelar" :
                "Se borraran los niveles, la chatarra y las mejoras.\n\n"
                "Enter: nueva partida | Escape: cancelar");
            texto.setCharacterSize(24);
            const auto area = texto.getLocalBounds();
            texto.setOrigin({area.position.x + area.size.x / 2.0f, area.position.y});
            texto.setPosition({centroX, tamanio.y / 2.0f - 90.0f});
            ventana.draw(texto);
        }
        else
        {
            const std::array<const char*, 7> CONTROLES{{
                "WASD: mover",
                "Ataque automatico al enemigo cercano",
                "F: interactuar con estaciones y contenedores",
                "Dos armas automaticas | 1 / 2 / 3: ofertas",
                "Escape: cancelar compra o pausar",
                "Prueba: R reinicia, K recibe dano, E da experiencia",
                "Enter o Escape: volver"
            }};
            texto.setCharacterSize(24);
            texto.setFillColor(sf::Color(220, 230, 235));
            for (std::size_t indice = 0; indice < CONTROLES.size(); ++indice)
            {
                texto.setString(CONTROLES[indice]);
                const sf::FloatRect area = texto.getLocalBounds();
                texto.setOrigin({area.position.x + area.size.x / 2.0f,
                                 area.position.y});
                texto.setPosition({centroX, tamanio.y / 2.0f - 120.0f + indice * 55.0f});
                ventana.draw(texto);
            }
        }
        if (_estado == EstadoJuego::Menu || esPausa)
        {
            texto.setString(!_mensajeMenu.empty() ? _mensajeMenu : esPausa ?
                "Flechas: elegir | Enter: confirmar | Escape: continuar" :
                "Flechas: elegir   |   Enter: confirmar");
            texto.setCharacterSize(24);
            texto.setFillColor(sf::Color(150, 170, 183));
            const sf::FloatRect area = texto.getLocalBounds();
            texto.setOrigin({area.position.x + area.size.x / 2.0f, area.position.y});
            texto.setPosition({centroX, tamanio.y / 2.0f + 290.0f});
            ventana.draw(texto);
        }
    }
    ventana.display();
}

void Juego::prepararCartas()
{
    _estado = EstadoJuego::Jugando;
    _errorGuardadoCarta = false;
    if (!jugador.requiereElegirMejora())
    {
        return;
    }
    _cartasDisponibles.fill(false);
    ArchivoMejoras archivo;
    for (const Mejora& mejora : archivo.leerCatalogo())
    {
        if (mejora.idMejora >= 1 && mejora.idMejora <= 3 &&
            mejora.nivelActual < mejora.nivelMaximo)
        {
            _cartasDisponibles[mejora.idMejora - 1] = true;
        }
    }
    if (std::none_of(_cartasDisponibles.begin(), _cartasDisponibles.end(),
        [](bool disponible) { return disponible; }))
    {
        while (jugador.requiereElegirMejora())
        {
            jugador.omitirMejora();
        }
        return;
    }
    if (_cartasDisponibles[0] || _cartasDisponibles[1])
    {
        std::bernoulli_distribution sorteo(0.25);
        _cartasDisponibles[2] = _cartasDisponibles[2] && sorteo(_generador);
    }
    _estado = EstadoJuego::SubiendoNivel;
}

void Juego::dibujarCartas()
{
    const sf::Vector2f tamanio = ventana.getDefaultView().getSize();
    sf::RectangleShape fondo(tamanio);
    fondo.setFillColor(sf::Color(0, 0, 0, 215));
    ventana.draw(fondo);
    if (!fuenteDanioDisponible)
    {
        return;
    }
    sf::Text titulo(fuenteDanio, "NUEVO NIVEL - ELEGI UNA MEJORA", 34);
    const auto limites = titulo.getLocalBounds();
    titulo.setPosition({(tamanio.x - limites.size.x) / 2.0f - limites.position.x, tamanio.y * 0.22f});
    ventana.draw(titulo);

    const std::array<const char*, 3> TEXTOS{{
        "1. VIDA MAXIMA\n+20 de vida",
        "2. VELOCIDAD\n+1 de velocidad",
        "3. REBOTE (RARA)\n+1 rebote por proyectil"
    }};
    const int cantidad = std::count(_cartasDisponibles.begin(), _cartasDisponibles.end(), true);
    const float ancho = std::min(360.0f, tamanio.x * 0.26f);
    const float separacion = ancho * 0.12f;
    float x = (tamanio.x - cantidad * ancho - (cantidad - 1) * separacion) / 2.0f;
    const float y = tamanio.y * 0.39f;
    for (std::size_t indice = 0; indice < TEXTOS.size(); ++indice)
    {
        if (!_cartasDisponibles[indice])
        {
            continue;
        }
        sf::RectangleShape carta({ancho, tamanio.y * 0.20f});
        carta.setPosition({x, y});
        carta.setFillColor(indice == 2 ? sf::Color(55, 43, 26) : sf::Color(35, 52, 57));
        carta.setOutlineThickness(2.0f);
        carta.setOutlineColor(indice == 2 ? sf::Color(255, 190, 55) : sf::Color(100, 120, 140));
        ventana.draw(carta);
        sf::Text texto(fuenteDanio, TEXTOS[indice], static_cast<unsigned int>(ancho * 0.065f));
        texto.setPosition({x + ancho * 0.07f, y + tamanio.y * 0.055f});
        ventana.draw(texto);
        x += ancho + separacion;
    }
    sf::Text ayuda(fuenteDanio, _errorGuardadoCarta ?
        "No se pudo guardar. Volve a intentar o pulsa Escape." : "Escape: omitir mejora", 22);
    const auto limitesAyuda = ayuda.getLocalBounds();
    ayuda.setPosition({(tamanio.x - limitesAyuda.size.x) / 2.0f - limitesAyuda.position.x, tamanio.y * 0.68f});
    ventana.draw(ayuda);
}

void Juego::cargarRanking()
{
    ArchivoPartidas archivo;
    _ranking = archivo.leerTodas();

    const std::size_t MAXIMO = 8;
    const std::size_t cantidad = std::min(_ranking.size(), MAXIMO);

    for (std::size_t i = 0; i < cantidad; ++i)
    {
        for (std::size_t j = i + 1; j < _ranking.size(); ++j)
        {
            if (_ranking[j].puntaje > _ranking[i].puntaje)
            {
                std::swap(_ranking[i], _ranking[j]);
            }
        }
    }

    if (_ranking.size() > MAXIMO)
    {
        _ranking.resize(MAXIMO);
    }
}

void Juego::dibujarRanking()
{
    ventana.setView(ventana.getDefaultView());
    ventana.clear(sf::Color(18, 23, 29));
    const sf::Vector2f tamanio = ventana.getDefaultView().getSize();
    const float centroX = tamanio.x / 2.0f;
    const float centroY = tamanio.y / 2.0f;
    sf::RectangleShape panel({1100.0f, 760.0f});
    panel.setPosition({centroX - 550.0f, centroY - 380.0f});
    panel.setFillColor(sf::Color(28, 35, 43));
    panel.setOutlineThickness(2.0f);
    panel.setOutlineColor(sf::Color(75, 92, 103));
    ventana.draw(panel);
    if (fuenteDanioDisponible)
    {
        sf::Text titulo(fuenteDanio, "RANKING", 48);
        titulo.setFillColor(sf::Color(255, 190, 55));
        const sf::FloatRect limites = titulo.getLocalBounds();
        titulo.setOrigin({limites.position.x + limites.size.x / 2.0f, limites.position.y});
        titulo.setPosition({centroX, centroY - 300.0f});
        ventana.draw(titulo);

        const std::array<const char*, 5> COLUMNAS{{"PUESTO", "PUNTOS", "NIVEL", "TIEMPO", "RESULTADO"}};
        const std::array<float, 5> POSICIONES{{-450.0f, -310.0f, -100.0f, 70.0f, 270.0f}};
        sf::Text texto(fuenteDanio, "", 24);
        texto.setFillColor(sf::Color(150, 170, 183));
        for (std::size_t columna = 0; columna < COLUMNAS.size(); ++columna)
        {
            texto.setString(COLUMNAS[columna]);
            texto.setPosition({centroX + POSICIONES[columna], centroY - 200.0f});
            ventana.draw(texto);
        }
        if (_ranking.empty())
        {
            texto.setString("Sin partidas para mostrar.");
            texto.setPosition({centroX - 450.0f, centroY - 120.0f});
            ventana.draw(texto);
        }
        const std::size_t cantidad = std::min(_ranking.size(), std::size_t{8});
        for (std::size_t indice = 0; indice < cantidad; ++indice)
        {
            const RegistroPartida& partida = _ranking[indice];
            const int minutos = partida.duracionSegundos / 60;
            const int segundos = partida.duracionSegundos % 60;
            const std::string tiempo = std::to_string(minutos) + ":" +
                (segundos < 10 ? "0" : "") + std::to_string(segundos);
            const std::array<std::string, 5> valores{{std::to_string(indice + 1),
                std::to_string(partida.puntaje), std::to_string(partida.nivelAlcanzado),
                tiempo, partida.victoria ? "Victoria" : "Derrota"}};
            texto.setFillColor(indice == 0 ? sf::Color(255, 190, 55) : sf::Color(220, 230, 235));
            for (std::size_t columna = 0; columna < valores.size(); ++columna)
            {
                texto.setString(valores[columna]);
                texto.setPosition({centroX + POSICIONES[columna], centroY - 140.0f + indice * 45.0f});
                ventana.draw(texto);
            }
        }
        texto.setString("Enter o Escape: volver al menu");
        texto.setFillColor(sf::Color(150, 170, 183));
        const sf::FloatRect ayuda = texto.getLocalBounds();
        texto.setOrigin({ayuda.position.x + ayuda.size.x / 2.0f, ayuda.position.y});
        texto.setPosition({centroX, centroY + 290.0f});
        ventana.draw(texto);
    }
    ventana.display();
}

void Juego::iniciarNuevaPartida()
{
    _partidaGuardada = false;
    cancelarCompraEstacion();
    _estacionSeleccionada = nullptr;
    jugador = Jugador{};
    _inventarioArmas.reiniciar();
    _estacionMejoras = EstacionMejoras{};
    _encuentros = EncuentroAleatorio(_generador());
    _costoCofre = 15;
    aplicarBonificacionesArmas();
    reiniciarPrueba();
    _estado = EstadoJuego::Jugando;
    _opcionMenu = 0;
}

bool Juego::comprarMejoraPermanente()
{
    constexpr int COSTO = 10;
    if (!jugador.estaVivo() || jugador.getChatarra() < COSTO)
    {
        return false;
    }
    ArchivoMejoras archivo;
    std::vector<Mejora> catalogo = archivo.leerCatalogo();
    for (Mejora& mejora : catalogo)
    {
        if (mejora.idMejora != 1 || mejora.nivelActual >= mejora.nivelMaximo)
        {
            continue;
        }
        if (!jugador.gastarChatarra(COSTO))
        {
            return false;
        }
        ++mejora.nivelActual;
        if (!archivo.guardarCatalogo(catalogo))
        {
            jugador.ganarChatarra(COSTO);
            return false;
        }
        jugador.aumentarVidaMaxima(20);
        return true;
    }
    return false;
}

bool Juego::reiniciarProgreso()
{
    cancelarCompraEstacion();
    if (!jugador.restablecerProgreso())
    {
        return false;
    }
    iniciarNuevaPartida();
    return true;
}

struct Juego::EstadoGuardado
{
    explicit EstadoGuardado(const Juego& juego)
        : jugador(juego.jugador), oleadas(juego._gestorOleadas), partida(juego._partida),
          inventario(juego._inventarioArmas), encuentros(juego._encuentros),
          estacion(juego._estacionMejoras), generador(juego._generador),
          proyectiles(juego._proyectiles), recursos(juego.recursosSuelo),
          armas{{juego.armaPrueba, juego._metralletaPrueba, juego._escopetaPrueba,
                 juego._canonEnergiaPrueba}},
          angulos(juego._angulosArmas), reflejadas(juego._armasReflejadas),
          armaSeleccionada(juego._armaSeleccionada), cartas(juego._cartasDisponibles),
          eligiendoCarta(juego._estado == EstadoJuego::SubiendoNivel),
          guardada(juego._partidaGuardada), infinito(juego._modoInfinito),
          siguienteEnemigo(Enemigo::obtenerSiguienteId()),
          eliminados(juego._eliminadosRegistrados), costoCofre(juego._costoCofre),
          opcionReemplazo(static_cast<std::uint32_t>(juego._opcionReemplazo)),
          tiempoDano(juego.tiempoRestanteDanoContacto)
    {
    }

    Jugador jugador;
    GestorOleadas oleadas;
    Partida partida;
    InventarioArmas inventario;
    EncuentroAleatorio encuentros;
    EstacionMejoras estacion;
    std::mt19937 generador;
    std::vector<Proyectil> proyectiles;
    std::vector<RecursoSuelo> recursos;
    std::array<Arma, 4> armas;
    std::array<float, 2> angulos;
    std::array<bool, 2> reflejadas;
    TipoArmaPrueba armaSeleccionada;
    std::array<bool, 3> cartas;
    bool eligiendoCarta;
    bool guardada;
    bool infinito;
    std::uint64_t siguienteEnemigo;
    int eliminados;
    int costoCofre;
    std::uint32_t opcionReemplazo;
    float tiempoDano;

    ObjetoEncuentro* buscarEstacionSeleccionada()
    {
        if (!estacion.menuAbierto())
        {
            return nullptr;
        }
        const Posicion posicion = estacion.obtenerPosicion();
        ObjetoEncuentro* objeto = encuentros.buscarCercano(posicion, TipoEncuentro::Estacion);
        if (objeto != nullptr && objeto->posicion.x == posicion.x && objeto->posicion.y == posicion.y)
        {
            return objeto;
        }
        return nullptr;
    }

    bool transferirEstado(ArchivoPartidaEnCurso& archivo)
    {
        std::uint32_t firma = FIRMA_PARTIDA;
        std::uint32_t version = VERSION_PARTIDA;
        if (!archivo(firma, version) || firma != FIRMA_PARTIDA || version != VERSION_PARTIDA ||
            !archivo(jugador, oleadas, partida, inventario, encuentros, estacion, generador,
                     armas, angulos, reflejadas, armaSeleccionada, cartas, eligiendoCarta,
                     guardada, infinito, siguienteEnemigo, eliminados, costoCofre, opcionReemplazo, tiempoDano) ||
            !archivo.lista(proyectiles, 20000, []
            {
                return Proyectil({0, 0}, 0, 0, 1, 1, 0);
            }) ||
            !archivo.lista(recursos, 100000) || !archivo.finalizar())
        {
            return false;
        }
        const int indiceArma = static_cast<int>(armaSeleccionada);
        const auto& enemigos = oleadas.obtenerEnemigos();
        const bool idsValidos = siguienteEnemigo > 0 && siguienteEnemigo < (std::uint64_t{1} << 63) &&
            std::all_of(enemigos.begin(), enemigos.end(), [&](const Enemigo& enemigo)
            {
                return enemigo.obtenerIdInstancia() < siguienteEnemigo;
            });
        const bool algunaCarta = std::any_of(cartas.begin(), cartas.end(), [](bool carta) { return carta; });
        return indiceArma >= 0 && indiceArma < static_cast<int>(armas.size()) &&
            idsValidos && infinito == partida.fueVictoria() && (!guardada || infinito) &&
            eliminados >= 0 && costoCofre > 0 && opcionReemplazo <= 3 && tiempoDano >= 0.0f &&
            (!eligiendoCarta || (jugador.requiereElegirMejora() && algunaCarta)) &&
            (estacion.menuAbierto() ? buscarEstacionSeleccionada() != nullptr : opcionReemplazo == 3);
    }
};

bool Juego::RecursoSuelo::transferirEstado(ArchivoPartidaEnCurso& archivo)
{
    return archivo(tipo, cantidad, posicion) && cantidad > 0 &&
        (tipo == TipoRecurso::Experiencia || tipo == TipoRecurso::Chatarra);
}

bool Juego::guardarPartida()
{
    asegurarCarpetaDatos();
    const std::string ruta = obtenerRutaDatos(ARCHIVO_PARTIDA_EN_CURSO);
    const std::string temporal = ruta + "." + std::to_string(_getpid()) + ".tmp";
    EstadoGuardado estado(*this);
    std::unique_ptr<FILE, decltype(&fclose)> archivo(fopen(temporal.c_str(), "wb"), fclose);
    if (!archivo)
    {
        return false;
    }
    ArchivoPartidaEnCurso binario(archivo.get(), false);
    const bool escrito = estado.transferirEstado(binario);
    const bool cerrado = fclose(archivo.release()) == 0;
    std::error_code error;
    if (escrito && cerrado)
    {
        std::filesystem::rename(temporal, ruta, error);
        if (!error)
        {
            return true;
        }
    }
    std::filesystem::remove(temporal, error);
    return false;
}

bool Juego::reanudarPartida()
{
    const std::string ruta = obtenerRutaDatos(ARCHIVO_PARTIDA_EN_CURSO);
    std::unique_ptr<FILE, decltype(&fclose)> archivo(fopen(ruta.c_str(), "rb"), fclose);
    if (!archivo)
    {
        return false;
    }
    EstadoGuardado estado(*this);
    ArchivoPartidaEnCurso binario(archivo.get(), true);
    const bool leido = estado.transferirEstado(binario);
    const bool cerrado = fclose(archivo.release()) == 0;
    if (!leido || !cerrado)
    {
        return false;
    }
    // Una partida reanudada queda en memoria hasta la siguiente salida.
    // Asi un resultado registrado nunca se puede obtener otra vez del guardado anterior.
    std::error_code error;
    if (!std::filesystem::remove(ruta, error) || error)
    {
        return false;
    }

    Enemigo::reservarIdentificadores(estado.siguienteEnemigo);
    jugador = std::move(estado.jugador);
    _gestorOleadas = std::move(estado.oleadas);
    _partida = estado.partida;
    _inventarioArmas = estado.inventario;
    _encuentros = std::move(estado.encuentros);
    _estacionMejoras = std::move(estado.estacion);
    _estacionSeleccionada = _estacionMejoras.menuAbierto() ?
        _encuentros.buscarCercano(_estacionMejoras.obtenerPosicion(), TipoEncuentro::Estacion) : nullptr;
    _generador = estado.generador;
    _proyectiles = std::move(estado.proyectiles);
    recursosSuelo = std::move(estado.recursos);
    armaPrueba = estado.armas[0];
    _metralletaPrueba = estado.armas[1];
    _escopetaPrueba = estado.armas[2];
    _canonEnergiaPrueba = estado.armas[3];
    _angulosArmas = estado.angulos;
    _armasReflejadas = estado.reflejadas;
    _armaSeleccionada = estado.armaSeleccionada;
    _cartasDisponibles = estado.cartas;
    _partidaGuardada = estado.guardada;
    _modoInfinito = estado.infinito;
    _eliminadosRegistrados = estado.eliminados;
    _costoCofre = estado.costoCofre;
    _opcionReemplazo = estado.opcionReemplazo;
    tiempoRestanteDanoContacto = estado.tiempoDano;
    _estado = estado.eligiendoCarta ? EstadoJuego::SubiendoNivel : EstadoJuego::Jugando;
    _continuacionDisponible = false;
    _mensajeMenu.clear();
    _opcionMenu = 0;
    _errorGuardadoCarta = false;
    _aspectoJugador.reiniciar(jugador.getPosicion());
    _aspectoEnemigos.reiniciar();
    _escenario.actualizar(jugador.getPosicion());
    relojFotogramas.restart();
    return true;
}

bool Juego::confirmarNuevaPartida()
{
    const std::string ruta = obtenerRutaDatos(ARCHIVO_PARTIDA_EN_CURSO);
    const std::string anterior = ruta + "." + std::to_string(_getpid()) + ".anterior";
    std::error_code error;
    const bool habiaGuardado = std::filesystem::exists(ruta, error);
    if (error)
    {
        return false;
    }
    if (habiaGuardado)
    {
        std::filesystem::rename(ruta, anterior, error);
        if (error)
        {
            return false;
        }
    }
    if (!reiniciarProgreso())
    {
        if (habiaGuardado)
        {
            std::filesystem::rename(anterior, ruta, error);
        }
        return false;
    }
    if (habiaGuardado)
    {
        std::filesystem::remove(anterior, error);
    }
    _continuacionDisponible = false;
    _mensajeMenu.clear();
    return true;
}

void Juego::salirDelJuego()
{
    const bool partidaActiva = _estado == EstadoJuego::Jugando || _estado == EstadoJuego::Pausa ||
        _estado == EstadoJuego::SubiendoNivel;
    if (partidaActiva && !jugador.estaVivo())
    {
        actualizarPartida(0.0f);
    }
    if (partidaActiva && jugador.estaVivo() && !guardarPartida())
    {
        ventana.setTitle("No se pudo guardar la partida. Volve a intentar.");
        if (_estado == EstadoJuego::Jugando && !_estacionMejoras.menuAbierto())
        {
            manejarEscape();
            _opcionMenu = 1;
        }
        _mensajeMenu = "No se pudo guardar. Volve a intentar.";
        return;
    }
    ventana.close();
}

bool Juego::puedeContinuarEnInfinito() const
{
    return _estado == EstadoJuego::Resultado && _partida.fueVictoria() && !_modoInfinito;
}

void Juego::continuarEnInfinito()
{
    if (puedeContinuarEnInfinito())
    {
        _modoInfinito = true;
        _estado = EstadoJuego::Jugando;
        relojFotogramas.restart();
    }
}

sf::FloatRect Juego::areaBotonInfinito() const
{
    const sf::Vector2f tamanio = ventana.getDefaultView().getSize();
    const float ancho = std::min(540.0f, tamanio.x * 0.8f);
    const float alto = std::min(66.0f, tamanio.y * 0.10f);
    return {{(tamanio.x - ancho) / 2.0f, tamanio.y * 0.68f}, {ancho, alto}};
}

void Juego::procesarClicResultado(sf::Vector2i posicion)
{
    if (puedeContinuarEnInfinito() &&
        areaBotonInfinito().contains(ventana.mapPixelToCoords(posicion, ventana.getDefaultView())))
    {
        continuarEnInfinito();
    }
}

void Juego::dibujarResultado()
{
    ventana.setView(ventana.getDefaultView());
    ventana.clear(sf::Color(18, 23, 29));
    if (fuenteDanioDisponible)
    {
        const sf::Vector2f tamanio = ventana.getDefaultView().getSize();
        const char* resultado = _modoInfinito ? "FIN DEL MODO INFINITO" :
            (_partida.fueVictoria() ? "VICTORIA" : "DERROTA");
        const int duracion = _modoInfinito ? static_cast<int>(_gestorOleadas.tiempoDePartida()) :
            _partida.obtenerDuracionSegundos();
        const std::array<std::string, 7> lineas{{resultado,
            "Duracion: " + std::to_string(duracion) + " s",
            "Enemigos eliminados: " + std::to_string(_partida.obtenerEnemigosEliminados()),
            "Chatarra recogida: " + std::to_string(_partida.obtenerChatarraObtenida()),
            "Nivel alcanzado: " + std::to_string(_partida.obtenerNivelAlcanzado()),
            "Puntaje: " + std::to_string(_partida.obtenerPuntaje()),
            _partidaGuardada ? (_modoInfinito ? "Victoria registrada al derrotar al jefe" :
                "Resultado guardado") : "No se pudo guardar el resultado"}};
        for (std::size_t indice = 0; indice < lineas.size(); ++indice)
        {
            sf::Text texto(fuenteDanio, lineas[indice], indice == 0 ? 54 : 28);
            const sf::FloatRect area = texto.getLocalBounds();
            texto.setOrigin({area.position.x + area.size.x / 2.0f, area.position.y});
            texto.setPosition({tamanio.x / 2.0f, tamanio.y * (0.16f + indice * 0.07f)});
            texto.setFillColor(indice == 0 ? sf::Color(255, 190, 55) : sf::Color(220, 230, 235));
            ventana.draw(texto);
        }
        if (puedeContinuarEnInfinito())
        {
            const sf::FloatRect area = areaBotonInfinito();
            sf::RectangleShape boton(area.size);
            boton.setPosition(area.position);
            boton.setFillColor(sf::Color(62, 43, 22));
            boton.setOutlineColor(sf::Color(255, 190, 55));
            boton.setOutlineThickness(2.0f);
            ventana.draw(boton);

            sf::Text texto(fuenteDanio, "Continuar en modo infinito  [I]", 28);
            const sf::FloatRect limites = texto.getLocalBounds();
            texto.setOrigin(limites.position + limites.size / 2.0f);
            texto.setPosition(area.position + area.size / 2.0f);
            texto.setFillColor(sf::Color(255, 216, 135));
            ventana.draw(texto);
        }
        sf::Text ayuda(fuenteDanio, "Enter: volver a jugar   |   Escape: menu principal", 24);
        const sf::FloatRect limites = ayuda.getLocalBounds();
        ayuda.setOrigin({limites.position.x + limites.size.x / 2.0f, limites.position.y});
        ayuda.setPosition({tamanio.x / 2.0f, tamanio.y * 0.84f});
        ventana.draw(ayuda);
    }
    ventana.display();
}
