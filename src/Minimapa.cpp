#include "Minimapa.h"

#include <cmath>

const float Minimapa::LADO = 190.0f;
const float Minimapa::ALCANCE = 1800.0f;
const float Minimapa::MARGEN = 18.0f;

namespace {

// Las marcas se distinguen por color Y por forma. Solo por color no alcanza:
// los enemigos ya ocupan el rojo, el amarillo, el violeta y el naranja, asi
// que un cofre amarillo se confundia con un enemigo rapido.
//
//   enemigos    circulo, con el color con que se dibujan en el mundo
//   cofre       cuadrado turquesa
//   estacion    rombo azul
//   jugador     circulo blanco al centro
//
// Los dos tonos frios no aparecen entre los enemigos, y la forma mantiene la
// diferencia aunque el color no se distinga bien.
const sf::Color FONDO(8, 10, 14, 205);
const sf::Color BORDE(110, 120, 135);
const sf::Color RETICULA(60, 68, 80, 140);
const sf::Color CONTORNO(10, 12, 16, 220);

// El jugador va verde. El cofre se corre al cian para no competir con ese
// verde, y como el cian y el azul de la estacion quedan cerca, lo que los
// separa es la forma: cuadrado contra rombo.
const sf::Color JUGADOR(80, 235, 110);
const sf::Color COFRE(60, 215, 235);
const sf::Color ESTACION(70, 140, 235);

const sf::Color BASICO(200, 60, 60);
const sf::Color RAPIDO(235, 190, 60);
const sf::Color RESISTENTE(130, 115, 220);
const sf::Color JEFE(240, 120, 20);

sf::Color colorDeVariante(int idEnemigo) {
    switch (idEnemigo) {
        case 2:  return RAPIDO;
        case 3:  return RESISTENTE;
        case 4:  return JEFE;
        default: return BASICO;
    }
}

}  // namespace

Minimapa::Minimapa() {
}

