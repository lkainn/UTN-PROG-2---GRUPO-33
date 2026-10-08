#include "Enemigo.h"
#include <cstring>
#include <cmath>

// Iniciamos el constructor
Enemigo::Enemigo(int _idEnemigo, float _x, float _y, float _ancho, float _alto, float _velocidad, int _vida, int _dano, const char* _nombre, int _experienciaOtorgada, int _chatarraOtorgada){
    idEnemigo = _idEnemigo;
    x = _x;
    y = _y;
    ancho = _ancho;
    alto = _alto;
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
        // Cuanto avanzaria en este frame
        float paso = velocidad * dt;

        if (paso >= distancia) {
            // Llegaria o se pasaria de largo: lo dejamos justo encima del
            // objetivo. Sin esto vibra al llegar, porque cada frame se pasa
            // para un lado y al siguiente vuelve para el otro.
            x = objetivo.x;
            y = objetivo.y;
        } else {
            // Normalizamos: el vector queda de largo 1, solo direccion
            dx /= distancia;
            dy /= distancia;

            // 4. Avanzamos segun la velocidad y el tiempo transcurrido
            x += dx * paso;
            y += dy * paso;
        }
    }
}

Posicion Enemigo::obtenerPosicion()const{ // informa donde esta el enemigo
    Posicion p;
    p.x = x;
    p.y = y;
    return p;
}

// Rectangulo que usa el SistemaColisiones para detectar impactos.
// x e y son la esquina superior izquierda, igual que en el resto del equipo.
FormaColision Enemigo::obtenerFormaColision()const{
    FormaColision forma;
    forma.x = x;
    forma.y = y;
    forma.ancho = ancho;
    forma.alto = alto;
    return forma;
}

// Sirve para saber de que variante es un enemigo ya creado: quien lo dibuja
// elige el color segun este id, y mas adelante identifica su fila en
// Enemigos.dat.
int Enemigo::obtenerIdEnemigo() const {
    return idEnemigo;
}

int Enemigo::obtenerDano() const {
    return dano;
}

Recompensa Enemigo::obtenerRecompensa() const {
    return {experienciaOtorgada, chatarraOtorgada};
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

Enemigo crearEnemigo(TipoEnemigo tipo, float x, float y) {
    switch (tipo) {
        case RAPIDO:
            return Enemigo(
                2, x, y, 40.0f, 40.0f, 220.0f,
                60, 8, "Robot rapido", 7, 2);

        case RESISTENTE:
            return Enemigo(
                3, x, y, 72.0f, 72.0f, 70.0f,
                250, 15, "Robot resistente", 12, 6);

        case BASICO:
        default:
            return Enemigo(
                1, x, y, 56.0f, 56.0f, 120.0f,
                100, 10, "Robot basico", 5, 3);
    }
}
