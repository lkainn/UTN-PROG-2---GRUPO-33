#pragma once

#include "Arma.h"
#include "Enemigo.h"
#include "Jugador.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <cstddef>
#include <vector>

class Juego {
public:
    Juego();

    void ejecutar();

private:
    static constexpr std::size_t CANTIDAD_ENEMIGOS = 3;

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
    Jugador jugador;
    std::array<Enemigo, CANTIDAD_ENEMIGOS> enemigosPrueba;
    std::array<sf::RectangleShape, CANTIDAD_ENEMIGOS> figurasEnemigosPrueba;
    std::array<float, CANTIDAD_ENEMIGOS> tiemposReaparicion;
    sf::RectangleShape figuraProyectilPrueba;
    Proyectil proyectilPrueba;
    Arma armaPrueba;
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
    void actualizarProyectil(float segundos);
    void dibujar();
    void dibujarRecursos();
    void dibujarIndicadoresDanio();
    void dibujarIndicadoresRecurso();
    void dibujarInterfaz();
    void actualizarDisparoAutomatico();
    void generarRecompensa(const Enemigo& enemigo);
    void reiniciarPrueba();
};
