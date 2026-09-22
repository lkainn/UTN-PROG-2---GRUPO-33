#pragma once

#include "Comunes.h"

class Enemigo {
    private:
        int idEnemigo;
        float x, y;
        float velocidad;
        int vida;
        int dano;
        char nombre[30];
        int experienciaOtorgada;
        int chatarraOtorgada;
    public:
        Enemigo(int , float , float , float , int , int , const char* , int , int );
        void actualizar(Posicion objetivo, float dt);
        Posicion obtenerPosicion() const;
        void recibirDanio(int cantidad);
        bool estaVivo() const;
};
