#pragma once

#include "TiposCompartidos.h"

#include <cstdint>

enum TipoEnemigo { // Diferentes tipos de enemigo
    BASICO,
    RAPIDO,
    RESISTENTE,
    JEFE        // aparece una sola vez por partida; derrotarlo es la victoria
};

class Enemigo {
    private:
        // Contador compartido por TODOS los enemigos: no pertenece a ninguno
        // en particular. Cada enemigo nuevo se lleva el valor actual y lo
        // incrementa, asi dos enemigos nunca comparten identificador.
        static std::uint64_t siguienteIdInstancia;

        std::uint64_t idInstancia;  // unico por enemigo; no se reutiliza nunca
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
        int obtenerIdEnemigo() const; // 1 basico, 2 rapido, 3 resistente
        // Identifica a ESTE enemigo, no a su variante. Sirve para no aplicar
        // dos veces el mismo efecto sobre el mismo enemigo (por ejemplo un
        // proyectil que atraviesa varios) y para contar sin repetir.
        std::uint64_t obtenerIdInstancia() const;
        int obtenerDano() const;
        Recompensa obtenerRecompensa() const;
        void recibirDanio(int cantidad);
        bool estaVivo() const;
};

Enemigo crearEnemigo(TipoEnemigo tipo, float x, float y);
