#pragma once

#include <direct.h>

// Crea la carpeta "datos" si todavia no existe.
//
// Hace falta porque fopen NO crea carpetas: si se abre "datos/Algo.dat" en
// modo escritura y la carpeta no esta, devuelve NULL y el guardado se pierde
// sin ningun aviso.
//
// Eso pasa, por ejemplo, al ejecutar el juego haciendo doble clic sobre
// bin\Debug\ProyectoTPI.exe: la ruta "datos/" se resuelve contra el directorio
// de trabajo, que ahi es bin\Debug y no la raiz del proyecto.
//
// _mkdir devuelve -1 si la carpeta ya existia, y eso no es un problema: lo
// unico que importa es que exista despues de llamarla.
inline void asegurarCarpetaDatos() {
    _mkdir("datos");
}
