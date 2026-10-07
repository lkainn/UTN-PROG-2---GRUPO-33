#include "Proyectil.h"

#include "SistemaColisiones.h"

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
    Proyectil proyectil({10.0f, 20.0f}, 100.0f, -20.0f,
                        8.0f, 4.0f, 25);

    correcto &= comprobar(
        proyectil.estaActivo() && proyectil.obtenerDano() == 25,
        "estado y dano iniciales");
    correcto &= comprobar(
        proyectil.obtenerFormaColision().x == 10.0f &&
        proyectil.obtenerFormaColision().y == 20.0f &&
        proyectil.obtenerFormaColision().ancho == 8.0f &&
        proyectil.obtenerFormaColision().alto == 4.0f,
        "forma de colision inicial");

    proyectil.actualizar(0.5f);
    correcto &= comprobar(
        proyectil.obtenerPosicion().x == 60.0f &&
        proyectil.obtenerPosicion().y == 10.0f,
        "movimiento por tiempo");

    proyectil.desactivar();
    proyectil.actualizar(1.0f);
    correcto &= comprobar(
        !proyectil.estaActivo() &&
        proyectil.obtenerPosicion().x == 60.0f &&
        proyectil.obtenerPosicion().y == 10.0f,
        "proyectil inactivo no se mueve");

    Proyectil dimensionesInvalidas({0.0f, 0.0f}, 10.0f, 0.0f,
                                   -1.0f, 4.0f, 5);
    correcto &= comprobar(
        !dimensionesInvalidas.estaActivo() &&
        dimensionesInvalidas.obtenerFormaColision().ancho == 0.0f,
        "dimensiones invalidas desactivan el proyectil");

    Proyectil proyectilEnEscenario({90.0f, 20.0f}, 20.0f, 0.0f,
                                   5.0f, 5.0f, 10);
    const FormaColision escenario{0.0f, 0.0f, 100.0f, 100.0f};
    correcto &= comprobar(
        SistemaColisiones::estaDentroDe(
            proyectilEnEscenario.obtenerFormaColision(), escenario),
        "proyectil dentro del escenario");
    proyectilEnEscenario.actualizar(0.5f);
    correcto &= comprobar(
        !SistemaColisiones::estaDentroDe(
            proyectilEnEscenario.obtenerFormaColision(), escenario),
        "proyectil fuera del escenario");

    Proyectil limitado({0.0f, 0.0f}, 800.0f, 0.0f, 8.0f, 4.0f, 10);
    limitado.limitarAlcance(600.0f);
    limitado.actualizar(0.5f);
    correcto &= comprobar(limitado.estaActivo() && limitado.obtenerPosicion().x == 400.0f,
                          "proyectil sigue activo antes de agotar alcance");
    limitado.actualizar(0.5f);
    correcto &= comprobar(!limitado.estaActivo() && limitado.obtenerPosicion().x == 600.0f,
                          "paso grande termina exactamente en el alcance");
    limitado.actualizar(1.0f);
    correcto &= comprobar(limitado.obtenerPosicion().x == 600.0f,
                          "proyectil agotado no vuelve a moverse");
    Proyectil diagonal({-100.0f, -200.0f}, 300.0f, 400.0f, 8.0f, 4.0f, 10);
    diagonal.limitarAlcance(500.0f);
    diagonal.actualizar(2.0f);
    correcto &= comprobar(!diagonal.estaActivo() && diagonal.obtenerPosicion().x == 200.0f &&
                          diagonal.obtenerPosicion().y == 200.0f,
                          "rango diagonal consume distancia euclidiana desde origen negativo");
    Proyectil invalido({0.0f, 0.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 10);
    invalido.limitarAlcance(-1.0f);
    correcto &= comprobar(!invalido.estaActivo(), "alcance invalido desactiva proyectil");

    if (correcto) {
        std::cout << "Proyectil: pruebas correctas\n";
    }
    return correcto ? 0 : 1;
}
