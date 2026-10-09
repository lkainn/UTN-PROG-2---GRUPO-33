#include "GestorOleadas.h"

#include <algorithm>
#include <cmath>

const float GestorOleadas::DISTANCIA_APARICION = 700.0f;

const float GestorOleadas::SEGUNDOS_POR_OLEADA = 30.0f;
const float GestorOleadas::SEGUNDOS_DIFICULTAD_MAXIMA = 300.0f;

const float GestorOleadas::INTERVALO_APARICION_INICIAL = 1.5f;
const float GestorOleadas::INTERVALO_APARICION_MINIMO = 0.4f;

const std::size_t GestorOleadas::MAXIMO_VIVOS_INICIAL = 15;
const std::size_t GestorOleadas::MAXIMO_VIVOS_FINAL = 60;

const float GestorOleadas::SEGUNDOS_APARECE_RAPIDO = 60.0f;
const float GestorOleadas::SEGUNDOS_APARECE_RESISTENTE = 180.0f;

namespace {

// Dos veces pi: una vuelta completa en radianes, que es la unidad que usan
// cos() y sin().
const float VUELTA_COMPLETA = 6.2831853f;

}  // namespace

GestorOleadas::GestorOleadas()
    : enemigos(),
      tiempoPartida(0.0f),
      tiempoAcumulado(0.0f),
      // Semilla fija a proposito: mientras estamos probando conviene que las
      // partidas sean repetibles. Para que cada partida sea distinta se
      // reemplaza por std::random_device{}().
      generador(20261005u) {
}

void GestorOleadas::actualizar(Posicion posicionJugador, float dt) {
    // El reloj de partida es la base de toda la dificultad: cada cuanto
    // aparecen, cuantos caben a la vez y que variantes estan habilitadas.
    tiempoPartida += dt;

    // El orden importa. Primero aparecen los nuevos, despues se mueven todos,
    // y al final se limpian los muertos en una pasada aparte: borrar mientras
    // se recorre el vector corrompe el recorrido.
    generarSiCorresponde(posicionJugador, dt);
    moverEnemigos(posicionJugador, dt);
    quitarMuertos();
}

void GestorOleadas::reiniciar() {
    enemigos.clear();
    tiempoPartida = 0.0f;
    tiempoAcumulado = 0.0f;
}

const std::vector<Enemigo>& GestorOleadas::obtenerEnemigos() const {
    return enemigos;
}

std::size_t GestorOleadas::cantidadEnemigos() const {
    return enemigos.size();
}

std::vector<Enemigo>& GestorOleadas::obtenerEnemigos()
{
    return enemigos;
}

void GestorOleadas::danarATodos(int cantidad) {
    for (Enemigo& enemigo : enemigos) {
        enemigo.recibirDanio(cantidad);
    }
}

float GestorOleadas::tiempoDePartida() const {
    return tiempoPartida;
}

// Las oleadas son tramos de tiempo, no grupos de enemigos: no hay que
// eliminarlos a todos para pasar a la siguiente. La oleada 1 son los primeros
// SEGUNDOS_POR_OLEADA segundos, y asi.
int GestorOleadas::oleadaActual() const {
    return (int)(tiempoPartida / SEGUNDOS_POR_OLEADA) + 1;
}

// 0 al empezar la partida, 1 cuando se alcanza la dificultad maxima. De ahi
// en adelante queda en 1: la dificultad deja de subir.
float GestorOleadas::progresoDificultad() const {
    if (tiempoPartida >= SEGUNDOS_DIFICULTAD_MAXIMA) {
        return 1.0f;
    }
    return tiempoPartida / SEGUNDOS_DIFICULTAD_MAXIMA;
}

// Interpolacion lineal entre el intervalo inicial y el minimo: empieza en 1.5
// segundos y termina en 0.4. Con progreso 0.5 da el valor del medio.
float GestorOleadas::intervaloActual() const {
    const float recorrido =
        INTERVALO_APARICION_INICIAL - INTERVALO_APARICION_MINIMO;
    return INTERVALO_APARICION_INICIAL - recorrido * progresoDificultad();
}

