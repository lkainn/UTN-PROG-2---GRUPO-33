#pragma once

#include "TiposCompartidos.h"

class Enemigo {
    private:
        int idEnemigo;
        float x, y;              // esquina superior izquierda, no el centro
        float ancho, alto;
        float velocidad;
        int vida;
        int dano;
        char nombre[30];
        int experienciaOtorgada;
        int chatarraOtorgada;
    public:
        Enemigo(int , float , float , float , float , float , int , int , const char* , int , int );
        void actualizar(Posicion objetivo, float dt);
        Posicion obtenerPosicion() const;
        FormaColision obtenerFormaColision() const;
        void recibirDanio(int cantidad);
        bool estaVivo() const;
};
