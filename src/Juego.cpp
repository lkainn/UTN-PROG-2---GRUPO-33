#include "Juego.h"

#include "Configuracion.h"
#include "SistemaColisiones.h"

#include <algorithm>

//std::min() recibe dos valores y devuelve el menor.

namespace {
constexpr float ANCHO_ENEMIGO = 56.0f;
constexpr float ALTO_ENEMIGO = 56.0f;
constexpr float VELOCIDAD_ENEMIGO = 150.0f;
constexpr float VELOCIDAD_PROYECTIL = 800.0f;
constexpr int DANO_PROYECTIL = 25;
}

Juego::Juego()
    : ventana(
          sf::VideoMode({Configuracion::ANCHO_VENTANA, Configuracion::ALTO_VENTANA}),
          "WASD: mover | Espacio: disparar | K: dano | E: experiencia"),
      enemigoPrueba(1, Configuracion::ANCHO_VENTANA * 0.75f,
                    Configuracion::ALTO_VENTANA / 2.0f - ALTO_ENEMIGO / 2.0f,
                    ANCHO_ENEMIGO, ALTO_ENEMIGO, VELOCIDAD_ENEMIGO,
                    100, 10, "Enemigo de prueba", 10, 5),
      figuraEnemigoPrueba({ANCHO_ENEMIGO, ALTO_ENEMIGO}),
      figuraProyectilPrueba({28.0f, 14.0f}),
      proyectilPrueba({0.0f, 0.0f}, VELOCIDAD_PROYECTIL, 0.0f,
                      28.0f, 14.0f, DANO_PROYECTIL) {
    ventana.setFramerateLimit(Configuracion::LIMITE_FPS);

    figuraEnemigoPrueba.setFillColor(sf::Color(205, 70, 70));
    const Posicion posicionEnemigo = enemigoPrueba.obtenerPosicion();
    figuraEnemigoPrueba.setPosition({posicionEnemigo.x, posicionEnemigo.y});

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
            } else if (tecla->scancode == sf::Keyboard::Scancode::K) {
                jugador.recibirDano(20);
            } else if (tecla->scancode == sf::Keyboard::Scancode::E) {
                jugador.ganarExperiencia(40);
            }
        }
    }
}

void Juego::actualizar() {
    // Limitar el salto reduce el riesgo de atravesar al enemigo si la ventana se pausa.
    const float segundos = std::min(relojFotogramas.restart().asSeconds(), 0.05f);
    jugador.procesarEntrada();
    jugador.actualizar();

    if (enemigoPrueba.estaVivo()) {
        const Posicion posicionJugador = jugador.getPosicion();
        const FormaColision formaEnemigo = enemigoPrueba.obtenerFormaColision();
        const Posicion objetivoEnemigo{
            posicionJugador.x - formaEnemigo.ancho / 2.0f,
            posicionJugador.y - formaEnemigo.alto / 2.0f
        };
        enemigoPrueba.actualizar(objetivoEnemigo, segundos);

        const Posicion posicionEnemigo = enemigoPrueba.obtenerPosicion();
        figuraEnemigoPrueba.setPosition({posicionEnemigo.x, posicionEnemigo.y});
    }

    if (!proyectilPrueba.estaActivo()) {
        return;
    }

    proyectilPrueba.actualizar(segundos);
    const Posicion posicionProyectil = proyectilPrueba.obtenerPosicion();
    figuraProyectilPrueba.setPosition({posicionProyectil.x, posicionProyectil.y});

    if (enemigoPrueba.estaVivo() &&
        SistemaColisiones::hayInterseccion(
            proyectilPrueba.obtenerFormaColision(),
            enemigoPrueba.obtenerFormaColision())) {
        enemigoPrueba.recibirDanio(proyectilPrueba.obtenerDano());
        proyectilPrueba.desactivar();
        if (!enemigoPrueba.estaVivo()) {
            ventana.setTitle("Enemigo eliminado | Escape: salir");
        }
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
    if (enemigoPrueba.estaVivo()) {
        ventana.draw(figuraEnemigoPrueba);
    }
    jugador.dibujar(ventana);
    if (proyectilPrueba.estaActivo()) {
        ventana.draw(figuraProyectilPrueba);
    }
    ventana.display();
}

void Juego::dispararProyectil() {
    if (proyectilPrueba.estaActivo()) {
        return;
    }

    const FormaColision formaJugador = jugador.getFormaColision();
    const Posicion posicionInicial{
        formaJugador.x + formaJugador.ancho,
        formaJugador.y + formaJugador.alto / 2.0f
            - figuraProyectilPrueba.getSize().y / 2.0f
    };
    proyectilPrueba = Proyectil(posicionInicial, VELOCIDAD_PROYECTIL, 0.0f,
                                figuraProyectilPrueba.getSize().x,
                                figuraProyectilPrueba.getSize().y,
                                DANO_PROYECTIL);
    figuraProyectilPrueba.setPosition({posicionInicial.x, posicionInicial.y});
}

void Juego::reiniciarPrueba() {
    enemigoPrueba = Enemigo(
        1, Configuracion::ANCHO_VENTANA * 0.75f,
        Configuracion::ALTO_VENTANA / 2.0f - ALTO_ENEMIGO / 2.0f,
        ANCHO_ENEMIGO, ALTO_ENEMIGO, VELOCIDAD_ENEMIGO,
        100, 10, "Enemigo de prueba", 10, 5);
    const Posicion posicionEnemigo = enemigoPrueba.obtenerPosicion();
    figuraEnemigoPrueba.setPosition({posicionEnemigo.x, posicionEnemigo.y});
    proyectilPrueba.desactivar();
    ventana.setTitle("WASD: mover | Espacio: disparar | K: dano | E: experiencia");
    relojFotogramas.restart();
}
