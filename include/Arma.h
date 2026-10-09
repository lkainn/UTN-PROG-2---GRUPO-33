#pragma once

#include "Proyectil.h"

class Arma {
public:
    Arma(int id, const char* nombre, int dano, float velocidadAtaque,
         float alcance, int nivelMaximo, int cantidadProyectiles = 1,
         float dispersion = 0.0f, int maximoImpactos = 1);

    void actualizar(float segundos);
    bool intentarDisparar(Proyectil& proyectil, Posicion origen,
                          Posicion objetivo, float velocidadProyectil,
                          float anchoProyectil, float altoProyectil);
    bool intentarDispararRafaga(std::vector<Proyectil>& proyectiles, Posicion origen,
                               Posicion objetivo, float velocidadProyectil,
                               float anchoProyectil, float altoProyectil);
    void reiniciar();
    void configurarMejoras(int porcentajeDano, int porcentajeCadencia, int especiales);

    int obtenerId() const;
    const char* obtenerNombre() const;
    int obtenerDano() const;
    float obtenerVelocidadAtaque() const;
    float obtenerAlcance() const;
    int obtenerCantidadProyectiles() const;
    int obtenerMaximoImpactos() const;
    int obtenerNivelActual() const;
    int obtenerNivelMaximo() const;
    bool puedeDisparar() const;

private:
    int _id;
    char _nombre[30];
    int _dano;
    float _velocidadAtaque;
    float _alcance;
    int _nivelActual;
    int _nivelMaximo;
    float _tiempoRestante;
    int _porcentajeDano = 0;
    int _porcentajeCadencia = 0;
    int _especiales = 0;
    int _cantidadProyectiles;
    float _dispersion;
    int _maximoImpactos;
};
