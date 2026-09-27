#include "Arma.h"

#include <algorithm>
#include <cmath>
#include <cstring>

Arma::Arma(int id, const char* nombre, int dano, float velocidadAtaque,
           float alcance, int nivelMaximo)
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
      _tiempoRestante(0.0f) {
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
                             anchoProyectil, altoProyectil, _dano);
    if (!nuevoProyectil.estaActivo()) {
        return false;
    }

    proyectil = nuevoProyectil;
    _tiempoRestante = 1.0f / _velocidadAtaque;
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
    return _dano;
}

float Arma::obtenerVelocidadAtaque() const {
    return _velocidadAtaque;
}

float Arma::obtenerAlcance() const {
    return _alcance;
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
