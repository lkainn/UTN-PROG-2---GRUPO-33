#include "Proyectil.h"
#include "ArchivoPartidaEnCurso.h"

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
    if (!_activo || idEnemigo == 0 || yaImpacto(idEnemigo))
    {
        return false;
    }
    if (!_trayectoRebote)
    {
        --_impactosRestantes;
    }
    _trayectoRebote = false;
    if (!puedeRebotar())
    {
        terminarImpacto();
    }
    if (_activo)
    {
        _enemigosImpactados.push_back(idEnemigo);
    }
    return true;
}

void Proyectil::configurarRebotes(int cantidad)
{
    _rebotesRestantes = std::clamp(cantidad, 0, 3);
}

bool Proyectil::puedeRebotar() const
{
    return _activo && _rebotesRestantes > 0;
}

bool Proyectil::yaImpacto(std::uint64_t idEnemigo) const
{
    return std::find(_enemigosImpactados.begin(), _enemigosImpactados.end(), idEnemigo) !=
        _enemigosImpactados.end();
}

bool Proyectil::rebotarHacia(Posicion objetivo)
{
    const float dx = objetivo.x - _posicion.x;
    const float dy = objetivo.y - _posicion.y;
    const float distancia = std::hypot(dx, dy);
    const float velocidad = obtenerVelocidad();
    if (!puedeRebotar() || !std::isfinite(distancia) || distancia <= 0.0f || velocidad <= 0.0f)
    {
        return false;
    }
    _velocidadX = dx / distancia * velocidad;
    _velocidadY = dy / distancia * velocidad;
    _dano = std::max(1, static_cast<int>(_dano * 0.70f));
    --_rebotesRestantes;
    _trayectoRebote = true;
    return true;
}

void Proyectil::terminarImpacto()
{
    if (!_trayectoRebote && _impactosRestantes == 0)
    {
        desactivar();
    }
}

float Proyectil::obtenerDistanciaRestante() const
{
    return _distanciaRestante;
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

bool Proyectil::transferirEstado(ArchivoPartidaEnCurso& archivo)
{
    bool tieneAlcance = std::isfinite(_distanciaRestante);
    float distancia = tieneAlcance ? _distanciaRestante : 0.0f;
    if (!archivo(_posicion, _velocidadX, _velocidadY, _ancho, _alto, _dano, _activo,
                 _idArma, _impactosRestantes, _rebotesRestantes, _trayectoRebote, tieneAlcance, distancia) ||
        !archivo.lista(_enemigosImpactados, 64) || _ancho <= 0.0f || _alto <= 0.0f ||
        _dano < 0 || _idArma < 0 || _idArma > 4 || _impactosRestantes < 0 ||
        _impactosRestantes > 6 || _rebotesRestantes < 0 || distancia < 0.0f)
    {
        return false;
    }
    if (archivo.leyendo())
    {
        _distanciaRestante = tieneAlcance ? distancia : std::numeric_limits<float>::infinity();
    }
    return true;
}
