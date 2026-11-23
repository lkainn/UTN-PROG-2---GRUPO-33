#pragma once

class ArchivoPartidaEnCurso;

#include <SFML/Graphics.hpp>
#include "TiposCompartidos.h"

// Estructura para representar las mejoras de progresion
struct Mejora {
    int idMejora = 0;
    char nombre[30] = "";
    int nivelActual = 0;
    int nivelMaximo = 1;
    char descripcion[100] = "";
    };

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
    int _mejorasPendientes = 0;
    int _rebotes = 0;

    sf::RectangleShape _formaGrafica;  // Representacion visual para pruebas

    void validarLimites();
    void moverEnDireccion(float direccionX, float direccionY);
    void verificarSubidaNivel();
    bool guardarProgreso();

public:
    bool transferirEstado(ArchivoPartidaEnCurso& archivo);
    Jugador();
    void procesarEntrada();
    void actualizar(bool limitarALaVentana = true);
    void recibirDano(int cantidad);
    void curar(int cantidad);
    void ganarExperiencia(int cantidad);
    void ganarChatarra(int cantidad);
    bool gastarChatarra(int cantidad);
    void aumentarVidaMaxima(int cantidad);
    void aumentarVelocidad(float cantidad);
    void reiniciar();
    void dibujar(sf::RenderWindow& ventana);
    bool restablecerProgreso();
    bool requiereElegirMejora() const;
    bool aplicarMejoraElegida(int idMejora);
    void omitirMejora();
    int getRebotes() const;

    Posicion getPosicion() const;
    FormaColision getFormaColision() const;
    int getVida() const;
    int getVidaMaxima() const;
    int getNivel() const;
    int getExperiencia() const;
    int getExperienciaSiguienteNivel() const;
    int getChatarra() const;
    float getVelocidad() const;
    bool estaVivo() const;

    // Metodos auxiliares para la interfaz grafica
    float getPorcentajeVida() const;
    float getPorcentajeExperiencia() const;

    void setPosicion(Posicion pos);
    void cargarEstado(int vida, int nivel, int exp, int chatarra);
};
