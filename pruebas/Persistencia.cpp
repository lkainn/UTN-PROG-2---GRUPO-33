#include "ArchivoJugadores.h"
#include "EntornoDatosPrueba.h"
#include "ArchivoMejoras.h"
#include "ArchivoEnemigos.h"
#include "ArchivoPartidas.h"
#include "CarpetaDatos.h"
#include "GestorOleadas.h"

#include <cstring>
#include <iostream>
#include <limits>

namespace
{
bool comprobar(bool condicion, const char* caso)
{
    if (!condicion)
    {
        std::cerr << "Fallo: " << caso << '\n';
    }
    return condicion;
}

bool escribirIncompleto(const char* nombre)
{
    asegurarCarpetaDatos();
    FILE* archivo = fopen(obtenerRutaDatos(nombre).c_str(), "wb");
    if (archivo == nullptr)
    {
        return false;
    }
    const bool escrito = fputc(1, archivo) != EOF;
    return fclose(archivo) == 0 && escrito;
}
}

int main(int cantidadArgumentos, char** argumentos)
{
    if (cantidadArgumentos == 2 && std::strcmp(argumentos[1], "--ruta") == 0)
    {
        std::cout << obtenerRutaDatos("Jugadores.dat") << '\n';
        return 0;
    }
    bool correcto = true;
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    ArchivoJugadores perfiles;
    correcto &= comprobar(perfiles.leer().nivelMaximo == 1, "perfil ausente usa valores iniciales");
    correcto &= comprobar(perfiles.guardar({4, 123, 200}), "guardar perfil");
    correcto &= comprobar(perfiles.leer().puntajeMaximo == 123, "leer perfil");
    correcto &= comprobar(!perfiles.guardar({-1, 0, 0}) && perfiles.leer().nivelMaximo == 4,
        "perfil invalido no pisa el existente");
    correcto &= comprobar(escribirIncompleto("Jugadores.dat") && perfiles.leer().nivelMaximo == 1,
        "perfil truncado no carga bytes parciales");
    correcto &= comprobar(perfiles.guardar({4, 123, 200}), "restaurar perfil de prueba");
    ArchivoMejoras mejoras;
    auto catalogo = mejoras.leerCatalogo();
    correcto &= comprobar(catalogo.size() == 2, "catalogo de mejoras inicial");
    catalogo[0].nivelActual = 2;
    catalogo[1].nivelActual = 1;
    correcto &= comprobar(mejoras.guardarCatalogo(catalogo), "guardar mejoras");
    Jugador jugador;
    correcto &= comprobar(jugador.getNivel() == 4 && jugador.getChatarra() == 200 &&
        jugador.getVidaMaxima() == 200 && jugador.getVelocidad() == 6, "perfil y mejoras aplicados al construir jugador");
    jugador.ganarChatarra(5);
    correcto &= comprobar(perfiles.leer().chatarraTotal == 205 && perfiles.leer().puntajeMaximo == 123,
        "guardar progreso conserva record de puntaje");
    std::memset(catalogo[0].nombre, 'X', sizeof(catalogo[0].nombre));
    correcto &= comprobar(!mejoras.guardarCatalogo(catalogo) && mejoras.leerCatalogo()[0].nivelActual == 2,
        "nombre sin terminador rechazado antes de escribir");
    correcto &= comprobar(escribirIncompleto("Mejoras.dat") && mejoras.leerCatalogo().size() == 2,
        "mejoras truncadas recuperan catalogo inicial");

    ArchivoEnemigos enemigos;
    auto variantes = enemigos.leerCatalogo();
    correcto &= comprobar(variantes.size() == 4, "catalogo de cuatro enemigos");
    variantes[0].dano = 77;
    correcto &= comprobar(enemigos.guardarCatalogo(variantes), "guardar balance de enemigos");
    GestorOleadas gestor;
    gestor.actualizar({500, 500}, 1.6f);
    correcto &= comprobar(!gestor.obtenerEnemigos().empty() && gestor.obtenerEnemigos()[0].obtenerDano() == 77,
        "oleadas fabrican enemigos con estadisticas del archivo");
    variantes[0].dano = 88;
    correcto &= comprobar(enemigos.guardarCatalogo(variantes), "actualizar catalogo entre partidas");
    gestor.reiniciar();
    gestor.actualizar({500, 500}, 1.6f);
    correcto &= comprobar(!gestor.obtenerEnemigos().empty() && gestor.obtenerEnemigos()[0].obtenerDano() == 88,
        "nueva partida vuelve a leer el catalogo");
    variantes[0].velocidad = std::numeric_limits<float>::infinity();
    correcto &= comprobar(!enemigos.guardarCatalogo(variantes) && enemigos.leerCatalogo()[0].dano == 88,
        "catalogo no acepta velocidades no finitas");
    correcto &= comprobar(escribirIncompleto("Enemigos.dat") && enemigos.leerCatalogo().size() == 4,
        "catalogo truncado recupera valores de fabrica");

    ArchivoPartidas partidas;
    Partida victoria(1);
    correcto &= comprobar(!partidas.agregar(victoria), "no guardar una partida en curso");
    victoria.registrarNivel(4);
    victoria.registrarChatarra(20);
    victoria.marcarVictoria();
    correcto &= comprobar(partidas.agregar(victoria) && victoria.obtenerIdPartida() == 1 &&
        !partidas.agregar(victoria) && partidas.cantidadRegistros() == 1, "guardado unico con identificador");
    Partida derrota(1);
    derrota.marcarDerrota();
    correcto &= comprobar(partidas.agregar(derrota) && derrota.obtenerIdPartida() == 2 &&
        partidas.leerTodas().size() == 2 && partidas.mejorPuntaje() == victoria.obtenerPuntaje(),
        "historial conserva victorias, derrotas y mejor puntaje");
    correcto &= comprobar(jugador.restablecerProgreso() && perfiles.leer().nivelMaximo == 1 &&
        perfiles.leer().chatarraTotal == 0 && mejoras.leerCatalogo()[0].nivelActual == 0 &&
        partidas.cantidadRegistros() == 2, "reinicio de perfil no borra historial");
    correcto &= comprobar(escribirIncompleto("Partidas.dat") && partidas.cantidadRegistros() == -1,
        "historial truncado se detecta antes de anexar");
    Partida nueva(1);
    nueva.marcarDerrota();
    correcto &= comprobar(!partidas.agregar(nueva) && partidas.vaciar() && partidas.cantidadRegistros() == 0,
        "historial corrupto no recibe registros desalineados y puede vaciarse explicitamente");
    std::cout << (correcto ? "PERSISTENCIA_OK\n" : "PERSISTENCIA_FALLO\n");
    return correcto ? 0 : 1;
}
