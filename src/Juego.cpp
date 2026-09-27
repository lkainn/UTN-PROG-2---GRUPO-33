#include "Juego.h"

#include "Configuracion.h"
#include "SistemaColisiones.h"

#include <algorithm>
#include <array>
#include <limits>

//std::min() recibe dos valores y devuelve el menor.

namespace {
constexpr float ANCHO_ENEMIGO = 56.0f;
constexpr float ALTO_ENEMIGO = 56.0f;
constexpr float VELOCIDAD_ENEMIGO = 150.0f;
constexpr float VELOCIDAD_PROYECTIL = 800.0f;
constexpr float ALCANCE_ARMA = 2210.0f;
constexpr float SEGUNDOS_REAPARICION = 3.0f;
constexpr std::array<Posicion, 3> POSICIONES_INICIALES_ENEMIGOS{{
    {1440.0f, 200.0f},
    {1440.0f, 512.0f},
    {1440.0f, 824.0f}
}};

Enemigo crearEnemigo(std::size_t indice) {
    const Posicion posicion = POSICIONES_INICIALES_ENEMIGOS[indice];
    return Enemigo(
        static_cast<int>(indice) + 1, posicion.x, posicion.y,
        ANCHO_ENEMIGO, ALTO_ENEMIGO, VELOCIDAD_ENEMIGO,
        100, 10, "Enemigo de prueba", 10, 5);
}
}

Juego::Juego()
    : ventana(
          sf::VideoMode({Configuracion::ANCHO_VENTANA, Configuracion::ALTO_VENTANA}),
          "WASD: mover | Disparo automatico | K: dano | E: experiencia"),
      enemigosPrueba{
          crearEnemigo(0),
          crearEnemigo(1),
          crearEnemigo(2)
      },
      figurasEnemigosPrueba{
          sf::RectangleShape({ANCHO_ENEMIGO, ALTO_ENEMIGO}),
          sf::RectangleShape({ANCHO_ENEMIGO, ALTO_ENEMIGO}),
          sf::RectangleShape({ANCHO_ENEMIGO, ALTO_ENEMIGO})
      },
      tiemposReaparicion{},
      figuraProyectilPrueba({28.0f, 14.0f}),
      proyectilPrueba({0.0f, 0.0f}, VELOCIDAD_PROYECTIL, 0.0f,
                      28.0f, 14.0f, 0),
      armaPrueba(1, "Pistola basica", 25, 2.0f, ALCANCE_ARMA, 5) {
    ventana.setFramerateLimit(Configuracion::LIMITE_FPS);

    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        figurasEnemigosPrueba[i].setFillColor(sf::Color(205, 70, 70));
    }

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
    armaPrueba.actualizar(segundos);
    jugador.procesarEntrada();
    jugador.actualizar();
    actualizarEnemigos(segundos);
    actualizarProyectil(segundos);
    actualizarDisparoAutomatico();
}

void Juego::actualizarEnemigos(float segundos) {
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (!enemigosPrueba[i].estaVivo()) {
            tiemposReaparicion[i] =
                std::max(0.0f, tiemposReaparicion[i] - segundos);
            if (tiemposReaparicion[i] <= 0.0f) {
                enemigosPrueba[i] = crearEnemigo(i);
                const Posicion posicion = enemigosPrueba[i].obtenerPosicion();
                figurasEnemigosPrueba[i].setPosition(
                    {posicion.x, posicion.y});
            }
            continue;
        }

        const Posicion posicionJugador = jugador.getPosicion();
        const FormaColision formaEnemigo =
            enemigosPrueba[i].obtenerFormaColision();
        const Posicion objetivoEnemigo{
            posicionJugador.x - formaEnemigo.ancho / 2.0f,
            posicionJugador.y - formaEnemigo.alto / 2.0f
        };
        enemigosPrueba[i].actualizar(objetivoEnemigo, segundos);

        const Posicion posicionEnemigo = enemigosPrueba[i].obtenerPosicion();
        figurasEnemigosPrueba[i].setPosition(
            {posicionEnemigo.x, posicionEnemigo.y});
    }
}

