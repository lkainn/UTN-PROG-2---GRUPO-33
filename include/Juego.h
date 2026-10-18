#pragma once

#include "Arma.h"
#include "AspectoProyectiles.h"
#include "AspectoArmas.h"
#include "AspectoEncuentros.h"
#include "GestorOleadas.h"
#include "Escenario.h"
#include "EstacionMejoras.h"
#include "EncuentroAleatorio.h"
#include "Jugador.h"
#include "Partida.h"

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
    enum class EstadoJuego
    {
        Menu,
        Controles,
        Jugando,
        Pausa,
        Resultado
    };
    EstadoJuego _estado = EstadoJuego::Menu;
    int _opcionMenu = 0;
    void manejarEscape();
    void procesarEntradaMenu(sf::Keyboard::Scancode tecla);
    void dibujarMenuPrincipal();

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
    int _eliminadosRegistrados = 0;
    AspectoProyectiles _aspectoProyectiles;
    AspectoArmas _aspectoArmas;
    AspectoEncuentros _aspectoEncuentros;
    std::array<float, 2> _angulosArmas{};
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
    void dibujarResultado();
    Posicion obtenerAnclajeArma(std::size_t slot) const;
    Arma& obtenerArmaPorId(int idArma);
    void dispararArma(std::size_t slot);
    void actualizarDanoContacto(float segundos);
    void actualizarIndicadoresDanio(float segundos);
    void actualizarRecursos(float segundos);
    void actualizarProyectiles(float segundos);
    void dibujar();
    void dibujarRecursos();
    void dibujarIndicadoresDanio();
    void dibujarIndicadoresRecurso();
    void dibujarInterfaz();
    void actualizarDisparoAutomatico();
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
