#include "Enemigo.h"
#include "ArchivoPartidaEnCurso.h"
#include <cstring>
#include <cmath>

// Arranca en 1 y no en 0 a proposito: Proyectil::registrarImpacto descarta el
// identificador 0, asi que un enemigo con ese id nunca recibiria impactos.
std::uint64_t Enemigo::siguienteIdInstancia = 1;

// Iniciamos el constructor
Enemigo::Enemigo(int _idEnemigo, float _x, float _y, float _ancho, float _alto, float _velocidad, int _vida, int _dano, const char* _nombre, int _experienciaOtorgada, int _chatarraOtorgada){
    // Se toma el numero libre y se deja el siguiente listo para el proximo
    // enemigo que se cree.
    idInstancia = siguienteIdInstancia;
    siguienteIdInstancia++;

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

std::uint64_t Enemigo::obtenerIdInstancia() const {
    return idInstancia;
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

bool Enemigo::transferirEstado(ArchivoPartidaEnCurso& archivo)
{
    return archivo(idInstancia, idEnemigo, x, y, ancho, alto, velocidad, vida, dano,
                   experienciaOtorgada, chatarraOtorgada) &&
        archivo.bytes(nombre, sizeof(nombre)) && nombre[sizeof(nombre) - 1] == '\0' &&
        idInstancia > 0 && idEnemigo >= 1 && idEnemigo <= 4 &&
        ancho > 0.0f && alto > 0.0f && velocidad >= 0.0f && vida >= 0 && dano >= 0 &&
        experienciaOtorgada >= 0 && chatarraOtorgada >= 0;
}

std::uint64_t Enemigo::obtenerSiguienteId()
{
    return siguienteIdInstancia;
}

void Enemigo::reservarIdentificadores(std::uint64_t siguiente)
{
    if (siguiente > siguienteIdInstancia)
    {
        siguienteIdInstancia = siguiente;
    }
}
