// Prueba aislada del modulo de enemigos.
//
// No forma parte del juego: tiene su propio main y su propia ventana, para
// poder demostrar el modulo sin depender del trabajo de los otros dos.
//
// Que se ve:
//   - los enemigos aparecen solos en un anillo alrededor del objetivo, desde
//     cualquier direccion, cada vez mas seguido a medida que pasa el tiempo
//   - persiguen al objetivo (el circulo verde) y se detienen al llegar
//   - el color indica la variante:
//       rojo    = basico     (56x56, vida 100, 120 px/s) desde el segundo 0
//       amarillo= rapido     (40x40, vida  60, 220 px/s) desde el segundo 60
//       violeta = resistente (72x72, vida 250,  70 px/s) desde el segundo 180
//   - el circulo gris marca el anillo por donde aparecen
//   - el titulo muestra oleada, tiempo, vivos sobre el tope e intervalo
//
// Controles:
//   ESPACIO  dano a todos los vivos (simula el impacto de un proyectil)
//   flechas  mover el objetivo, para ver como lo persiguen
//   R        reiniciar
//   ESC      cerrar
//
// Compilar con compilar_pruebas.bat

#include "Enemigo.h"
#include "GestorOleadas.h"

#include <cstddef>
#include <string>

#include <SFML/Graphics.hpp>

namespace {

const unsigned int ANCHO = 1280;
const unsigned int ALTO = 720;

const int DANO_POR_GOLPE = 25;
const float VELOCIDAD_OBJETIVO = 300.0f;
const float RADIO_OBJETIVO = 10.0f;

// El Enemigo no sabe de colores ni de SFML, y esta bien que sea asi: el color
// lo elige quien dibuja, a partir del id de la variante.
sf::Color colorDeVariante(int idEnemigo) {
    switch (idEnemigo) {
        case 2:
            return sf::Color(235, 190, 60);   // rapido
        case 3:
            return sf::Color(130, 115, 220);  // resistente
        case 1:
        default:
            return sf::Color(200, 60, 60);    // basico
    }
}

}  // namespace

int main() {
    sf::RenderWindow ventana(sf::VideoMode({ANCHO, ALTO}),
                             "Prueba del modulo de enemigos");
    ventana.setFramerateLimit(60);

    GestorOleadas gestor;

    // El objetivo simula al jugador. Es solo una coordenada: el modulo de
    // enemigos no conoce la clase Jugador y no tiene por que conocerla.
    Posicion objetivo;
    objetivo.x = ANCHO / 2.0f;
    objetivo.y = ALTO / 2.0f;

    sf::CircleShape figuraObjetivo(RADIO_OBJETIVO);
    figuraObjetivo.setFillColor(sf::Color(90, 200, 120));

    // Referencia visual del anillo de aparicion. Queda fuera de la ventana:
    // sirve para entender de donde vienen los enemigos.
    sf::CircleShape anillo(GestorOleadas::DISTANCIA_APARICION);
    anillo.setFillColor(sf::Color::Transparent);
    anillo.setOutlineColor(sf::Color(70, 75, 85));
    anillo.setOutlineThickness(2.0f);

    // Una sola figura reutilizada para dibujar a todos: antes de cada draw le
    // ponemos el tamano, el color y la posicion del enemigo que toca.
    sf::RectangleShape figuraEnemigo;

    // El reloj va ANTES del bucle. Si se declarara adentro se reiniciaria en
    // cada vuelta y el delta time daria siempre casi cero.
    sf::Clock reloj;

    float tiempoDesdeTitulo = 0.0f;  // para no reescribir el titulo siempre

    while (ventana.isOpen()) {
        // 1. Cuantos segundos pasaron desde el frame anterior. Se limita el
        // salto para que al arrastrar o minimizar la ventana nada se
        // teletransporte.
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) {
            dt = 0.05f;
        }

        // 2. Eventos del teclado y de la ventana.
        while (const auto evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                if (tecla->scancode == sf::Keyboard::Scancode::Escape) {
                    ventana.close();
                }

                // Dano simulado. Reemplaza al impacto de un proyectil, que en
                // esta prueba no existe.
                if (tecla->scancode == sf::Keyboard::Scancode::Space) {
                    gestor.danarATodos(DANO_POR_GOLPE);
                }

                if (tecla->scancode == sf::Keyboard::Scancode::R) {
                    gestor.reiniciar();
                }
            }
        }

        // 3. Mover el objetivo con las flechas, para ver como lo persiguen.
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
            objetivo.x -= VELOCIDAD_OBJETIVO * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
            objetivo.x += VELOCIDAD_OBJETIVO * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
            objetivo.y -= VELOCIDAD_OBJETIVO * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
            objetivo.y += VELOCIDAD_OBJETIVO * dt;
        }

        // 4. Toda la logica del modulo pasa por esta unica llamada: genera,
        // mueve y limpia.
        gestor.actualizar(objetivo, dt);

        // 5. Dibujar.
        ventana.clear(sf::Color(28, 31, 36));

        anillo.setPosition({objetivo.x - GestorOleadas::DISTANCIA_APARICION,
                            objetivo.y - GestorOleadas::DISTANCIA_APARICION});
        ventana.draw(anillo);

        figuraObjetivo.setPosition({objetivo.x - RADIO_OBJETIVO,
                                    objetivo.y - RADIO_OBJETIVO});
        ventana.draw(figuraObjetivo);

        for (const Enemigo& enemigo : gestor.obtenerEnemigos()) {
            // El tamano y la posicion salen de la forma de colision, asi lo
            // que se ve es exactamente el rectangulo que usaria el
            // SistemaColisiones.
            const FormaColision forma = enemigo.obtenerFormaColision();
            figuraEnemigo.setSize({forma.ancho, forma.alto});
            figuraEnemigo.setPosition({forma.x, forma.y});
            figuraEnemigo.setFillColor(
                colorDeVariante(enemigo.obtenerIdEnemigo()));

            ventana.draw(figuraEnemigo);
        }

        ventana.display();

        // 6. Titulo con el estado de la partida. Se reescribe como maximo
        // cuatro veces por segundo: hacerlo en cada fotograma es trabajo al
        // vacio y en Windows hace titilar la barra de titulo.
        tiempoDesdeTitulo += dt;
        if (tiempoDesdeTitulo >= 0.25f) {
            tiempoDesdeTitulo = 0.0f;

            const int segundos = (int)gestor.tiempoDePartida();
            ventana.setTitle(
                "Enemigos - oleada " + std::to_string(gestor.oleadaActual()) +
                " - " + std::to_string(segundos / 60) + ":" +
                (segundos % 60 < 10 ? "0" : "") +
                std::to_string(segundos % 60) +
                " - vivos " + std::to_string(gestor.cantidadEnemigos()) + "/" +
                std::to_string(gestor.maximoVivosActual()) +
                " - cada " + std::to_string(gestor.intervaloActual()).substr(0, 4) +
                "s");
        }
    }

    return 0;
}
