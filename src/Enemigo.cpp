#include "Enemigo.h"
#include <cstring>
#include <cmath>

// Iniciamos el constructor
Enemigo::Enemigo(int _idEnemigo, float _x, float _y, float _velocidad, int _vida, int _dano, const char* _nombre, int _experienciaOtorgada, int _chatarraOtorgada){
    idEnemigo = _idEnemigo;
    x = _x;
    y = _y;
    velocidad = _velocidad;
    vida = _vida;
    dano = _dano;
    strncpy(nombre, _nombre, 30);
    nombre[29] = '\0';
    experienciaOtorgada = _experienciaOtorgada;
    chatarraOtorgada = _chatarraOtorgada;
}

// Iniciamos los metodos
void Enemigo::actualizar(Posicion objetivo, float dt) {
    // 1. Vector que apunta del enemigo hacia el objetivo
    float dx = objetivo.x - x;
    float dy = objetivo.y - y;

    // 2. Largo de ese vector: la distancia que los separa
    float distancia = std::sqrt(dx * dx + dy * dy);

    // 3. Si ya esta encima del objetivo no hay hacia donde ir
    if (distancia > 0.0f) {
        // Normalizamos: el vector queda de largo 1, solo direccion
        dx /= distancia;
        dy /= distancia;

        // 4. Avanzamos segun la velocidad y el tiempo transcurrido
        x += dx * velocidad * dt;
        y += dy * velocidad * dt;
    }
}

Posicion Enemigo::obtenerPosicion()const{ // informa donde esta el enemigo
    Posicion p;
    p.x = x;
    p.y = y;
    return p;
}

void Enemigo::recibirDanio(int cantidad){ // aca calculamos el dano recibido
    vida = vida - cantidad;
    if(vida < 0){
        vida = 0;
    }
}

bool Enemigo::estaVivo()const{ // aca verificamos si esta vivo o no el enemigo
    return vida > 0;
}
