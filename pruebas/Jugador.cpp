#include "Jugador.h"

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

    Jugador jugador;
    correcto &= verificar(jugador.getVida() == 100, "vida inicial");
    correcto &= verificar(jugador.getNivel() == 1, "nivel inicial");
    correcto &= verificar(jugador.estaVivo(), "estado inicial vivo");

    jugador.recibirDano(20);
    correcto &= verificar(jugador.getVida() == 80, "dano recibido");

    jugador.ganarExperiencia(350);
    correcto &= verificar(jugador.getNivel() == 3, "subida de varios niveles");
    correcto &= verificar(jugador.getExperiencia() == 100, "experiencia restante");
    correcto &= verificar(jugador.getVidaMaxima() == 140, "vida maxima por nivel");

    jugador.ganarChatarra(25);
    correcto &= verificar(jugador.getChatarra() == 25, "chatarra obtenida");

    jugador.cargarEstado(80, 5, 50, 200);
    correcto &= verificar(jugador.getVida() == 80, "vida cargada");
    correcto &= verificar(jugador.getNivel() == 5, "nivel cargado");
    correcto &= verificar(jugador.getExperiencia() == 50, "experiencia cargada");
    correcto &= verificar(jugador.getChatarra() == 200, "chatarra cargada");

    jugador.setPosicion({-50.0f, -50.0f});
    jugador.actualizar();
    const FormaColision forma = jugador.getFormaColision();
    correcto &= verificar(forma.x == 0.0f && forma.y == 0.0f,
                          "limite superior izquierdo");

    jugador.recibirDano(200);
    correcto &= verificar(!jugador.estaVivo(), "muerte por dano");
    const int experienciaAntes = jugador.getExperiencia();
    const int chatarraAntes = jugador.getChatarra();
    jugador.ganarExperiencia(100);
    jugador.ganarChatarra(100);
    correcto &= verificar(jugador.getExperiencia() == experienciaAntes,
                          "sin experiencia al morir");
    correcto &= verificar(jugador.getChatarra() == chatarraAntes,
                          "sin chatarra al morir");

    jugador.reiniciar();
    correcto &= verificar(jugador.estaVivo(), "jugador vivo tras reiniciar");
    correcto &= verificar(jugador.getVida() == jugador.getVidaMaxima(),
                          "vida restaurada al reiniciar");

    if (correcto) {
        std::cout << "Jugador: pruebas correctas\n";
        return 0;
    }

    return 1;
}
