#pragma once

#include <SFML/Graphics.hpp>
#include "TiposCompartidos.h"

class Jugador {
private:
    Posicion _posicion;
    float _velocidad;
    int _vida;
    int _vidaMaxima;
    int _nivel;
    int _experiencia;
    int _experienciaSiguienteNivel;
    int _chatarra;
    bool _vivo;

    sf::RectangleShape _formaGrafica;  // Representacion visual para pruebas

    void validarLimites();
    void verificarSubidaNivel();

public:
    Jugador();
    void procesarEntrada();
    // TEMPORAL: Juego puede desactivar el limite fijo para probar la camara.
    // Adaptar esta opcion al modulo del responsable de Jugador cuando se integre.
    void actualizar(bool limitarALaVentana = true);
    void recibirDano(int cantidad);
    void ganarExperiencia(int cantidad);
    void ganarChatarra(int cantidad);
    // TEMPORAL: estacion de prueba; adaptar al modulo del responsable de Jugador.
    bool gastarChatarra(int cantidad);
    void aumentarVidaMaxima(int cantidad);
    void reiniciar();
    void dibujar(sf::RenderWindow& ventana);

    Posicion getPosicion() const;
    FormaColision getFormaColision() const;
    int getVida() const;
    int getVidaMaxima() const;
    int getNivel() const;
    int getExperiencia() const;
    int getChatarra() const;
    bool estaVivo() const;

    void setPosicion(Posicion pos);
    void cargarEstado(int vida, int nivel, int exp, int chatarra);
};