void Juego::actualizarProyectil(float segundos) {
    if (!proyectilPrueba.estaActivo()) {
        return;
    }

    proyectilPrueba.actualizar(segundos);
    const Posicion posicionProyectil = proyectilPrueba.obtenerPosicion();
    figuraProyectilPrueba.setPosition(
        {posicionProyectil.x, posicionProyectil.y});

    bool impacto = false;
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (enemigosPrueba[i].estaVivo() &&
            SistemaColisiones::hayInterseccion(
                proyectilPrueba.obtenerFormaColision(),
                enemigosPrueba[i].obtenerFormaColision())) {
            enemigosPrueba[i].recibirDanio(proyectilPrueba.obtenerDano());
            proyectilPrueba.desactivar();
            impacto = true;
            if (!enemigosPrueba[i].estaVivo()) {
                tiemposReaparicion[i] = SEGUNDOS_REAPARICION;
            }
            break;
        }
    }

    if (!impacto &&
        !SistemaColisiones::estaDentroDe(
            proyectilPrueba.obtenerFormaColision(),
            {0.0f, 0.0f,
             static_cast<float>(Configuracion::ANCHO_VENTANA),
             static_cast<float>(Configuracion::ALTO_VENTANA)})) {
        proyectilPrueba.desactivar();
    }
}

void Juego::dibujar() {
    ventana.clear(sf::Color(28, 31, 36));
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (enemigosPrueba[i].estaVivo()) {
            ventana.draw(figurasEnemigosPrueba[i]);
        }
    }
    jugador.dibujar(ventana);
    if (proyectilPrueba.estaActivo()) {
        ventana.draw(figuraProyectilPrueba);
    }
    ventana.display();
}

void Juego::actualizarDisparoAutomatico() {
    if (proyectilPrueba.estaActivo() || !armaPrueba.puedeDisparar()) {
        return;
    }

    const FormaColision formaJugador = jugador.getFormaColision();
    const Posicion centroJugador{
        formaJugador.x + formaJugador.ancho / 2.0f,
        formaJugador.y + formaJugador.alto / 2.0f
    };

    int indiceObjetivo = -1;
    float distanciaMinima = std::numeric_limits<float>::max();
    Posicion centroObjetivo{0.0f, 0.0f};
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        if (!enemigosPrueba[i].estaVivo()) {
            continue;
        }

        const FormaColision formaEnemigo =
            enemigosPrueba[i].obtenerFormaColision();
        const Posicion centroEnemigo{
            formaEnemigo.x + formaEnemigo.ancho / 2.0f,
            formaEnemigo.y + formaEnemigo.alto / 2.0f
        };
        const float diferenciaX = centroEnemigo.x - centroJugador.x;
        const float diferenciaY = centroEnemigo.y - centroJugador.y;
        const float distanciaCuadrada =
            diferenciaX * diferenciaX + diferenciaY * diferenciaY;
        if (distanciaCuadrada < distanciaMinima) {
            distanciaMinima = distanciaCuadrada;
            indiceObjetivo = static_cast<int>(i);
            centroObjetivo = centroEnemigo;
        }
    }

    if (indiceObjetivo < 0) {
        return;
    }

    if (armaPrueba.intentarDisparar(
            proyectilPrueba, centroJugador, centroObjetivo, VELOCIDAD_PROYECTIL,
            figuraProyectilPrueba.getSize().x,
            figuraProyectilPrueba.getSize().y)) {
        const Posicion posicionProyectil = proyectilPrueba.obtenerPosicion();
        figuraProyectilPrueba.setPosition(
            {posicionProyectil.x, posicionProyectil.y});
    }
}

void Juego::reiniciarPrueba() {
    for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
        enemigosPrueba[i] = crearEnemigo(i);
        const Posicion posicionEnemigo = enemigosPrueba[i].obtenerPosicion();
        figurasEnemigosPrueba[i].setPosition(
            {posicionEnemigo.x, posicionEnemigo.y});
        tiemposReaparicion[i] = 0.0f;
    }
    proyectilPrueba.desactivar();
    armaPrueba.reiniciar();
    ventana.setTitle(
        "WASD: mover | Disparo automatico | K: dano | E: experiencia");
    relojFotogramas.restart();
}