// Misma interpolacion, pero al reves: el techo sube de 15 a 60.
std::size_t GestorOleadas::maximoVivosActual() const {
    const float recorrido =
        (float)(MAXIMO_VIVOS_FINAL - MAXIMO_VIVOS_INICIAL);
    const float actual =
        (float)MAXIMO_VIVOS_INICIAL + recorrido * progresoDificultad();
    return (std::size_t)actual;
}

// Elige un punto al azar del anillo que rodea al centro.
//
// cos() y sin() convierten un angulo en una direccion: con el angulo variando
// de 0 a una vuelta completa, el punto recorre todo el circulo. Multiplicando
// por la distancia, queda sobre el anillo.
Posicion GestorOleadas::posicionDeAparicion(Posicion centro) {
    std::uniform_real_distribution<float> angulos(0.0f, VUELTA_COMPLETA);
    const float angulo = angulos(generador);

    Posicion aparicion;
    aparicion.x = centro.x + std::cos(angulo) * DISTANCIA_APARICION;
    aparicion.y = centro.y + std::sin(angulo) * DISTANCIA_APARICION;
    return aparicion;
}

// Las variantes se van habilitando con el tiempo: al principio solo basicos,
// despues entran los rapidos y al final los resistentes. Entre las
// habilitadas se sortea con la misma probabilidad.
TipoEnemigo GestorOleadas::variantePorAparecer() {
    int habilitadas = 1;  // el basico esta desde el segundo cero

    if (tiempoPartida >= SEGUNDOS_APARECE_RAPIDO) {
        habilitadas = 2;
    }
    if (tiempoPartida >= SEGUNDOS_APARECE_RESISTENTE) {
        habilitadas = 3;
    }

    std::uniform_int_distribution<int> sorteo(0, habilitadas - 1);

    switch (sorteo(generador)) {
        case 1:
            return RAPIDO;
        case 2:
            return RESISTENTE;
        default:
            return BASICO;
    }
}

void GestorOleadas::generarSiCorresponde(Posicion posicionJugador, float dt) {
    // Temporizador por acumulacion: no se puede "esperar" dentro de un juego
    // porque congelaria todo, asi que se suma el tiempo de cada fotograma
    // hasta llegar al intervalo.
    tiempoAcumulado += dt;

    if (tiempoAcumulado < intervaloActual()) {
        return;
    }

    tiempoAcumulado = 0.0f;

    // El techo se consulta recien aca: el temporizador corre igual, pero si ya
    // hay demasiados vivos esta aparicion se descarta.
    if (enemigos.size() >= maximoVivosActual()) {
        return;
    }

    const Posicion aparicion = posicionDeAparicion(posicionJugador);
    enemigos.push_back(
        crearEnemigo(variantePorAparecer(), aparicion.x, aparicion.y));
}

void GestorOleadas::moverEnemigos(Posicion posicionJugador, float dt) {
    // El & es obligatorio: sin el, el for entrega una copia de cada enemigo,
    // se mueve la copia y el original queda quieto. No avisa nada.
    for (Enemigo& enemigo : enemigos) {
        // El enemigo persigue la esquina que deja su centro sobre el centro
        // del jugador. Sin esta correccion perseguiria con su esquina
        // superior izquierda y quedaria siempre corrido medio cuerpo.
        const FormaColision forma = enemigo.obtenerFormaColision();
        Posicion objetivo;
        objetivo.x = posicionJugador.x - forma.ancho / 2.0f;
        objetivo.y = posicionJugador.y - forma.alto / 2.0f;

        enemigo.actualizar(objetivo, dt);
    }
}

void GestorOleadas::quitarMuertos() {
    // remove_if no borra: empuja los muertos al final y devuelve donde empieza
    // esa zona. El erase corta desde ahi hasta el final.
    enemigos.erase(
        std::remove_if(enemigos.begin(), enemigos.end(),
                       [](const Enemigo& enemigo) {
                           return !enemigo.estaVivo();
                       }),
        enemigos.end());
}
