#include "Juego.h"

#include "Configuracion.h"
#include "SistemaColisiones.h"

#include <algorithm>

//std::min() recibe dos valores y devuelve el menor.

namespace {
constexpr float VELOCIDAD_PROYECTIL = 800.0f;

FormaColision obtenerFormaColision(const sf::RectangleShape& figura) {
    const sf::FloatRect limites = figura.getGlobalBounds();
    return {limites.position.x, limites.position.y,
            limites.size.x, limites.size.y};
}
}

Juego::Juego()
    : ventana(
          sf::VideoMode({Configuracion::ANCHO_VENTANA, Configuracion::ALTO_VENTANA}),
          "Espacio: disparar | R: reiniciar | Escape: salir"),
      robotPrueba({64.0f, 64.0f}),
      blancoPrueba({100.0f, 100.0f}),
      figuraProyectilPrueba({28.0f, 14.0f}),
      proyectilPrueba({0.0f, 0.0f}, VELOCIDAD_PROYECTIL, 0.0f,
                      28.0f, 14.0f, 1) {
    ventana.setFramerateLimit(Configuracion::LIMITE_FPS);

    robotPrueba.setFillColor(sf::Color(221, 146, 46));
    robotPrueba.setOrigin(robotPrueba.getGeometricCenter());
    robotPrueba.setPosition({
        Configuracion::ANCHO_VENTANA * 0.2f,
        Configuracion::ALTO_VENTANA / 2.0f
    });

    blancoPrueba.setPosition({
        Configuracion::ANCHO_VENTANA * 0.7f,
        Configuracion::ALTO_VENTANA / 2.0f - blancoPrueba.getSize().y / 2.0f
    });
    figuraProyectilPrueba.setFillColor(sf::Color(255, 225, 70));
    reiniciarPrueba();
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
            } else if (tecla->scancode == sf::Keyboard::Scancode::Space) {
                dispararProyectil();
            } else if (tecla->scancode == sf::Keyboard::Scancode::R) {
                reiniciarPrueba();
            }
        }
    }
}

void Juego::actualizar() {
    // limitar el salto evita atravesar el blanco si la ventana se pausa.
    const float segundos = std::min(relojFotogramas.restart().asSeconds(), 0.05f);
    if (!proyectilPrueba.estaActivo()) {
        return;
    }

    proyectilPrueba.actualizar(segundos);
    const Posicion posicionProyectil = proyectilPrueba.obtenerPosicion();
    figuraProyectilPrueba.setPosition({posicionProyectil.x, posicionProyectil.y});

    if (SistemaColisiones::hayInterseccion(
            proyectilPrueba.obtenerFormaColision(),
            obtenerFormaColision(blancoPrueba))) {
        proyectilPrueba.desactivar();
        blancoImpactado = true;
        blancoPrueba.setFillColor(sf::Color(75, 205, 105));
        ventana.setTitle("Impacto detectado | R: reiniciar | Escape: salir");
    } else if (!SistemaColisiones::estaDentroDe(
                   proyectilPrueba.obtenerFormaColision(),
                   {0.0f, 0.0f,
                    static_cast<float>(Configuracion::ANCHO_VENTANA),
                    static_cast<float>(Configuracion::ALTO_VENTANA)})) {
        proyectilPrueba.desactivar();
    }
}

void Juego::dibujar() {
    ventana.clear(sf::Color(28, 31, 36));
    ventana.draw(robotPrueba);
    ventana.draw(blancoPrueba);
    if (proyectilPrueba.estaActivo()) {
        ventana.draw(figuraProyectilPrueba);
    }
    ventana.display();
}

void Juego::dispararProyectil() {
    if (proyectilPrueba.estaActivo() || blancoImpactado) {
        return;
    }

    const Posicion posicionInicial{
        robotPrueba.getPosition().x + robotPrueba.getSize().x / 2.0f,
        robotPrueba.getPosition().y - figuraProyectilPrueba.getSize().y / 2.0f
    };
    proyectilPrueba = Proyectil(posicionInicial, VELOCIDAD_PROYECTIL, 0.0f,
                                figuraProyectilPrueba.getSize().x,
                                figuraProyectilPrueba.getSize().y, 1);
    figuraProyectilPrueba.setPosition({posicionInicial.x, posicionInicial.y});
}

void Juego::reiniciarPrueba() {
    proyectilPrueba.desactivar();
    blancoImpactado = false;
    blancoPrueba.setFillColor(sf::Color(205, 70, 70));
    ventana.setTitle("Espacio: disparar | R: reiniciar | Escape: salir");
    relojFotogramas.restart();
}
