#include "ArchivoEnemigos.h"

#include "CarpetaDatos.h"

#include <cstring>
#include <cmath>

const char* ArchivoEnemigos::RUTA = "datos/Enemigos.dat";

namespace {

// Arma un registro copiando el nombre al arreglo de tamanio fijo. strncpy no
// agrega el cero final si el texto llega justo al limite, por eso se fuerza.
RegistroEnemigo armarRegistro(int id, const char* nombre, int vida, int dano,
                              float velocidad, int experiencia, int chatarra,
                              float ancho, float alto) {
    RegistroEnemigo registro;
    registro.idEnemigo = id;
    std::strncpy(registro.nombre, nombre, 30);
    registro.nombre[29] = '\0';
    registro.vida = vida;
    registro.dano = dano;
    registro.velocidad = velocidad;
    registro.experienciaOtorgada = experiencia;
    registro.chatarraOtorgada = chatarra;
    registro.ancho = ancho;
    registro.alto = alto;
    return registro;
}

// Un registro leido puede venir de un archivo editado a mano o cortado por la
// mitad. Antes de usarlo se revisa que los valores tengan sentido.
bool registroEsUsable(const RegistroEnemigo& registro) {
    return registro.idEnemigo >= 1 && registro.idEnemigo <= 4 &&
           registro.vida > 0 && registro.dano >= 0 &&
           std::isfinite(registro.velocidad) && registro.velocidad > 0.0f &&
           registro.experienciaOtorgada >= 0 &&
           registro.chatarraOtorgada >= 0 &&
           std::isfinite(registro.ancho) && std::isfinite(registro.alto) &&
           registro.ancho > 0.0f && registro.alto > 0.0f &&
           std::memchr(registro.nombre, '\0', sizeof(registro.nombre)) != nullptr;
}

}  // namespace

// Los mismos valores que antes estaban escritos en crearEnemigo(). Ahora viven
// en un solo lugar y de aca salen tanto el archivo como el juego.
std::vector<RegistroEnemigo> ArchivoEnemigos::catalogoDeFabrica() {
    std::vector<RegistroEnemigo> catalogo;
    catalogo.push_back(armarRegistro(1, "Robot basico", 100, 10, 120.0f,
                                     5, 3, 56.0f, 56.0f));
    catalogo.push_back(armarRegistro(2, "Robot rapido", 60, 8, 220.0f,
                                     7, 2, 40.0f, 40.0f));
    catalogo.push_back(armarRegistro(3, "Robot resistente", 250, 15, 70.0f,
                                     12, 6, 72.0f, 72.0f));
    catalogo.push_back(armarRegistro(4, "Jefe chatarrero", 3000, 35, 85.0f,
                                     200, 120, 120.0f, 120.0f));
    return catalogo;
}

bool ArchivoEnemigos::guardarCatalogo(
        const std::vector<RegistroEnemigo>& catalogo) {
    if (catalogo.empty()) {
        return false;
    }
    for (const RegistroEnemigo& registro : catalogo)
    {
        if (!registroEsUsable(registro))
        {
            return false;
        }
    }

    // fopen no crea carpetas: sin esto, al ejecutar desde otro directorio el
    // guardado falla en silencio.
    asegurarCarpetaDatos();

    FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "wb");
    if (archivo == NULL) {
        return false;  // el disco esta protegido o la ruta es invalida
    }

    const std::size_t escritos =
        fwrite(catalogo.data(), sizeof(RegistroEnemigo), catalogo.size(),
               archivo);
    const bool cerrado = fclose(archivo) == 0;

    return escritos == catalogo.size() && cerrado;
}

std::vector<RegistroEnemigo> ArchivoEnemigos::leerCatalogo() {
    FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "rb");

    // Primera ejecucion en esta maquina: se crea el archivo con los valores de
    // fabrica para que la partida pueda empezar igual.
    if (archivo == NULL) {
        const std::vector<RegistroEnemigo> fabrica = catalogoDeFabrica();
        guardarCatalogo(fabrica);
        return fabrica;
    }

    std::vector<RegistroEnemigo> catalogo;
    RegistroEnemigo registro;

    // fread devuelve cuantos registros COMPLETOS leyo. Al llegar al final, o
    // si el archivo quedo cortado por la mitad, devuelve algo distinto de 1 y
    // el bucle termina sin cargar basura.
    while (fread(&registro, sizeof(RegistroEnemigo), 1, archivo) == 1) {
        if (registroEsUsable(registro)) {
            catalogo.push_back(registro);
        }
    }
    fclose(archivo);

    // El archivo existia pero no quedo nada aprovechable: se rehace.
    if (catalogo.empty()) {
        const std::vector<RegistroEnemigo> fabrica = catalogoDeFabrica();
        guardarCatalogo(fabrica);
        return fabrica;
    }

    return catalogo;
}

Enemigo crearEnemigoDesdeCatalogo(const RegistroEnemigo& registro,
                                  float x, float y) {
    return Enemigo(registro.idEnemigo, x, y, registro.ancho, registro.alto,
                   registro.velocidad, registro.vida, registro.dano,
                   registro.nombre, registro.experienciaOtorgada,
                   registro.chatarraOtorgada);
}
