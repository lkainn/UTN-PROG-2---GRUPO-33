#include "ArchivoMejoras.h"

bool ArchivoMejoras::guardarCatalogo(const std::vector<Mejora>& catalogo) {
    FILE *p = fopen(rutaArchivo, "wb");
    if (p == NULL) return false;

bool ok = fwrite(catalogo.data(), sizeof(Mejora), catalogo.size(), p) == catalogo.size();
    fclose(p);
    return ok;
}

std::vector<Mejora> ArchivoMejoras::leerCatalogo() {
    std::vector<Mejora> catalogo;
    FILE *p = fopen(rutaArchivo, "rb");

    if (p == NULL) {
        // Catalogo por defecto para la primera partida
        Mejora m1 = {1, "Vida Maxima", 0, 5, "Aumenta tu salud limite"};
        Mejora m2 = {2, "Velocidad", 0, 3, "Corres mas rapido"};
        catalogo.push_back(m1);
        catalogo.push_back(m2);

        guardarCatalogo(catalogo); // Escribe Mejoras.dat en disco
        return catalogo;
    }

    Mejora m;
    while (fread(&m, sizeof(Mejora), 1, p) == 1) {
        catalogo.push_back(m);
    }
    fclose(p);
    return catalogo;
}

