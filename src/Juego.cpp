#include "Juego.h"

#include "Configuracion.h"

Juego::Juego()
    : ventana(
          sf::VideoMode({Configuracion::ANCHO_VENTANA, Configuracion::ALTO_VENTANA}),
          "Scrap Protocol"),
      robotPrueba({64.0f, 64.0f}) {
    ventana.setFramerateLimit(Configuracion::LIMITE_FPS);

    robotPrueba.setFillColor(sf::Color(221, 146, 46));
    robotPrueba.setOrigin(robotPrueba.getGeometricCenter());
    robotPrueba.setPosition({
        Configuracion::ANCHO_VENTANA / 2.0f,
        Configuracion::ALTO_VENTANA / 2.0f
    });
}

void Juego::ejecutar() {
    while (ventana.isOpen()) {
        procesarEventos();
        actualizar();
        dibujar();
    }
}

void Juego::procesarEventos() {
    while (const auto evento = ventana.pollEvent()) {
        if (evento->is<sf::Event::Closed>()) {
            ventana.close();
        }

        if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
            if (tecla->scancode == sf::Keyboard::Scancode::Escape) {
                ventana.close();
            }
        }
    }
}

void Juego::actualizar() {
    // Los sistemas del juego se actualizaran aqui en los proximos incrementos.
}

void Juego::dibujar() {
    ventana.clear(sf::Color(28, 31, 36));
    ventana.draw(robotPrueba);
    ventana.display();
}