float Minimapa::distanciaCuadrada(Posicion a, Posicion b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// El jugador queda en el centro del recuadro, y el resto se ubica alrededor
// en la misma proporcion: una distancia de ALCANCE en el mundo equivale a
// medio lado del recuadro.
sf::Vector2f Minimapa::aPantalla(Posicion punto, Posicion jugador,
                                 sf::Vector2f esquina) const {
    const float escala = (LADO / 2.0f) / ALCANCE;
    return sf::Vector2f(
        esquina.x + LADO / 2.0f + (punto.x - jugador.x) * escala,
        esquina.y + LADO / 2.0f + (punto.y - jugador.y) * escala);
}

void Minimapa::dibujarMarca(sf::RenderTarget& destino, sf::Vector2f centro,
                            float radio, sf::Color color) const {
    sf::CircleShape marca(radio);
    marca.setFillColor(color);
    marca.setPosition({centro.x - radio, centro.y - radio});
    destino.draw(marca);
}

// Cuadrado para los cofres. El contorno oscuro lo despega del fondo y de
// cualquier enemigo que le quede encima.
void Minimapa::dibujarCuadrado(sf::RenderTarget& destino, sf::Vector2f centro,
                               float lado, sf::Color color) const {
    sf::RectangleShape marca({lado, lado});
    marca.setFillColor(color);
    marca.setOutlineColor(CONTORNO);
    marca.setOutlineThickness(1.0f);
    marca.setPosition({centro.x - lado / 2.0f, centro.y - lado / 2.0f});
    destino.draw(marca);
}

// Rombo para las estaciones: un circulo de cuatro lados ya sale apoyado en
// una punta, que es justo la forma que lo distingue del cuadrado.
void Minimapa::dibujarRombo(sf::RenderTarget& destino, sf::Vector2f centro,
                            float radio, sf::Color color) const {
    sf::CircleShape marca(radio, 4);
    marca.setFillColor(color);
    marca.setOutlineColor(CONTORNO);
    marca.setOutlineThickness(1.0f);
    marca.setPosition({centro.x - radio, centro.y - radio});
    destino.draw(marca);
}

void Minimapa::dibujar(sf::RenderTarget& destino,
                       Posicion jugador,
                       const std::vector<Enemigo>& enemigos,
                       const std::vector<Posicion>& cofres,
                       const std::vector<Posicion>& estaciones) const {
    // Esquina inferior derecha: la franja de arriba ya la ocupa el HUD.
    const sf::Vector2f tamanio = destino.getView().getSize();
    const sf::Vector2f esquina(tamanio.x - LADO - MARGEN,
                               tamanio.y - LADO - MARGEN);

    sf::RectangleShape fondo({LADO, LADO});
    fondo.setPosition(esquina);
    fondo.setFillColor(FONDO);
    fondo.setOutlineColor(BORDE);
    fondo.setOutlineThickness(1.0f);
    destino.draw(fondo);

    // Dos lineas suaves al centro, para dar sensacion de orientacion.
    sf::RectangleShape eje({LADO, 1.0f});
    eje.setFillColor(RETICULA);
    eje.setPosition({esquina.x, esquina.y + LADO / 2.0f});
    destino.draw(eje);
    eje.setSize({1.0f, LADO});
    eje.setPosition({esquina.x + LADO / 2.0f, esquina.y});
    destino.draw(eje);

    // Todo lo que esta mas lejos que el alcance no se dibuja, salvo el jefe.
    const float alcanceCuadrado = ALCANCE * ALCANCE;

    for (const Posicion& punto : estaciones) {
        if (distanciaCuadrada(punto, jugador) > alcanceCuadrado) { continue; }
        dibujarRombo(destino, aPantalla(punto, jugador, esquina), 4.0f, ESTACION);
    }

    for (const Posicion& punto : cofres) {
        if (distanciaCuadrada(punto, jugador) > alcanceCuadrado) { continue; }
        dibujarCuadrado(destino, aPantalla(punto, jugador, esquina), 6.0f, COFRE);
    }

    for (const Enemigo& enemigo : enemigos) {
        if (!enemigo.estaVivo()) { continue; }

        const FormaColision forma = enemigo.obtenerFormaColision();
        const Posicion centro{forma.x + forma.ancho / 2.0f,
                              forma.y + forma.alto / 2.0f};
        const bool esJefe = enemigo.obtenerIdEnemigo() == 4;
        const bool lejos = distanciaCuadrada(centro, jugador) > alcanceCuadrado;

        if (lejos && !esJefe) { continue; }

        sf::Vector2f punto = aPantalla(centro, jugador, esquina);

        // El jefe se muestra siempre: si quedo fuera del alcance, se lo pega
        // al borde para que se vea de que lado viene.
        if (lejos) {
            const float minimo = esquina.x + 4.0f;
            const float maximo = esquina.x + LADO - 4.0f;
            punto.x = std::min(std::max(punto.x, minimo), maximo);
            const float minimoY = esquina.y + 4.0f;
            const float maximoY = esquina.y + LADO - 4.0f;
            punto.y = std::min(std::max(punto.y, minimoY), maximoY);
        }

        dibujarMarca(destino, punto, esJefe ? 5.0f : 2.5f,
                     colorDeVariante(enemigo.obtenerIdEnemigo()));
    }

    // El jugador va ultimo para que ninguna marca lo tape. Blanco y con
    // contorno: es la referencia desde la que se lee todo lo demas.
    const sf::Vector2f centro(esquina.x + LADO / 2.0f, esquina.y + LADO / 2.0f);
    sf::CircleShape yo(4.0f);
    yo.setFillColor(JUGADOR);
    yo.setOutlineColor(CONTORNO);
    yo.setOutlineThickness(1.5f);
    yo.setPosition({centro.x - 4.0f, centro.y - 4.0f});
    destino.draw(yo);
}
