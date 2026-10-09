#include "Enemigo.h"

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
    Enemigo enemigo(
        1, 100.0f, 100.0f, 56.0f, 56.0f,
        150.0f, 100, 10, "Prueba", 10, 5);
    const Recompensa recompensa = enemigo.obtenerRecompensa();

    const Enemigo rapido = crearEnemigo(RAPIDO, 20.0f, 30.0f);
    const FormaColision formaRapido = rapido.obtenerFormaColision();
    const Recompensa recompensaRapido = rapido.obtenerRecompensa();

    const Enemigo resistente = crearEnemigo(RESISTENTE, 40.0f, 50.0f);
    const FormaColision formaResistente = resistente.obtenerFormaColision();
    const Recompensa recompensaResistente = resistente.obtenerRecompensa();

    const Enemigo basico = crearEnemigo(BASICO, 10.0f, 15.0f);
    const FormaColision formaBasico = basico.obtenerFormaColision();
    const Recompensa recompensaBasico = basico.obtenerRecompensa();

    const bool correcto =
        verificar(recompensa.experiencia == 10,
                  "experiencia otorgada por el enemigo") &&
        verificar(recompensa.chatarra == 5,
                  "chatarra otorgada por el enemigo") &&
        verificar(formaBasico.ancho == 56.0f && formaBasico.alto == 56.0f,
                  "dimensiones del enemigo basico") &&
        verificar(basico.obtenerDano() == 10,
                  "dano del enemigo basico") &&
        verificar(recompensaBasico.experiencia == 5 &&
                      recompensaBasico.chatarra == 3,
                  "recompensa del enemigo basico") &&
        verificar(formaRapido.ancho == 40.0f && formaRapido.alto == 40.0f,
                  "dimensiones del enemigo rapido") &&
        verificar(rapido.obtenerDano() == 8,
                  "dano del enemigo rapido") &&
        verificar(recompensaRapido.experiencia == 7 &&
                      recompensaRapido.chatarra == 2,
                  "recompensa del enemigo rapido") &&
        verificar(formaResistente.ancho == 72.0f &&
                      formaResistente.alto == 72.0f,
                  "dimensiones del enemigo resistente") &&
        verificar(resistente.obtenerDano() == 15,
                  "dano del enemigo resistente") &&
        verificar(recompensaResistente.experiencia == 12 &&
                      recompensaResistente.chatarra == 6,
                  "recompensa del enemigo resistente");

    if (correcto) {
        std::cout << "Recompensas: pruebas correctas\n";
        return 0;
    }

    return 1;
}
