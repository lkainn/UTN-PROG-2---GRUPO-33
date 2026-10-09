#include "Arma.h"

#include <cstring>
#include <iostream>

namespace {
bool comprobar(bool condicion, const char* caso) {
    if (!condicion) {
        std::cerr << "Fallo: " << caso << '\n';
    }
    return condicion;
}
}

int main() {
    bool correcto = true;
    Arma arma(1, "Pistola basica", 25, 2.0f, 500.0f, 5);
    Proyectil proyectil({0.0f, 0.0f}, 0.0f, 0.0f,
                        0.0f, 0.0f, 0);

    correcto &= comprobar(
        arma.obtenerId() == 1 &&
        std::strcmp(arma.obtenerNombre(), "Pistola basica") == 0 &&
        arma.obtenerDano() == 25 &&
        arma.obtenerVelocidadAtaque() == 2.0f &&
        arma.obtenerAlcance() == 500.0f &&
        arma.obtenerNivelActual() == 1 &&
        arma.obtenerNivelMaximo() == 5,
        "estado inicial del arma");

    correcto &= comprobar(
        arma.intentarDisparar(proyectil, {100.0f, 100.0f},
                              {400.0f, 100.0f}, 100.0f, 8.0f, 4.0f) &&
        proyectil.estaActivo() && proyectil.obtenerDano() == 25 &&
        proyectil.obtenerPosicion().x == 96.0f &&
        proyectil.obtenerPosicion().y == 98.0f,
        "primer disparo inmediato y centrado");

    proyectil.actualizar(0.1f);
    correcto &= comprobar(
        proyectil.obtenerPosicion().x == 106.0f &&
        proyectil.obtenerPosicion().y == 98.0f,
        "direccion horizontal hacia el objetivo");

    correcto &= comprobar(
        !arma.intentarDisparar(proyectil, {100.0f, 100.0f},
                               {400.0f, 100.0f}, 100.0f, 8.0f, 4.0f) &&
        proyectil.obtenerPosicion().x == 106.0f,
        "proyectil activo no se sobrescribe");

    proyectil.desactivar();
    correcto &= comprobar(
        !arma.intentarDisparar(proyectil, {100.0f, 100.0f},
                               {400.0f, 100.0f}, 100.0f, 8.0f, 4.0f),
        "cooldown bloquea un disparo anticipado");

    arma.actualizar(0.5f);
    correcto &= comprobar(
        arma.intentarDisparar(proyectil, {100.0f, 100.0f},
                              {400.0f, 500.0f}, 100.0f, 8.0f, 4.0f),
        "cooldown habilita el siguiente disparo");

    proyectil.actualizar(1.0f);
    correcto &= comprobar(
        proyectil.obtenerPosicion().x == 156.0f &&
        proyectil.obtenerPosicion().y == 178.0f,
        "direccion diagonal normalizada");

    proyectil.desactivar();
    arma.actualizar(0.5f);
    correcto &= comprobar(
        !arma.intentarDisparar(proyectil, {100.0f, 100.0f},
                               {700.0f, 100.0f}, 100.0f, 8.0f, 4.0f) &&
        arma.puedeDisparar(),
        "objetivo fuera de alcance no consume el cooldown");

    correcto &= comprobar(
        !arma.intentarDisparar(proyectil, {100.0f, 100.0f},
                               {100.0f, 100.0f}, 100.0f, 8.0f, 4.0f) &&
        arma.puedeDisparar(),
        "origen y objetivo coincidentes no producen un disparo");

    correcto &= comprobar(
        !arma.intentarDisparar(proyectil, {100.0f, 100.0f},
                               {400.0f, 100.0f}, -100.0f, 8.0f, 4.0f) &&
        arma.puedeDisparar(),
        "velocidad invalida no consume el cooldown");

    Arma armaInvalida(0, nullptr, -10, 0.0f, -1.0f, 0);
    correcto &= comprobar(
        armaInvalida.obtenerId() == 0 &&
        std::strcmp(armaInvalida.obtenerNombre(), "") == 0 &&
        armaInvalida.obtenerNivelActual() == 0 &&
        !armaInvalida.puedeDisparar(),
        "datos invalidos dejan el arma inhabilitada");

    Arma armaPantalla(2, "Arma de pantalla", 25, 2.0f, 2210.0f, 5);
    Proyectil proyectilDiagonal({0.0f, 0.0f}, 0.0f, 0.0f,
                                0.0f, 0.0f, 0);
    correcto &= comprobar(
        armaPantalla.intentarDisparar(
            proyectilDiagonal, {0.0f, 0.0f}, {1920.0f, 1080.0f},
            800.0f, 8.0f, 4.0f),
        "alcance suficiente para cubrir la diagonal de la pantalla");

    if (correcto) {
        std::cout << "Arma: pruebas correctas\n";
    }
    return correcto ? 0 : 1;
}
