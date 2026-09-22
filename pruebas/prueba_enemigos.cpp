// Prueba aislada del modulo de enemigos.
//
// No forma parte del juego: tiene su propio main y su propia ventana, para
// poder demostrar el modulo sin depender del trabajo de los otros dos.
//
// Que se ve:
//   - un cuadrado rojo (el enemigo) persiguiendo al punto verde
//   - con ESPACIO se le hace dano; al llegar a cero deja de dibujarse
//   - con R vuelve a aparecer un enemigo nuevo
//   - con ESC se cierra
//
// Compilar con compilar_pruebas.bat

#include "Enemigo.h"

#include <SFML/Graphics.hpp>

namespace {

const unsigned int ANCHO = 800;
const unsigned int ALTO = 600;

const float LADO_ENEMIGO = 32.0f;
const int DANO_POR_GOLPE = 25;

// Crea un enemigo de prueba en la esquina superior izquierda.
// Orden de los parametros: id, x, y, ancho, alto, velocidad, vida, dano,
// nombre, experiencia otorgada, chatarra otorgada.
Enemigo crearEnemigoDePrueba() {
    return Enemigo(1, 60.0f, 60.0f, LADO_ENEMIGO, LADO_ENEMIGO, 120.0f, 100, 10,
                   "Robot basico", 5, 3);
}

}  // namespace

int main() {
    sf::RenderWindow ventana(sf::VideoMode({ANCHO, ALTO}),
                             "Prueba del modulo de enemigos");
    ventana.setFramerateLimit(60);

    Enemigo enemigo = crearEnemigoDePrueba();

    // El objetivo simula al jugador. Es solo una coordenada: el modulo de
    // enemigos no conoce la clase Jugador y no tiene por que conocerla.
    Posicion objetivo;
    objetivo.x = ANCHO / 2.0f;
    objetivo.y = ALTO / 2.0f;

    // Figuras para dibujar. El Enemigo no sabe dibujarse (no incluye SFML):
    // estas figuras son solo su representacion en pantalla.
    // El tamano sale de la forma de colision, asi lo que se ve en pantalla es
    // exactamente el rectangulo que va a usar el SistemaColisiones.
    const FormaColision formaEnemigo = enemigo.obtenerFormaColision();
    sf::RectangleShape figuraEnemigo({formaEnemigo.ancho, formaEnemigo.alto});
    figuraEnemigo.setFillColor(sf::Color(200, 60, 60));

    sf::CircleShape figuraObjetivo(6.0f);
    figuraObjetivo.setFillColor(sf::Color(90, 200, 120));
    figuraObjetivo.setPosition({objetivo.x - 6.0f, objetivo.y - 6.0f});

    // El reloj va ANTES del bucle. Si se declarara adentro se reiniciaria en
    // cada vuelta y el delta time daria siempre casi cero.
    sf::Clock reloj;

    while (ventana.isOpen()) {
        // 1. Cuantos segundos pasaron desde el frame anterior.
        const float dt = reloj.restart().asSeconds();

        // 2. Eventos del teclado y de la ventana.
        while (const auto evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                if (tecla->scancode == sf::Keyboard::Scancode::Escape) {
                    ventana.close();
                }

                // Dano simulado: reemplaza al impacto de un proyectil, que
                // todavia no esta integrado.
                if (tecla->scancode == sf::Keyboard::Scancode::Space) {
                    enemigo.recibirDanio(DANO_POR_GOLPE);
                }

                if (tecla->scancode == sf::Keyboard::Scancode::R) {
                    enemigo = crearEnemigoDePrueba();
                }
            }
        }

        // 3. Actualizar la simulacion y sincronizar la figura con la posicion
        // real del enemigo. Sin este setPosition el enemigo se mueve en
        // memoria pero en pantalla no pasa nada.
        if (enemigo.estaVivo()) {
            enemigo.actualizar(objetivo, dt);

            const Posicion posicion = enemigo.obtenerPosicion();
            figuraEnemigo.setPosition({posicion.x, posicion.y});
        }

        // 4. Dibujar.
        ventana.clear(sf::Color(28, 31, 36));
        ventana.draw(figuraObjetivo);
        if (enemigo.estaVivo()) {
            ventana.draw(figuraEnemigo);
        }
        ventana.display();
    }

    return 0;
}
