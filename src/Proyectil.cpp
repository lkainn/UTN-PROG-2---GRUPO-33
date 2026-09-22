#include "Proyectil.h"

#include <cmath>

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

    //distancia = velocidad * tiempo
    _posicion.x += _velocidadX * segundos;
    _posicion.y += _velocidadY * segundos;
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
