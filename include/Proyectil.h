#pragma once

#include "TiposCompartidos.h"

//const despues del metodo es para que el metodo no pueda modificar los atributos de proyectil.

class Proyectil {
public:
    Proyectil(Posicion posicion, float velocidadX, float velocidadY,
              float ancho, float alto, int dano);

    void actualizar(float segundos);
    void desactivar();

    Posicion obtenerPosicion() const;
    FormaColision obtenerFormaColision() const;
    int obtenerDano() const;
    bool estaActivo() const;

private:
    Posicion _posicion;
    float _velocidadX;
    float _velocidadY;
    float _ancho;
    float _alto;
    int _dano;
    bool _activo;
};
