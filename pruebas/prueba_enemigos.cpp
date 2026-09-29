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

// El Enemigo no sabe de colores ni de SFML, y esta bien que sea asi. Para la
// prueba necesitamos recordar con que color dibujar cada uno, asi que los
// juntamos aca: al borrar un enemigo del vector se va con su color.
struct EnemigoVisible {
    Enemigo enemigo;
    sf::Color color;
};

// Carga las tres variantes en posiciones distintas, para verlas convergir al
// mismo objetivo a velocidades diferentes.
void poblarEnemigos(std::vector<EnemigoVisible>& enemigos) {
    enemigos.clear();

    enemigos.push_back({crearEnemigo(BASICO, 60.0f, 60.0f),
                        sf::Color(200, 60, 60)});
    enemigos.push_back({crearEnemigo(RAPIDO, 700.0f, 70.0f),
                        sf::Color(235, 190, 60)});
    enemigos.push_back({crearEnemigo(RESISTENTE, 80.0f, 470.0f),
                        sf::Color(130, 115, 220)});
}

}  // namespace

int main() {
    sf::RenderWindow ventana(sf::VideoMode({ANCHO, ALTO}),
                             "Prueba del modulo de enemigos");
    ventana.setFramerateLimit(60);

    std::vector<EnemigoVisible> enemigos;
    poblarEnemigos(enemigos);

    // El objetivo simula al jugador. Es solo una coordenada: el modulo de
    // enemigos no conoce la clase Jugador y no tiene por que conocerla.
    Posicion objetivo;
    objetivo.x = ANCHO / 2.0f;
    objetivo.y = ALTO / 2.0f;

    sf::CircleShape figuraObjetivo(6.0f);
    figuraObjetivo.setFillColor(sf::Color(90, 200, 120));
    figuraObjetivo.setPosition({objetivo.x - 6.0f, objetivo.y - 6.0f});

    // Una sola figura reutilizada para dibujar a todos: antes de cada draw le
    // ponemos el tamano, el color y la posicion del enemigo que toca.
    sf::RectangleShape figuraEnemigo;

    // El reloj va ANTES del bucle. Si se declarara adentro se reiniciaria en
    // cada vuelta y el delta time daria siempre casi cero.
    sf::Clock reloj;

    int vivosMostrados = -1;  // para no reescribir el titulo en cada frame

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

                // Dano simulado a todos los vivos. Reemplaza al impacto de un
                // proyectil, que en esta prueba no existe.
                if (tecla->scancode == sf::Keyboard::Scancode::Space) {
                    for (EnemigoVisible& ev : enemigos) {
                        if (ev.enemigo.estaVivo()) {
                            ev.enemigo.recibirDanio(DANO_POR_GOLPE);
                        }
                    }
                }

                if (tecla->scancode == sf::Keyboard::Scancode::R) {
                    poblarEnemigos(enemigos);
                }
            }
        }

        // 3. Actualizar a todos. El & del for es obligatorio: sin el, C++ da
        // una copia de cada enemigo, se mueve la copia y el original queda
        // quieto. No avisa nada, simplemente no pasa nada en pantalla.
        for (EnemigoVisible& ev : enemigos) {
            if (ev.enemigo.estaVivo()) {
                ev.enemigo.actualizar(objetivo, dt);
            }
        }

        // Sacar del vector a los muertos. Va en una pasada APARTE: borrar
        // adentro del for de arriba corrompe el recorrido (y el programa puede
        // crashear, saltearse enemigos, o andar de casualidad).
        //
        // remove_if no borra: empuja los muertos al final y devuelve donde
        // empieza esa zona. El erase corta desde ahi hasta el final.
        enemigos.erase(
            std::remove_if(enemigos.begin(), enemigos.end(),
                           [](const EnemigoVisible& ev) {
                               return !ev.enemigo.estaVivo();
                           }),
            enemigos.end());

        // 4. Dibujar.
        ventana.clear(sf::Color(28, 31, 36));
        ventana.draw(figuraObjetivo);

        // Todos los que quedan en el vector estan vivos: los muertos ya se
        // sacaron mas arriba.
        for (const EnemigoVisible& ev : enemigos) {
            // El tamano y la posicion salen de la forma de colision, asi lo
            // que se ve es exactamente el rectangulo que usaria el
            // SistemaColisiones.
            const FormaColision forma = ev.enemigo.obtenerFormaColision();
            figuraEnemigo.setSize({forma.ancho, forma.alto});
            figuraEnemigo.setPosition({forma.x, forma.y});
            figuraEnemigo.setFillColor(ev.color);

            ventana.draw(figuraEnemigo);
        }

        ventana.display();

        // 5. Titulo con la cantidad de enemigos, solo cuando cambia.
        const int vivos = (int)enemigos.size();
        if (vivos != vivosMostrados) {
            vivosMostrados = vivos;
            ventana.setTitle("Prueba del modulo de enemigos - enemigos: " +
                             std::to_string(vivos));
        }
    }

    return 0;
}
