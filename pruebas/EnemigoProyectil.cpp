#include "Enemigo.h"
#include "Proyectil.h"
#include "SistemaColisiones.h"

#include <iostream>

namespace {
bool verificar(bool condicion, const char* mensaje) {
    if (!condicion) {
        std::cerr << "Fallo: " << mensaje << '\n';
    }
    return condicion;
}
}

int main() {
    bool correcto = true;

    Enemigo enemigo(1, 100.0f, 100.0f, 56.0f, 56.0f,
                    150.0f, 100, 10, "Prueba", 10, 5);
    const FormaColision formaEnemigo = enemigo.obtenerFormaColision();
    correcto &= verificar(enemigo.obtenerDano() == 10,
                          "dano de contacto del enemigo");
    correcto &= verificar(
        formaEnemigo.x == 100.0f && formaEnemigo.y == 100.0f &&
        formaEnemigo.ancho == 56.0f && formaEnemigo.alto == 56.0f,
        "hitbox del enemigo");

    Proyectil proyectil({110.0f, 110.0f}, 0.0f, 0.0f,
                        28.0f, 14.0f, 100);
    if (SistemaColisiones::hayInterseccion(
            proyectil.obtenerFormaColision(),
            enemigo.obtenerFormaColision())) {
        enemigo.recibirDanio(proyectil.obtenerDano());
        proyectil.desactivar();
    }

    correcto &= verificar(!enemigo.estaVivo(), "dano aplicado al enemigo");
    correcto &= verificar(!proyectil.estaActivo(),
                          "proyectil desactivado tras impactar");

    enemigo = Enemigo(1, 100.0f, 100.0f, 56.0f, 56.0f,
                      150.0f, 100, 10, "Prueba", 10, 5);
    correcto &= verificar(enemigo.estaVivo(), "enemigo vivo tras reiniciar");
    const FormaColision formaReiniciada = enemigo.obtenerFormaColision();
    correcto &= verificar(
        formaReiniciada.x == 100.0f && formaReiniciada.y == 100.0f &&
        formaReiniciada.ancho == 56.0f && formaReiniciada.alto == 56.0f,
        "hitbox restaurado al reiniciar");

    Enemigo enemigoLejano(2, 300.0f, 300.0f, 56.0f, 56.0f,
                          150.0f, 100, 10, "Lejano", 10, 5);
    Proyectil proyectilLejano({10.0f, 10.0f}, 0.0f, 0.0f,
                              28.0f, 14.0f, 25);
    correcto &= verificar(
        !SistemaColisiones::hayInterseccion(
            proyectilLejano.obtenerFormaColision(),
            enemigoLejano.obtenerFormaColision()),
        "sin impacto fuera del hitbox");
    correcto &= verificar(enemigoLejano.estaVivo(),
                          "enemigo sin dano fuera del hitbox");
    correcto &= verificar(proyectilLejano.estaActivo(),
                          "proyectil activo sin impacto");

    if (correcto) {
        std::cout << "Enemigo y proyectil: pruebas correctas\n";
        return 0;
    }

    return 1;
}
