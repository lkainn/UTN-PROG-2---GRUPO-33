#pragma once

/*Datos compartidos

 Tipo y Significado

 Posicion: Coordenadas x e y en el espacio de juego.
 FormaColision: Rectangulo con posicion x, y y dimensiones ancho, alto.
 Recompensa: Experiencia y chatarra otorgadas al eliminar un enemigo.
*/


struct Posicion {
    float x;
    float y;
};

struct FormaColision {
    float x;
    float y;
    float ancho;
    float alto;
};

struct Recompensa {
    int experiencia;
    int chatarra;
};
