#include "Proyectil.h"

#include <cmath>
#include <algorithm>

float Proyectil::obtenerAngulo() const
{
    return std::atan2(_velocidadY, _velocidadX);
}

Proyectil::Proyectil(Posicion posicion, float velocidadX, float velocidadY,
                     float ancho, float alto, int dano, int idArma,
                     int maximoImpactos)
    : _posicion(posicion),
      _velocidadX(velocidadX),
      _velocidadY(velocidadY),
      _ancho(std::isfinite(ancho) && ancho > 0.0f ? ancho : 0.0f),
      _alto(std::isfinite(alto) && alto > 0.0f ? alto : 0.0f),
      _dano(dano),
      _activo(std::isfinite(posicion.x) && std::isfinite(posicion.y) &&
              std::isfinite(velocidadX) && std::isfinite(velocidadY) &&
              _ancho > 0.0f && _alto > 0.0f),
      _idArma(idArma >= 0 ? idArma : 0),
      _impactosRestantes(std::clamp(maximoImpactos, 1, 6)),
      _enemigosImpactados{} {
}

void Proyectil::desviar(float angulo)
{
    if (!_activo || !std::isfinite(angulo))
    {
        return;
    }
    const float direccion = obtenerAngulo() + angulo;
    const float velocidad = obtenerVelocidad();
    _velocidadX = std::cos(direccion) * velocidad;
    _velocidadY = std::sin(direccion) * velocidad;
}

bool Proyectil::registrarImpacto(std::uint64_t idEnemigo)
{
    if (!_activo || idEnemigo == 0 ||
        std::find(_enemigosImpactados.begin(), _enemigosImpactados.end(), idEnemigo) !=
            _enemigosImpactados.end())
    {
        return false;
    }
    --_impactosRestantes;
    if (_impactosRestantes == 0)
    {
        desactivar();
    }
    else
    {
        _enemigosImpactados.push_back(idEnemigo);
    }
    return true;
}

float Proyectil::obtenerVelocidad() const
{
    return std::hypot(_velocidadX, _velocidadY);
}

int Proyectil::obtenerIdArma() const
{
    return _idArma;
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
