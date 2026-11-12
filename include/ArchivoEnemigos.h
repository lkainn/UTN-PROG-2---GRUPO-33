#pragma once

#include "Enemigo.h"

#include <cstdio>
#include <cstdint>
#include <vector>

// Un renglon de Enemigos.dat: las estadisticas de una variante de enemigo.
//
// Son los campos previstos en el informe mas ancho y alto, que hacen falta
// para construir el enemigo y para su rectangulo de colision. Sin ellos el
// catalogo no alcanzaria y los tamanios quedarian sueltos en el codigo.
//
// Solo tipos simples y arreglos de char: un std::string guarda internamente un
// puntero, y volcarlo al archivo guardaria una direccion de memoria que manana
// no significa nada.
struct RegistroEnemigo {
    int idEnemigo;
    char nombre[30];
    int vida;
    int dano;
    float velocidad;
    int experienciaOtorgada;
    int chatarraOtorgada;
    float ancho;
    float alto;
    std::uint32_t eliminado = 0;
};

// Catalogo de variantes de enemigo guardado en datos/Enemigos.dat.
//
// El archivo es obligatorio y se lee al empezar la partida.
class ArchivoEnemigos {
    public:
        static const char* RUTA;

        // Devuelve cuatro registros ordenados por ID o informa un error de lectura.
        std::vector<RegistroEnemigo> leerCatalogo();

        bool guardarCatalogo(const std::vector<RegistroEnemigo>& catalogo);

};

// Construye un enemigo de la variante descrita por el registro, en la posicion
// indicada. Es el reemplazo de crearEnemigo() una vez que los valores salen
// del archivo en lugar de estar escritos en el codigo.
Enemigo crearEnemigoDesdeCatalogo(const RegistroEnemigo& registro,
                                  float x, float y);
