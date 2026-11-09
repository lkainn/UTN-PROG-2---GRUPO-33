#pragma once

#include "Arma.h"
#include "ArchivoArmas.h"
#include "AspectoProyectiles.h"
#include "AspectoArmas.h"
#include "AspectoJugador.h"
#include "AspectoEncuentros.h"
#include "AspectoEnemigos.h"
#include "GestorOleadas.h"
#include "Escenario.h"
#include "EstacionMejoras.h"
#include "EncuentroAleatorio.h"
#include "Jugador.h"
#include "Partida.h"
#include "ArchivoPartidas.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <cstddef>
#include <vector>
#include <string>
#include <random>
#include <cstdint>

class Juego {
public:
    Juego();

    void ejecutar();

private:
    explicit Juego(const ArchivoArmas::Catalogo& catalogoArmas);
    enum class EstadoJuego
    {
        Menu,
        Controles,
        Ranking,
        Jugando,
        Pausa,
        Resultado,
        SubiendoNivel,
        ConfirmarReinicio
    };
    EstadoJuego _estado = EstadoJuego::Menu;
    int _opcionMenu = 0;
    bool _continuacionDisponible = false;
    std::string _mensajeMenu;
    struct EstadoGuardado;
    bool guardarPartida();
    bool reanudarPartida();
    bool confirmarNuevaPartida();
    void salirDelJuego();
    void manejarEscape();
    void procesarEntradaMenu(sf::Keyboard::Scancode tecla);
    void dibujarMenuPrincipal();
    void cargarRanking();
    void dibujarRanking();
    void prepararCartas();
    void dibujarCartas();
    std::array<bool, 3> _cartasDisponibles{};
    bool _errorGuardadoCarta = false;
    std::vector<RegistroPartida> _ranking;

    enum class TipoArmaPrueba
    {
        Pistola,
        Metralleta,
        Escopeta,
        CanonEnergia
    };

    struct IndicadorDanio {
        int cantidad;
        Posicion posicion;
        float tiempoRestante;
    };

    enum class TipoRecurso {
        Experiencia,
        Chatarra
    };

    struct RecursoSuelo {
        TipoRecurso tipo;
        int cantidad;
        Posicion posicion;
        bool transferirEstado(ArchivoPartidaEnCurso& archivo);
    };

    struct IndicadorRecurso {
        TipoRecurso tipo;
        int cantidad;
        Posicion posicion;
        float tiempoRestante;
    };

    sf::RenderWindow ventana;
    Escenario _escenario;
    EstacionMejoras _estacionMejoras;
    std::mt19937 _generador{std::random_device{}()};
    EncuentroAleatorio _encuentros{_generador()};
    // Prestado del mapa; std::map conserva el nodo al generar otras zonas.
    ObjetoEncuentro* _estacionSeleccionada = nullptr;
    int _costoCofre = 15;
    InventarioArmas _inventarioArmas;
    std::size_t _opcionReemplazo = 3;
    std::string _mensajeCofre;
    float _tiempoMensajeCofre = 0.0f;
    Jugador jugador;
    GestorOleadas _gestorOleadas;
    Partida _partida{1};
    bool _partidaGuardada = false;
    bool _modoInfinito = false;
    int _eliminadosRegistrados = 0;
    AspectoProyectiles _aspectoProyectiles;
    AspectoArmas _aspectoArmas;
    AspectoJugador _aspectoJugador;
    AspectoEncuentros _aspectoEncuentros;
    AspectoEnemigos _aspectoEnemigos;
    std::array<float, 2> _angulosArmas{};
    std::array<bool, 2> _armasReflejadas{};
    std::vector<Proyectil> _proyectiles;
    Arma armaPrueba;
    Arma _metralletaPrueba;
    Arma _escopetaPrueba;
    Arma _canonEnergiaPrueba;
    TipoArmaPrueba _armaSeleccionada;
    sf::Font fuenteDanio;
    bool fuenteDanioDisponible;
    std::vector<IndicadorDanio> indicadoresDanio;
    std::vector<RecursoSuelo> recursosSuelo;
    std::vector<IndicadorRecurso> indicadoresRecurso;
    float tiempoRestanteDanoContacto;
    sf::Clock relojFotogramas;

    void procesarEventos();
    void actualizar();
    void actualizarEnemigos(float segundos);
    void actualizarPartida(float segundos);
    void iniciarNuevaPartida();
    bool comprarMejoraPermanente();
    bool reiniciarProgreso();
    void dibujarResultado();
    bool puedeContinuarEnInfinito() const;
    void continuarEnInfinito();
    sf::FloatRect areaBotonInfinito() const;
    void procesarClicResultado(sf::Vector2i posicion);
    Posicion obtenerAnclajeArma(std::size_t slot) const;
    Arma& obtenerArmaPorId(int idArma);
    const char* obtenerNombreArma(int idArma);
    void dispararArma(std::size_t slot);
    void actualizarDanoContacto(float segundos);
    void actualizarIndicadoresDanio(float segundos);
    void actualizarRecursos(float segundos);
    void actualizarProyectiles(float segundos);
    bool intentarRebote(Proyectil& proyectil, const std::vector<Enemigo>& enemigos);
    void dibujar();
    void dibujarRecursos();
    void dibujarIndicadoresDanio();
    void dibujarIndicadoresRecurso();
    void dibujarInterfaz();
    void actualizarDisparoAutomatico();
    bool buscarObjetivo(float alcance, Posicion& objetivo, float& distanciaCuadrada) const;
    void actualizarApuntadoArma(std::size_t slot, Posicion objetivo);
    void actualizarArmas(float segundos);
    void seleccionarArmaPrueba(TipoArmaPrueba tipo);
    void seleccionarSlotArma(std::size_t slot);
    Arma& obtenerArmaSeleccionada();
    void generarRecompensa(const Enemigo& enemigo);
    void reiniciarPrueba();
    void intentarComprarMejora();
    void elegirMejoraEstacion(std::size_t indice);
    void confirmarReemplazoPistola();
    void finalizarCompraEstacion(std::size_t indice, bool confirmarReemplazo);
    void cancelarCompraEstacion();
    void dibujarEstacionMejoras(const ObjetoEncuentro& objeto);
    void dibujarEncuentros();
    void dibujarMenuEstacion();
    void actualizarEncuentros(float segundos);
    void interactuarEncuentro();
    void intentarAbrirCofre();
    bool hayRecompensaCofre() const;
    void aplicarBonificacionesArmas();
    void dibujarCofre(const ObjetoEncuentro& objeto);
};
