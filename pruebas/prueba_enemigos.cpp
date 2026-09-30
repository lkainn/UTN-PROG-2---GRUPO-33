// Prueba aislada del modulo de enemigos.
//
// No forma parte del juego: tiene su propio main y su propia ventana, para
// poder demostrar el modulo sin depender del trabajo de los otros dos.
//
// Que se ve:
//   - tres variantes persiguiendo al punto verde, cada una de otro color:
//       rojo    = basico     (56x56, vida 100, 120 px/s)
//       amarillo= rapido     (40x40, vida  60, 220 px/s)
//       violeta = resistente (72x72, vida 250,  70 px/s)
//   - con ESPACIO se les hace dano a todos los vivos
//   - al morir salen del vector, no quedan acumulados
//   - con R vuelven a aparecer los tres
//   - con ESC se cierra
//   - el titulo de la ventana muestra cuantos quedan
//
// Compilar con compilar_pruebas.bat

#include "Enemigo.h"

#include <algorithm>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

namespace {

const unsigned int ANCHO = 800;
const unsigned int ALTO = 600;

const int DANO_POR_GOLPE = 25;

struct EnemigoVisible {
    Enemigo enemigo;
    sf::Color color;
};

void poblarEnemigos(std::vector<EnemigoVisible>& enemigos) {
    enemigos.clear();

    enemigos.push_back({crearEnemigo(BASICO, 60.0f, 60.0f),
                        sf::Color(200, 60, 60)});
    enemigos.push_back({crearEnemigo(RAPIDO, 700.0f, 70.0f),
                        sf::Color(235, 190, 60)});
    enemigos.push_back({crearEnemigo(RESISTENTE, 80.0f, 470.0f),
                        sf::Color(130, 115, 220)});
}

}

int main() {
    sf::RenderWindow ventana(sf::VideoMode({ANCHO, ALTO}),
                             "Prueba del modulo de enemigos");
    ventana.setFramerateLimit(60);

    std::vector<EnemigoVisible> enemigos;
    poblarEnemigos(enemigos);

    Posicion objetivo;
    objetivo.x = ANCHO / 2.0f;
    objetivo.y = ALTO / 2.0f;

    sf::CircleShape figuraObjetivo(6.0f);
    figuraObjetivo.setFillColor(sf::Color(90, 200, 120));
    figuraObjetivo.setPosition({objetivo.x - 6.0f, objetivo.y - 6.0f});

    sf::RectangleShape figuraEnemigo;
    sf::Clock reloj;
    int vivosMostrados = -1;

    while (ventana.isOpen()) {
        const float dt = reloj.restart().asSeconds();

        while (const auto evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                if (tecla->scancode == sf::Keyboard::Scancode::Escape) {
                    ventana.close();
                }

                if (tecla->scancode == sf::Keyboard::Scancode::Space) {
                    for (EnemigoVisible& enemigoVisible : enemigos) {
                        if (enemigoVisible.enemigo.estaVivo()) {
                            enemigoVisible.enemigo.recibirDanio(DANO_POR_GOLPE);
                        }
                    }
                }

                if (tecla->scancode == sf::Keyboard::Scancode::R) {
                    poblarEnemigos(enemigos);
                }
            }
        }

        for (EnemigoVisible& enemigoVisible : enemigos) {
            if (enemigoVisible.enemigo.estaVivo()) {
                enemigoVisible.enemigo.actualizar(objetivo, dt);
            }
        }

        enemigos.erase(
            std::remove_if(
                enemigos.begin(), enemigos.end(),
                [](const EnemigoVisible& enemigoVisible) {
                    return !enemigoVisible.enemigo.estaVivo();
                }),
            enemigos.end());

        ventana.clear(sf::Color(28, 31, 36));
        ventana.draw(figuraObjetivo);

        for (const EnemigoVisible& enemigoVisible : enemigos) {
            const FormaColision forma =
                enemigoVisible.enemigo.obtenerFormaColision();
            figuraEnemigo.setSize({forma.ancho, forma.alto});
            figuraEnemigo.setPosition({forma.x, forma.y});
            figuraEnemigo.setFillColor(enemigoVisible.color);
            ventana.draw(figuraEnemigo);
        }

        ventana.display();

        const int vivos = static_cast<int>(enemigos.size());
        if (vivos != vivosMostrados) {
            vivosMostrados = vivos;
            ventana.setTitle("Prueba del modulo de enemigos - enemigos: " +
                             std::to_string(vivos));
        }
    }

    return 0;
}
