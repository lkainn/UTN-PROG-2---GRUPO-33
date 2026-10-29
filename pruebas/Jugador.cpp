#include <SFML/Graphics.hpp>
#include "EntornoDatosPrueba.h"
#define private public
#include "Jugador.h"
#undef private

#include <iostream>
#include <cmath>
#include <limits>

namespace {
bool verificar(bool condicion, const char* mensaje) {
    if (!condicion) {
        std::cerr << "Fallo: " << mensaje << '\n';
    }
    return condicion;
}
}

int main() {
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    bool correcto = true;

    Jugador jugador;
    correcto &= verificar(jugador.getVida() == 100, "vida inicial");
    correcto &= verificar(jugador.getNivel() == 1, "nivel inicial");
    correcto &= verificar(jugador.estaVivo(), "estado inicial vivo");
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            jugador.setPosicion({500.0f, 500.0f});
            jugador.moverEnDireccion(static_cast<float>(x), static_cast<float>(y));
            const Posicion posicion = jugador.getPosicion();
            const float distancia = std::hypot(posicion.x - 500.0f, posicion.y - 500.0f);
            const float esperada = x == 0 && y == 0 ? 0.0f : jugador.getVelocidad();
            correcto &= verificar(std::abs(distancia - esperada) < 0.0001f,
                                  "cardinales y diagonales tienen igual velocidad; opuestos se cancelan");
        }
    }
    jugador.setPosicion({500, 500});
    jugador.moverEnDireccion(std::numeric_limits<float>::infinity(), 1);
    correcto &= verificar(jugador.getPosicion().x == 500 && jugador.getPosicion().y == 500,
                          "direccion no finita no altera posicion");

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
