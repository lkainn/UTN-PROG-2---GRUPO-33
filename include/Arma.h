#pragma once

#include "Proyectil.h"

class Arma {
public:
    Arma(int id, const char* nombre, int dano, float velocidadAtaque,
         float alcance, int nivelMaximo);

    void actualizar(float segundos);
    bool intentarDisparar(Proyectil& proyectil, Posicion origen,
                          Posicion objetivo, float velocidadProyectil,
                          float anchoProyectil, float altoProyectil);
    void reiniciar();
    void configurarBonificaciones(int nivelDano, int nivelCadencia);

    int obtenerId() const;
    const char* obtenerNombre() const;
    int obtenerDano() const;
    float obtenerVelocidadAtaque() const;
    float obtenerAlcance() const;
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
    int _nivelDano = 0;
    int _nivelCadencia = 0;
};
