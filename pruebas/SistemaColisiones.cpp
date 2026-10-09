#include "SistemaColisiones.h"

#include <iostream>
#include <limits>

namespace {
bool comprobar(bool condicion, const char* caso) {
    if (!condicion) {
        std::cerr << "Fallo: " << caso << '\n';
    }
    return condicion;
}
}

int main() {
    const FormaColision base{10.0f, 10.0f, 20.0f, 20.0f};
    const FormaColision escenario{0.0f, 0.0f, 100.0f, 100.0f};
    bool correcto = true;

    correcto &= comprobar(
        SistemaColisiones::hayInterseccion(base, {25.0f, 25.0f, 10.0f, 10.0f}),
        "rectangulos superpuestos");
    correcto &= comprobar(
        !SistemaColisiones::hayInterseccion(base, {30.0f, 10.0f, 10.0f, 10.0f}),
        "bordes que solo se tocan");
    correcto &= comprobar(
        !SistemaColisiones::hayInterseccion(base, {50.0f, 50.0f, 10.0f, 10.0f}),
        "rectangulos separados");
    correcto &= comprobar(
        !SistemaColisiones::hayInterseccion(base, {20.0f, 20.0f, 0.0f, 10.0f}) &&
        !SistemaColisiones::hayInterseccion(base, {20.0f, 20.0f, -1.0f, 10.0f}),
        "dimensiones invalidas");
    correcto &= comprobar(
        !SistemaColisiones::hayInterseccion(
            base, {std::numeric_limits<float>::quiet_NaN(), 20.0f, 10.0f, 10.0f}),
        "coordenada no finita");
    correcto &= comprobar(
        SistemaColisiones::estaDentroDe({0.0f, 0.0f, 20.0f, 20.0f}, escenario) &&
        SistemaColisiones::estaDentroDe({80.0f, 80.0f, 20.0f, 20.0f}, escenario) &&
        !SistemaColisiones::estaDentroDe({90.0f, 90.0f, 20.0f, 20.0f}, escenario),
        "limites inclusivos y salida del escenario");
    correcto &= comprobar(
        SistemaColisiones::estaDentroDeRadio(
            {0.0f, 0.0f}, {3.0f, 4.0f}, 5.0f) &&
        !SistemaColisiones::estaDentroDeRadio(
            {0.0f, 0.0f}, {3.0f, 4.0f}, 4.9f),
        "radio de recoleccion inclusivo");
    correcto &= comprobar(
        !SistemaColisiones::estaDentroDeRadio(
            {0.0f, 0.0f}, {3.0f, 4.0f}, -1.0f) &&
        !SistemaColisiones::estaDentroDeRadio(
            {0.0f, 0.0f},
            {std::numeric_limits<float>::quiet_NaN(), 4.0f}, 5.0f),
        "radio o coordenadas invalidas");

    if (correcto) {
        std::cout << "SistemaColisiones: pruebas correctas\n";
    }
    return correcto ? 0 : 1;
}
