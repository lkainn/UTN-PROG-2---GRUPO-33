#include "Juego.h"

#include "Configuracion.h"
#include "SistemaColisiones.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <cmath>

//std::min() recibe dos valores y devuelve el menor.

namespace {
constexpr float VELOCIDAD_PROYECTIL = 800.0f;
constexpr float ALCANCE_ARMA = 600.0f;
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
      _escenario{},
      _gestorOleadas{},
      _aspectoProyectiles{},
      _proyectiles{},
      armaPrueba(1, "Pistola basica", 25, 2.0f, ALCANCE_ARMA, 5),
      _metralletaPrueba(2, "Metralleta", 6, 8.0f, ALCANCE_ARMA, 5),
      _escopetaPrueba(3, "Escopeta", 12, 0.8f, 300.0f, 5, 5, DISPERSION_ESCOPETA),
      _canonEnergiaPrueba(4, "Canon de energia", 60, 0.5f, 800.0f, 5, 1, 0.0f, 3),
      _armaSeleccionada(TipoArmaPrueba::Pistola),
      fuenteDanioDisponible(cargarFuenteDanio(fuenteDanio)),
      indicadoresDanio{},
      recursosSuelo{},
      indicadoresRecurso{},
      tiempoRestanteDanoContacto(0.0f) {
    ventana.setFramerateLimit(Configuracion::LIMITE_FPS);
    ventana.setKeyRepeatEnabled(false);

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
                if (_estacionMejoras.menuAbierto())
                {
                    cancelarCompraEstacion();
                }
                else
                {
                    ventana.close();
                }
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
            }
            else if (tecla->scancode == sf::Keyboard::Scancode::Num1 ||
                     tecla->scancode == sf::Keyboard::Scancode::Numpad1)
            {
                seleccionarSlotArma(0);
            }
            else if (tecla->scancode == sf::Keyboard::Scancode::Num2 ||
                     tecla->scancode == sf::Keyboard::Scancode::Numpad2)
            {
                seleccionarSlotArma(1);
            }
        }
    }
}

void Juego::actualizar() {
    // Limitar el salto reduce el riesgo de atravesar al enemigo si la ventana se pausa.
    const float segundos = std::min(relojFotogramas.restart().asSeconds(), 0.05f);
    if (_estacionMejoras.menuAbierto())
    {
        return;
    }
    actualizarArmas(segundos);
    jugador.procesarEntrada();
    jugador.actualizar(false);
    _escenario.actualizar(jugador.getPosicion());
    actualizarEncuentros(segundos);
    actualizarEnemigos(segundos);
    actualizarDanoContacto(segundos);
    actualizarIndicadoresDanio(segundos);
    actualizarProyectiles(segundos);
    actualizarRecursos(segundos);
    actualizarDisparoAutomatico();
}

void Juego::actualizarEnemigos(float segundos)
{
    if (jugador.estaVivo())
    {
        sincronizarIdentificadoresEnemigos();
        const auto& enemigos = _gestorOleadas.obtenerEnemigos();
        std::size_t vivos = 0;
        // GestorOleadas agrega al final y conserva el orden al quitar muertos.
        for (std::size_t indice = 0; indice < enemigos.size(); ++indice)
        {
            if (enemigos[indice].estaVivo())
            {
                _identificadoresEnemigos[vivos] = _identificadoresEnemigos[indice];
                ++vivos;
            }
        }
        _identificadoresEnemigos.resize(vivos);
        _gestorOleadas.actualizar(jugador.getPosicion(), segundos);
        sincronizarIdentificadoresEnemigos();
    }
}

