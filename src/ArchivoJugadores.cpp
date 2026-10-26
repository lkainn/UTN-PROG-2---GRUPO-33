#include "ArchivoJugadores.h"

bool ArchivoJugadores::guardar(const RegistroJugador& reg) {
    FILE *p = fopen(rutaArchivo, "wb");
    if (p == NULL) {
        return false;
    }

bool ok = fwrite(&reg, sizeof(RegistroJugador), 1, p) == 1;
    fclose(p);
    return ok;
}

RegistroJugador ArchivoJugadores::leer() {
    RegistroJugador reg = {1, 0, 0};  // Valores iniciales jugador nuevo

FILE *p = fopen(rutaArchivo, "rb");
    if (p == NULL) {
	return reg;   // Si el archivo no existe, devuelve el perfil básico
}

    fread(&reg, sizeof(RegistroJugador), 1, p);
    fclose(p);
    return reg;
}

