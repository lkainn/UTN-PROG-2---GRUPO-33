#include "ArchivoJugadores.h"
#include "EntornoDatosPrueba.h"
#include "ArchivoMejoras.h"
#include "ArchivoEnemigos.h"
#include "ArchivoPartidas.h"
#include "CarpetaDatos.h"
#include "GestorOleadas.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <filesystem>
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

bool coincideEstadisticas(Enemigo enemigo, int vida, int dano, float velocidad)
{
    const Posicion origen = enemigo.obtenerPosicion();
    enemigo.actualizar({origen.x + 1000.0f, origen.y}, 1.0f);
    const bool avanceCorrecto =
        std::abs(enemigo.obtenerPosicion().x - origen.x - velocidad) < 0.001f;
    enemigo.recibirDanio(vida - 1);
    const bool quedaUnPunto = enemigo.estaVivo();
    enemigo.recibirDanio(1);
    return avanceCorrecto && enemigo.obtenerDano() == dano && quedaUnPunto && !enemigo.estaVivo();
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
    correcto &= comprobar(catalogo.size() == 3, "catalogo de mejoras inicial");
    catalogo[0].nivelActual = 2;
    catalogo[1].nivelActual = 1;
    catalogo.resize(2);
    correcto &= comprobar(mejoras.guardarCatalogo(catalogo), "guardar catalogo anterior a Rebote");
    catalogo = mejoras.leerCatalogo();
    correcto &= comprobar(catalogo.size() == 3 && catalogo[2].idMejora == 3 &&
        catalogo[2].nivelActual == 0, "catalogo anterior incorpora Rebote");
    Jugador jugador;
    correcto &= comprobar(jugador.getNivel() == 4 && jugador.getChatarra() == 200 &&
        jugador.getVidaMaxima() == 200 && jugador.getVelocidad() == 6, "perfil y mejoras aplicados al construir jugador");
    jugador.ganarChatarra(5);
    correcto &= comprobar(perfiles.leer().chatarraTotal == 205 && perfiles.leer().puntajeMaximo == 123,
        "guardar progreso conserva record de puntaje");
    std::memset(catalogo[0].nombre, 'X', sizeof(catalogo[0].nombre));
    correcto &= comprobar(!mejoras.guardarCatalogo(catalogo) && mejoras.leerCatalogo()[0].nivelActual == 2,
        "nombre sin terminador rechazado antes de escribir");
    jugador.ganarExperiencia(550);
    correcto &= comprobar(jugador.aplicarMejoraElegida(3) && jugador.requiereElegirMejora() &&
        jugador.aplicarMejoraElegida(2) && !jugador.requiereElegirMejora(),
        "dos niveles habilitan dos cartas");
    Jugador recargado;
    correcto &= comprobar(recargado.getRebotes() == 1 && recargado.getVelocidad() == 7 &&
        recargado.getVidaMaxima() == jugador.getVidaMaxima(), "reabrir restaura las mejoras");
    correcto &= comprobar(escribirIncompleto("Mejoras.dat") && mejoras.leerCatalogo().size() == 3,
        "mejoras truncadas recuperan catalogo inicial");

    ArchivoEnemigos enemigos;
    auto variantes = enemigos.leerCatalogo();
    const std::string rutaEnemigos = obtenerRutaDatos(ArchivoEnemigos::RUTA);
    correcto &= comprobar(variantes.size() == 4 &&
        std::filesystem::file_size(rutaEnemigos) == 4 * sizeof(RegistroEnemigo),
        "carga el catalogo de enemigos incluido");
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

    variantes = enemigos.leerCatalogo();
    variantes[BASICO].vida = 120;
    variantes[BASICO].velocidad = 100.0f;
    variantes[RAPIDO].eliminado = 1;
    variantes[RESISTENTE].eliminado = 1;
    const RegistroEnemigo jefe = variantes[JEFE];
    std::swap(variantes[BASICO], variantes[JEFE]);
    correcto &= comprobar(enemigos.guardarCatalogo(variantes) &&
        enemigos.leerCatalogo()[RESISTENTE].eliminado == 1,
        "guarda bajas logicas y ordena los registros por ID");
    gestor.reiniciar();
    gestor.actualizar({500, 500}, 150.0f);
    gestor.actualizar({500, 500}, 150.0f);
    gestor.actualizar({500, 500}, 60.0f);
    const auto& activos = gestor.obtenerEnemigos();
    correcto &= comprobar(activos.size() == 4 && gestor.jefeEstaVivo() &&
        std::all_of(activos.begin(), activos.end(), [](const Enemigo& enemigo)
        {
            return enemigo.obtenerIdEnemigo() == BASICO + 1 || enemigo.obtenerIdEnemigo() == JEFE + 1;
        }), "las bajas logicas no aparecen en las oleadas y el jefe sigue disponible");
    correcto &= comprobar(activos.size() == 4 &&
        coincideEstadisticas(activos[0], 150, 99, 107.5f) &&
        coincideEstadisticas(activos[1], jefe.vida, jefe.dano, jefe.velocidad) &&
        coincideEstadisticas(activos[2], 180, 110, 115.0f) &&
        coincideEstadisticas(activos[3], 180, 110, 115.0f),
        "la dificultad escala vida, dano y velocidad al aparecer, con tope y sin escalar al jefe");
    for (Enemigo& enemigo : gestor.obtenerEnemigos())
    {
        if (enemigo.obtenerIdEnemigo() == JEFE + 1)
        {
            enemigo.recibirDanio(jefe.vida);
        }
    }
    gestor.actualizar({500, 500}, 60.0f);
    correcto &= comprobar(gestor.jefeFueDerrotado() && activos.size() == 4 &&
        coincideEstadisticas(activos.front(), 150, 99, 107.5f) &&
        coincideEstadisticas(activos.back(), 240, 136, 125.0f),
        "despues del jefe los nuevos enemigos ganan vida, dano y velocidad adicionales");
    gestor.actualizar({500, 500}, 660.0f);
    correcto &= comprobar(activos.size() == 5 &&
        coincideEstadisticas(activos.back(), 570, 282, 175.0f),
        "el modo infinito limita la velocidad y sigue aumentando la vida y el dano");
    gestor.reiniciar();
    gestor.actualizar({500, 500}, 1.6f);
    correcto &= comprobar(gestor.obtenerEnemigos().size() == 1 &&
        coincideEstadisticas(gestor.obtenerEnemigos()[0], 120, 88, 100.08f),
        "reiniciar vuelve al principio de la curva usando los valores base del archivo");
    variantes[0].eliminado = 1;
    correcto &= comprobar(!enemigos.guardarCatalogo(variantes), "el jefe no admite baja logica");
    variantes[0].eliminado = 0;
    variantes[3].eliminado = 1;
    correcto &= comprobar(!enemigos.guardarCatalogo(variantes), "el basico no admite baja logica");

    FILE* catalogoAnterior = fopen(rutaEnemigos.c_str(), "wb");
    bool formatoAnteriorEscrito = catalogoAnterior != nullptr;
    if (catalogoAnterior != nullptr)
    {
        for (const RegistroEnemigo& variante : variantes)
        {
            formatoAnteriorEscrito &= fwrite(&variante, offsetof(RegistroEnemigo, eliminado),
                1, catalogoAnterior) == 1;
        }
        formatoAnteriorEscrito &= fclose(catalogoAnterior) == 0;
    }
    const auto compatibilidad = enemigos.leerCatalogo();
    correcto &= comprobar(formatoAnteriorEscrito && compatibilidad[BASICO].dano == 88 &&
        compatibilidad[RAPIDO].eliminado == 0 && compatibilidad[RESISTENTE].eliminado == 0 &&
        std::filesystem::file_size(rutaEnemigos) == 4 * offsetof(RegistroEnemigo, eliminado),
        "lee el formato anterior con sus valores y sin reescribirlo");
    correcto &= comprobar(escribirIncompleto("Enemigos.dat"), "prepara catalogo truncado");
    correcto &= comprobar(lecturaRechazada(enemigos, "Enemigos.dat", "danado") &&
        std::filesystem::file_size(rutaEnemigos) == 1,
        "rechaza el catalogo truncado y conserva el archivo");
    correcto &= comprobar(std::filesystem::remove(rutaEnemigos) &&
        lecturaRechazada(enemigos, "Enemigos.dat", "Falta") && !std::filesystem::exists(rutaEnemigos),
        "el catalogo es obligatorio y el lector no lo recrea");

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