void Juego::sincronizarIdentificadoresEnemigos()
{
    const std::size_t cantidad = _gestorOleadas.cantidadEnemigos();
    if (_identificadoresEnemigos.size() > cantidad)
    {
        _identificadoresEnemigos.resize(cantidad);
    }
    while (_identificadoresEnemigos.size() < cantidad)
    {
        _identificadoresEnemigos.push_back(_siguienteIdEnemigo);
        ++_siguienteIdEnemigo;
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
    sincronizarIdentificadoresEnemigos();
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
                    proyectil.registrarImpacto(_identificadoresEnemigos[indice]))
                {
                    enemigo.recibirDanio(proyectil.obtenerDano());
                    if (!enemigo.estaVivo())
                    {
                        generarRecompensa(enemigo);
                    }
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
void Juego::dibujar() {
    ventana.setView(_escenario.obtenerVista());
    ventana.clear(sf::Color(28, 31, 36));
    _escenario.dibujar(ventana);
    dibujarEncuentros();
    dibujarRecursos();
    for (const Enemigo& enemigo : _gestorOleadas.obtenerEnemigos())
    {
        if (enemigo.estaVivo()) {
            sf::RectangleShape figura;
            configurarFiguraEnemigo(figura, enemigo, enemigo.obtenerIdEnemigo());
            ventana.draw(figura);
        }
    }
    jugador.dibujar(ventana);
    const FormaColision formaJugador = jugador.getFormaColision();
    _aspectoArmas.dibujar(ventana,
        {formaJugador.x + formaJugador.ancho / 2.0f,
         formaJugador.y + formaJugador.alto / 2.0f},
        _anguloArma, obtenerArmaSeleccionada().obtenerId());
    for (const Proyectil& proyectil : _proyectiles)
    {
        _aspectoProyectiles.dibujar(ventana, proyectil);
    }
    dibujarIndicadoresDanio();
    dibujarIndicadoresRecurso();
    ventana.setView(ventana.getDefaultView());
    dibujarInterfaz();
    dibujarMenuEstacion();
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

    sf::RectangleShape fondo({1250.0f, 44.0f});
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
    sf::Text oleada(
        fuenteDanio, "OLEADA: " + std::to_string(_gestorOleadas.oleadaActual()) +
        " | TIEMPO: " + std::to_string(static_cast<int>(_gestorOleadas.tiempoDePartida())) +
        " s", 22);
    oleada.setPosition({900.0f, 22.0f});
    ventana.draw(oleada);
    std::string slots = "1: " + std::string(InventarioArmas::obtenerNombreArma(_inventarioArmas.obtenerIdSlot(0))) +
        (_inventarioArmas.obtenerIdSlot(0) == 1 ? " (reemplazable)" : " (fija)") +
        " | 2: " + InventarioArmas::obtenerNombreArma(_inventarioArmas.obtenerIdSlot(1));
    if (_inventarioArmas.obtenerIdSlot(1) != 0)
    {
        slots += " (fija)";
    }
    sf::Text inventario(fuenteDanio, slots, 20);
    inventario.setPosition({26.0f, 60.0f});
    inventario.setOutlineThickness(2.0f);
    inventario.setOutlineColor(sf::Color::Black);
    ventana.draw(inventario);
    if (_tiempoMensajeCofre > 0.0f)
    {
        sf::Text mensaje(fuenteDanio, _mensajeCofre, 26);
        mensaje.setPosition({26.0f, 92.0f});
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
        _mensajeCofre = "COFRE: +40 experiencia para el personaje";
    }
    else
    {
        jugador.aumentarVidaMaxima(20);
        _mensajeCofre = "COFRE: Blindaje +20 vida maxima y actual";
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
    sf::RectangleShape cuerpo({64.0f, 42.0f});
    cuerpo.setOrigin({32.0f, 21.0f});
    cuerpo.setPosition({posicion.x, posicion.y});
    cuerpo.setFillColor(objeto.usado ? sf::Color(45, 32, 24) : sf::Color(115, 75, 40));
    cuerpo.setOutlineThickness(cerca ? 3.0f : 1.0f);
    cuerpo.setOutlineColor(puedeAbrir ? sf::Color(255, 220, 95) : sf::Color(150, 150, 150));
    ventana.draw(cuerpo);
    if (objeto.usado)
    {
        sf::RectangleShape tapa({64.0f, 12.0f});
        tapa.setPosition({posicion.x - 32.0f, posicion.y - 38.0f});
        tapa.setFillColor(sf::Color(100, 65, 35));
        ventana.draw(tapa);
    }
    else
    {
        sf::RectangleShape cerradura({12.0f, 18.0f});
        cerradura.setPosition({posicion.x - 6.0f, posicion.y - 9.0f});
        cerradura.setFillColor(sf::Color(225, 175, 65));
        ventana.draw(cerradura);
    }
    if (fuenteDanioDisponible)
    {
        std::string etiqueta = objeto.usado ? "COFRE ABIERTO - VACIO" :
            "COFRE: " + std::to_string(_costoCofre) + " CHATARRA";
        if (cerca && !objeto.usado)
        {
            etiqueta += !disponible ? " | Arma al maximo" :
                (puedeAbrir ? " | F: abrir" : " | Falta chatarra");
        }
        sf::Text texto(fuenteDanio, etiqueta, 18);
        texto.setPosition({posicion.x - 100.0f, posicion.y + 35.0f});
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

void Juego::actualizarDisparoAutomatico()
{
    Arma& arma = obtenerArmaSeleccionada();
    if (!jugador.estaVivo() || _estacionMejoras.menuAbierto())
    {
        return;
    }

    const FormaColision formaJugador = jugador.getFormaColision();
    const Posicion centroJugador{
        formaJugador.x + formaJugador.ancho / 2.0f,
        formaJugador.y + formaJugador.alto / 2.0f
    };

    bool hayObjetivo = false;
    float distanciaMinima = std::numeric_limits<float>::max();
    const FormaColision areaVisible = _escenario.obtenerAreaVisible();
    Posicion centroObjetivo{0.0f, 0.0f};
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
            distanciaCuadrada > arma.obtenerAlcance() * arma.obtenerAlcance() ||
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

    if (!hayObjetivo) {
        return;
    }

    const float diferenciaX = centroObjetivo.x - centroJugador.x;
    const float diferenciaY = centroObjetivo.y - centroJugador.y;
    if (diferenciaX != 0.0f || diferenciaY != 0.0f)
    {
        _anguloArma = std::atan2(diferenciaY, diferenciaX);
    }
    if (!arma.puedeDisparar())
    {
        return;
    }

    const bool esMetralleta = _armaSeleccionada == TipoArmaPrueba::Metralleta;
    float ancho = esMetralleta ? ANCHO_PROYECTIL_METRALLETA : ANCHO_PROYECTIL_PISTOLA;
    float alto = esMetralleta ? ALTO_PROYECTIL_METRALLETA : ALTO_PROYECTIL_PISTOLA;
    float velocidad = VELOCIDAD_PROYECTIL;
    if (_armaSeleccionada == TipoArmaPrueba::Escopeta)
    {
        ancho = 8.0f;
        alto = 8.0f;
        velocidad = 650.0f;
    }
    else if (_armaSeleccionada == TipoArmaPrueba::CanonEnergia)
    {
        ancho = 40.0f;
        alto = 10.0f;
        velocidad = 1000.0f;
    }
    arma.intentarDispararRafaga(_proyectiles, centroJugador, centroObjetivo,
                              velocidad, ancho, alto);
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
    switch (_armaSeleccionada)
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

void Juego::reiniciarPrueba() {
    cancelarCompraEstacion();
    jugador.reiniciar();
    _estacionSeleccionada = nullptr;
    _opcionReemplazo = 3;
    _encuentros.generarAlrededor(jugador.getPosicion());
    _mensajeCofre.clear();
    _tiempoMensajeCofre = 0.0f;
    _escenario.actualizar(jugador.getPosicion());
    _gestorOleadas.reiniciar();
    _identificadoresEnemigos.clear();
    _siguienteIdEnemigo = 1;
    _proyectiles.clear();
    armaPrueba.reiniciar();
    _metralletaPrueba.reiniciar();
    _escopetaPrueba.reiniciar();
    _canonEnergiaPrueba.reiniciar();
    seleccionarSlotArma(0);
    _anguloArma = 0.0f;
    indicadoresDanio.clear();
    recursosSuelo.clear();
    indicadoresRecurso.clear();
    tiempoRestanteDanoContacto = 0.0f;
    ventana.setTitle(
        "WASD: mover | F: estacion/cofre | 1/2: slots de armas | R: reiniciar prueba");
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
    ventana.setTitle("WASD: mover | F: estacion/cofre | 1/2: slots de armas | R: reiniciar prueba");
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
    ventana.setTitle("WASD: mover | F: estacion/cofre | 1/2: slots de armas | R: reiniciar prueba");
    relojFotogramas.restart();
}

void Juego::dibujarEstacionMejoras(const ObjetoEncuentro& objeto)
{
    const Posicion posicion = objeto.posicion;
    const bool cerca = SistemaColisiones::estaDentroDeRadio(jugador.getPosicion(), posicion, 100.0f);
    const bool puedeComprar = !objeto.usado && cerca && jugador.estaVivo() &&
        !_estacionMejoras.menuAbierto() && jugador.getChatarra() >= _estacionMejoras.obtenerCosto();
    sf::RectangleShape cuerpo({56.0f, 68.0f});
    cuerpo.setOrigin({28.0f, 34.0f});
    cuerpo.setPosition({posicion.x, posicion.y});
    cuerpo.setFillColor(sf::Color(55, 67, 72));
    cuerpo.setOutlineThickness(cerca ? 3.0f : 1.0f);
    cuerpo.setOutlineColor(puedeComprar ? sf::Color(85, 235, 165) : sf::Color(185, 135, 65));
    ventana.draw(cuerpo);
    sf::RectangleShape pantalla({36.0f, 24.0f});
    pantalla.setPosition({posicion.x - 18.0f, posicion.y - 23.0f});
    pantalla.setFillColor(objeto.usado ? sf::Color(15, 22, 24) : sf::Color(30, 125, 100));
    ventana.draw(pantalla);
    sf::RectangleShape ranura({30.0f, 5.0f});
    ranura.setPosition({posicion.x - 15.0f, posicion.y + 14.0f});
    ranura.setFillColor(sf::Color(15, 20, 24));
    ventana.draw(ranura);
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
    const std::string slots = "Slot 1: " + std::string(InventarioArmas::obtenerNombreArma(_inventarioArmas.obtenerIdSlot(0))) +
        (_inventarioArmas.obtenerIdSlot(0) == 1 ? " (solo la pistola se puede reemplazar)" : " (fijo)") +
        " | Slot 2: " + InventarioArmas::obtenerNombreArma(_inventarioArmas.obtenerIdSlot(1)) +
        (_inventarioArmas.obtenerIdSlot(1) == 0 ? "" : " (fijo)");
    sf::Text inventario(fuenteDanio, slots, 22);
    inventario.setPosition({300.0f, 315.0f});
    ventana.draw(inventario);
    if (_opcionReemplazo < 3)
    {
        const BonificacionesArma bonos = _inventarioArmas.obtenerBonificaciones(1);
        sf::Text advertencia(fuenteDanio,
            "REEMPLAZAR PISTOLA: ELECCION DEFINITIVA\n\n" +
            _estacionMejoras.describirOpcion(_opcionReemplazo) +
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
                        _estacionMejoras.describirOpcion(i), 24);
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
