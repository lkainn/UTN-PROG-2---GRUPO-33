#include "Proyectil.h"

#include <cmath>
#include <algorithm>

float Proyectil::obtenerAngulo() const
{
    return std::atan2(_velocidadY, _velocidadX);
}

Proyectil::Proyectil(Posicion posicion, float velocidadX, float velocidadY,
                     float ancho, float alto, int dano)
    : _posicion(posicion),
      _velocidadX(velocidadX),
      _velocidadY(velocidadY),
      _ancho(std::isfinite(ancho) && ancho > 0.0f ? ancho : 0.0f),
      _alto(std::isfinite(alto) && alto > 0.0f ? alto : 0.0f),
      _dano(dano),
      _activo(std::isfinite(posicion.x) && std::isfinite(posicion.y) &&
              std::isfinite(velocidadX) && std::isfinite(velocidadY) &&
              _ancho > 0.0f && _alto > 0.0f) {
}

void Proyectil::actualizar(float segundos) {
    if (!_activo || !std::isfinite(segundos) || segundos <= 0.0f) {
        return;
    }

    const float paso = std::hypot(_velocidadX, _velocidadY) * segundos;
    if (!std::isfinite(paso))
    {
        desactivar();
        return;
    }
    if (paso <= 0.0f)
    {
        return;
    }
    const float fraccion = std::min(1.0f, _distanciaRestante / paso);
    _posicion.x += _velocidadX * segundos * fraccion;
    _posicion.y += _velocidadY * segundos * fraccion;
    _distanciaRestante = std::max(0.0f, _distanciaRestante - paso);
    if (_distanciaRestante <= 0.0f)
    {
        desactivar();
    }
}

void Proyectil::limitarAlcance(float alcance)
{
    if (!std::isfinite(alcance) || alcance <= 0.0f)
    {
        desactivar();
        return;
    }
    _distanciaRestante = alcance;
}

void Proyectil::desactivar() {
    _activo = false;
}

Posicion Proyectil::obtenerPosicion() const {
    return _posicion;
}

FormaColision Proyectil::obtenerFormaColision() const {
    return {_posicion.x, _posicion.y, _ancho, _alto};
}

int Proyectil::obtenerDano() const {
    return _dano;
}

bool Proyectil::estaActivo() const {
    return _activo;
}
