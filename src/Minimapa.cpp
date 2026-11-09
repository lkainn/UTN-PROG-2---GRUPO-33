#include "Minimapa.h"

#include <algorithm>
#include <cmath>
#include <string>

const float Minimapa::LADO = 280.0f;
const float Minimapa::ALCANCE = 1800.0f;
const float Minimapa::MARGEN = 18.0f;
const float Minimapa::ICONO = 20.0f;
const float Minimapa::ICONO_JEFE = 32.0f;
const float Minimapa::ICONO_OBJETO = 22.0f;

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

// Los iconos salen del primer cuadro de cada atlas de animacion, que ya es
// la silueta recortada del bicho. El jefe usa celdas de 256 y el resto de
// 192, igual que en AspectoEnemigos.
Minimapa::Minimapa() : _iconos{}, _iconoCargado{}, _objetos{}, _objetoCargado{} {
    _iconoCargado[0] = cargarIcono(0, "recursos/enemigos/mordaza_atlas", 192);
    _iconoCargado[1] = cargarIcono(1, "recursos/enemigos/zancudo_atlas", 192);
    _iconoCargado[2] = cargarIcono(2, "recursos/enemigos/yunque_atlas", 192);
    _iconoCargado[3] = cargarIcono(3, "recursos/enemigos/desguazador_atlas", 256);

    _objetoCargado[0] = cargarObjeto(0, "recursos/encuentros/contenedor_suministros_v1.png");
    _objetoCargado[1] = cargarObjeto(1, "recursos/encuentros/estacion_mejoras_v1.png");
}

// Las imagenes de los encuentros son atlas de dos estados, uno al lado del
// otro: cerrado y abierto. El radar solo muestra los que no se usaron, asi
// que alcanza con el primero.
//
// Dentro de esa mitad el dibujo no llega a los bordes, y un icono de veinte
// pixeles con margen transparente se ve diminuto. Por eso se busca el
// rectangulo que realmente tiene pixeles y se recorta ahi.
bool Minimapa::cargarObjeto(int indice, const char* ruta) {
    sf::Image imagen;
    if (!imagen.loadFromFile(ruta)) {
        return false;
    }

    const sf::Vector2u tamanio = imagen.getSize();
    const unsigned int anchoCelda = tamanio.x / 2;
    if (anchoCelda == 0 || tamanio.y == 0) {
        return false;
    }

    unsigned int izquierda = anchoCelda;
    unsigned int derecha = 0;
    unsigned int arriba = tamanio.y;
    unsigned int abajo = 0;

    for (unsigned int y = 0; y < tamanio.y; ++y) {
        for (unsigned int x = 0; x < anchoCelda; ++x) {
            if (imagen.getPixel({x, y}).a <= 20) {
                continue;   // transparente: no cuenta como dibujo
            }
            izquierda = std::min(izquierda, x);
            derecha = std::max(derecha, x);
            arriba = std::min(arriba, y);
            abajo = std::max(abajo, y);
        }
    }

    if (derecha < izquierda || abajo < arriba) {
        return false;   // la celda estaba vacia
    }

    const sf::IntRect recorte(
        {(int)izquierda, (int)arriba},
        {(int)(derecha - izquierda + 1), (int)(abajo - arriba + 1)});

    if (!_objetos[indice].loadFromImage(imagen, false, recorte)) {
        return false;
    }
    _objetos[indice].setSmooth(true);
    return true;
}

bool Minimapa::dibujarObjeto(sf::RenderTarget& destino, int indice,
                             sf::Vector2f centro, float lado) const {
    if (indice < 0 || indice >= (int)_objetos.size() || !_objetoCargado[indice]) {
        return false;
    }

    const sf::Vector2u tamanio = _objetos[indice].getSize();
    if (tamanio.x == 0 || tamanio.y == 0) {
        return false;
    }

    // Se escala por el lado mas largo para que el icono entre completo sin
    // deformarse, sea alto o ancho.
    const float escala = lado / (float)std::max(tamanio.x, tamanio.y);
    sf::Sprite icono(_objetos[indice]);
    icono.setScale({escala, escala});
    icono.setPosition({centro.x - (float)tamanio.x * escala / 2.0f,
                       centro.y - (float)tamanio.y * escala / 2.0f});
    destino.draw(icono);
    return true;
}

bool Minimapa::cargarIcono(int indice, const char* carpeta, int celda) {
    const std::string ruta = std::string(carpeta) + "/cuerpo_0.png";

    // Solo la primera celda del atlas: el resto son los demas cuadros de la
    // caminata, que para un icono no hacen falta.
    if (!_iconos[indice].loadFromFile(ruta, false,
                                      sf::IntRect({0, 0}, {celda, celda}))) {
        return false;
    }
    _iconos[indice].setSmooth(true);   // se ve a un decimo de su tamanio
    return true;
}

bool Minimapa::dibujarIcono(sf::RenderTarget& destino, int idEnemigo,
                            sf::Vector2f centro, float lado) const {
    const int indice = idEnemigo - 1;
    if (indice < 0 || indice >= (int)_iconos.size() || !_iconoCargado[indice]) {
        return false;
    }

    sf::Sprite icono(_iconos[indice]);
    const sf::Vector2u tamanio = _iconos[indice].getSize();
    if (tamanio.x == 0 || tamanio.y == 0) {
        return false;
    }

    const float escala = lado / (float)tamanio.x;
    icono.setScale({escala, escala});
    icono.setPosition({centro.x - lado / 2.0f,
                       centro.y - (float)tamanio.y * escala / 2.0f});
    destino.draw(icono);
    return true;
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
        const sf::Vector2f donde = aPantalla(punto, jugador, esquina);
        if (!dibujarObjeto(destino, 1, donde, ICONO_OBJETO)) {
            dibujarRombo(destino, donde, 4.0f, ESTACION);
        }
    }

    for (const Posicion& punto : cofres) {
        if (distanciaCuadrada(punto, jugador) > alcanceCuadrado) { continue; }
        const sf::Vector2f donde = aPantalla(punto, jugador, esquina);
        if (!dibujarObjeto(destino, 0, donde, ICONO_OBJETO)) {
            dibujarCuadrado(destino, donde, 6.0f, COFRE);
        }
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

        const int id = enemigo.obtenerIdEnemigo();
        if (!dibujarIcono(destino, id, punto, esJefe ? ICONO_JEFE : ICONO)) {
            // Sin la imagen a mano, la marca de color cumple igual.
            dibujarMarca(destino, punto, esJefe ? 5.0f : 2.5f,
                         colorDeVariante(id));
        }
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
