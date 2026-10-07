#pragma once

#include "TiposCompartidos.h"
#include <limits>
#include <cstdint>
#include <vector>

//const despues del metodo es para que el metodo no pueda modificar los atributos de proyectil.

class Proyectil {
public:
    Proyectil(Posicion posicion, float velocidadX, float velocidadY,
              float ancho, float alto, int dano, int idArma = 0,
              int maximoImpactos = 1);

    void actualizar(float segundos);
    void desactivar();
    void limitarAlcance(float alcance);
    void desviar(float angulo);
    bool registrarImpacto(std::uint64_t idEnemigo);

    Posicion obtenerPosicion() const;
    FormaColision obtenerFormaColision() const;
    int obtenerDano() const;
    float obtenerAngulo() const;
    float obtenerVelocidad() const;
    int obtenerIdArma() const;
    bool estaActivo() const;

private:
    Posicion _posicion;
    float _velocidadX;
    float _velocidadY;
    float _ancho;
    float _alto;
    int _dano;
    bool _activo;
    float _distanciaRestante = std::numeric_limits<float>::infinity();
    int _idArma;
    int _impactosRestantes;
    std::vector<std::uint64_t> _enemigosImpactados;
};
