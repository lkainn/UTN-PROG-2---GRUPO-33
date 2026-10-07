#include "Arma.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

Arma::Arma(int id, const char* nombre, int dano, float velocidadAtaque,
           float alcance, int nivelMaximo, int cantidadProyectiles,
           float dispersion, int maximoImpactos)
    : _id(id > 0 ? id : 0),
      _nombre{},
      _dano(dano > 0 ? dano : 0),
      _velocidadAtaque(
          std::isfinite(velocidadAtaque) && velocidadAtaque > 0.0f
              ? velocidadAtaque
              : 0.0f),
      _alcance(std::isfinite(alcance) && alcance > 0.0f ? alcance : 0.0f),
      _nivelActual(nivelMaximo > 0 ? 1 : 0),
      _nivelMaximo(nivelMaximo > 0 ? nivelMaximo : 0),
      _tiempoRestante(0.0f),
      _cantidadProyectiles(std::clamp(cantidadProyectiles, 1, 9)),
      _dispersion(std::isfinite(dispersion) && dispersion >= 0.0f && dispersion <= 3.14159265f
          ? dispersion : 0.0f),
      _maximoImpactos(std::clamp(maximoImpactos, 1, 6)) {
    if (nombre != nullptr) {
        std::strncpy(_nombre, nombre, sizeof(_nombre) - 1);
        _nombre[sizeof(_nombre) - 1] = '\0';
    }
}

void Arma::actualizar(float segundos) {
    if (!std::isfinite(segundos) || segundos <= 0.0f) {
        return;
    }

    _tiempoRestante = std::max(0.0f, _tiempoRestante - segundos);
}

bool Arma::intentarDisparar(Proyectil& proyectil, Posicion origen,
                            Posicion objetivo, float velocidadProyectil,
                            float anchoProyectil, float altoProyectil) {
    if (!puedeDisparar() || proyectil.estaActivo() ||
        !std::isfinite(velocidadProyectil) || velocidadProyectil <= 0.0f) {
        return false;
    }

    const float diferenciaX = objetivo.x - origen.x;
    const float diferenciaY = objetivo.y - origen.y;
    const float distancia = std::hypot(diferenciaX, diferenciaY);
    if (!std::isfinite(distancia) || distancia <= 0.0f ||
        distancia > _alcance) {
        return false;
    }

    const float velocidadX = diferenciaX / distancia * velocidadProyectil;
    const float velocidadY = diferenciaY / distancia * velocidadProyectil;
    const Posicion posicionInicial{
        origen.x - anchoProyectil / 2.0f,
        origen.y - altoProyectil / 2.0f
    };

    Proyectil nuevoProyectil(posicionInicial, velocidadX, velocidadY,
                             anchoProyectil, altoProyectil, obtenerDano(), _id, obtenerMaximoImpactos());
    nuevoProyectil.limitarAlcance(_alcance);
    if (!nuevoProyectil.estaActivo()) {
        return false;
    }

    proyectil = nuevoProyectil;
    _tiempoRestante = 1.0f / obtenerVelocidadAtaque();
    return true;
}

bool Arma::intentarDispararRafaga(std::vector<Proyectil>& proyectiles, Posicion origen,
                                Posicion objetivo, float velocidadProyectil,
                                float anchoProyectil, float altoProyectil)
{
    Proyectil central({0.0f, 0.0f}, 0.0f, 0.0f, 0.0f, 0.0f, 0);
    if (!intentarDisparar(central, origen, objetivo, velocidadProyectil,
                         anchoProyectil, altoProyectil))
    {
        return false;
    }
    // El disparo central valida toda la descarga y consume una sola recarga.
    const int cantidad = obtenerCantidadProyectiles();
    const float dispersion = _dispersion > 0.0f ? _dispersion : 0.1745329f;
    for (int indice = 0; indice < cantidad; ++indice)
    {
        Proyectil perdigon = central;
        if (cantidad > 1)
        {
            const float angulo = -dispersion / 2.0f +
                dispersion * indice / (cantidad - 1);
            perdigon.desviar(angulo);
        }
        proyectiles.push_back(perdigon);
    }
    return true;
}

void Arma::reiniciar() {
    _tiempoRestante = 0.0f;
}

int Arma::obtenerId() const {
    return _id;
}

const char* Arma::obtenerNombre() const {
    return _nombre;
}

int Arma::obtenerDano() const {
    const long long porcentaje = 100LL + _porcentajeDano;
    if (_dano > static_cast<long long>(std::numeric_limits<int>::max()) * 100 / porcentaje)
    {
        return std::numeric_limits<int>::max();
    }
    const long long dano = (static_cast<long long>(_dano) * porcentaje + 99) / 100;
    return static_cast<int>(std::min(dano,
        static_cast<long long>(std::numeric_limits<int>::max())));
}

float Arma::obtenerVelocidadAtaque() const {
    return _velocidadAtaque * (1.0f + 0.01f * _porcentajeCadencia);
}

void Arma::configurarMejoras(int porcentajeDano, int porcentajeCadencia, int especiales)
{
    const float anterior = obtenerVelocidadAtaque();
    _porcentajeDano = std::max(0, porcentajeDano);
    _porcentajeCadencia = std::max(0, porcentajeCadencia);
    _especiales = std::clamp(especiales, 0, _id == 4 ? 3 : 4);
    if (anterior > 0.0f && obtenerVelocidadAtaque() > 0.0f)
    {
        _tiempoRestante *= anterior / obtenerVelocidadAtaque();
    }
}

float Arma::obtenerAlcance() const {
    return _alcance;
}

int Arma::obtenerCantidadProyectiles() const
{
    if (_id == 4)
    {
        return _cantidadProyectiles;
    }
    const int maximo = _id == 1 || _id == 2 ? 5 : 9;
    return std::min(_cantidadProyectiles + _especiales, maximo);
}

int Arma::obtenerMaximoImpactos() const
{
    return std::min(_maximoImpactos + (_id == 4 ? _especiales : 0), 6);
}

int Arma::obtenerNivelActual() const {
    return _nivelActual;
}

int Arma::obtenerNivelMaximo() const {
    return _nivelMaximo;
}

bool Arma::puedeDisparar() const {
    return _dano > 0 && _velocidadAtaque > 0.0f && _tiempoRestante <= 0.0f;
}
