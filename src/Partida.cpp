#include "Partida.h"

const int Partida::PUNTOS_POR_ENEMIGO = 10;
const int Partida::PUNTOS_POR_CHATARRA = 2;
const int Partida::PUNTOS_POR_VICTORIA = 5000;

Partida::Partida(int _idJugador)
    : idPartida(0),           // lo asigna ArchivoPartidas al guardar
      idJugador(_idJugador),
      puntaje(0),
      nivelAlcanzado(1),
      enemigosEliminados(0),
      chatarraObtenida(0),
      duracionSegundos(0.0f),
      resultado(ResultadoPartida::EnCurso) {
}

// El reloj se detiene al terminar: la duracion que se guarda es la del momento
// del desenlace, no la de cuando el jugador cierra la pantalla de resultado.
void Partida::actualizar(float dt) {
    if (resultado != ResultadoPartida::EnCurso) {
        return;
    }
    if (dt > 0.0f) {
        duracionSegundos += dt;
    }
}

void Partida::registrarEliminados(int cantidad) {
    if (cantidad <= 0) {
        return;
    }
    enemigosEliminados += cantidad;
    recalcularPuntaje();
}

void Partida::registrarChatarra(int cantidad) {
    if (cantidad <= 0) {
        return;
    }
    chatarraObtenida += cantidad;
    recalcularPuntaje();
}

// Se guarda el mayor alcanzado y no el ultimo informado: asi una mejora que
// baje el nivel, o un informe fuera de orden, no empeoran el registro.
void Partida::registrarNivel(int nivel) {
    if (nivel > nivelAlcanzado) {
        nivelAlcanzado = nivel;
        recalcularPuntaje();
    }
}

// Los dos desenlaces solo valen si la partida seguia en curso. Sin esta
// guarda, un jugador que muere en el mismo fotograma en que cae el jefe
// podria terminar con derrota pisando la victoria, o al reves, segun el orden
// en que el ciclo de juego revise cada condicion.
void Partida::marcarVictoria() {
    if (resultado == ResultadoPartida::EnCurso) {
        resultado = ResultadoPartida::Victoria;
        recalcularPuntaje();
    }
}

void Partida::marcarDerrota() {
    if (resultado == ResultadoPartida::EnCurso) {
        resultado = ResultadoPartida::Derrota;
    }
}

bool Partida::estaEnCurso() const {
    return resultado == ResultadoPartida::EnCurso;
}

ResultadoPartida Partida::obtenerResultado() const {
    return resultado;
}

bool Partida::fueVictoria() const {
    return resultado == ResultadoPartida::Victoria;
}

int Partida::obtenerIdPartida() const {
    return idPartida;
}

int Partida::obtenerIdJugador() const {
    return idJugador;
}

int Partida::obtenerPuntaje() const {
    return puntaje;
}

int Partida::obtenerNivelAlcanzado() const {
    return nivelAlcanzado;
}

int Partida::obtenerEnemigosEliminados() const {
    return enemigosEliminados;
}

int Partida::obtenerChatarraObtenida() const {
    return chatarraObtenida;
}

int Partida::obtenerDuracionSegundos() const {
    return (int)duracionSegundos;
}

void Partida::asignarIdPartida(int id) {
    if (id > 0) {
        idPartida = id;
    }
}

// El puntaje se recalcula entero en vez de irse sumando, para que no dependa
// del orden en que llegaron los datos y para que la formula se lea de una vez.
//
// La cuenta se hace en long long y se recorta, porque el producto de dos int
// grandes desborda silenciosamente: no avisa, devuelve un numero negativo.
void Partida::recalcularPuntaje() {
    long long total = 0;
    total += (long long)enemigosEliminados * PUNTOS_POR_ENEMIGO;
    total += (long long)chatarraObtenida * PUNTOS_POR_CHATARRA;

    if (resultado == ResultadoPartida::Victoria) {
        total += PUNTOS_POR_VICTORIA;
    }

    const long long MAXIMO = 2147483647LL;  // el mayor int que entra en el .dat
    puntaje = (int)(total > MAXIMO ? MAXIMO : total);
}
