#pragma once

#include "Arma.h"
#include "AspectoProyectiles.h"
#include "Enemigo.h"
#include "Escenario.h"
#include "EstacionMejoras.h"
#include "EncuentroAleatorio.h"
#include "Jugador.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <cstddef>
#include <vector>
#include <string>
#include <random>

class Juego {
public:
    Juego();

    void ejecutar();

private:
    static constexpr std::size_t CANTIDAD_ENEMIGOS = 9;

    enum class TipoArmaPrueba
    {
        Pistola,
        Metralleta
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
    bool _metralletaAdquirida = false;
    std::array<int, 2> _danoCofres{};
    std::array<int, 2> _cadenciaCofres{};
    std::string _mensajeCofre;
    float _tiempoMensajeCofre = 0.0f;
    Jugador jugador;
    std::array<Enemigo, CANTIDAD_ENEMIGOS> enemigosPrueba;
    std::array<sf::RectangleShape, CANTIDAD_ENEMIGOS> figurasEnemigosPrueba;
    std::array<float, CANTIDAD_ENEMIGOS> tiemposReaparicion;
    AspectoProyectiles _aspectoProyectiles;
    std::vector<Proyectil> _proyectiles;
    Arma armaPrueba;
    Arma _metralletaPrueba;
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
    Arma& obtenerArmaSeleccionada();
    void generarRecompensa(const Enemigo& enemigo);
    void reiniciarPrueba();
    void intentarComprarMejora();
    void elegirMejoraEstacion(std::size_t indice);
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
