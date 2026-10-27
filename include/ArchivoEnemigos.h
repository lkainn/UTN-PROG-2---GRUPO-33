#pragma once

#include "Enemigo.h"

#include <cstdio>
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
};

// Catalogo de variantes de enemigo guardado en datos/Enemigos.dat.
//
// El archivo se lee una vez al empezar la partida. Si no existe, se crea con
// los valores de fabrica, asi la primera ejecucion en una maquina nueva
// funciona sin pasos previos.
class ArchivoEnemigos {
    public:
        static const char* RUTA;

        // Devuelve el catalogo completo. Si el archivo falta o esta
        // inservible, devuelve los valores de fabrica y los escribe.
        std::vector<RegistroEnemigo> leerCatalogo();

        bool guardarCatalogo(const std::vector<RegistroEnemigo>& catalogo);

        // Los valores de fabrica, sin tocar el disco. Separado para poder
        // compararlo contra lo leido en las pruebas.
        static std::vector<RegistroEnemigo> catalogoDeFabrica();
};

// Construye un enemigo de la variante descrita por el registro, en la posicion
// indicada. Es el reemplazo de crearEnemigo() una vez que los valores salen
// del archivo en lugar de estar escritos en el codigo.
Enemigo crearEnemigoDesdeCatalogo(const RegistroEnemigo& registro,
                                  float x, float y);
