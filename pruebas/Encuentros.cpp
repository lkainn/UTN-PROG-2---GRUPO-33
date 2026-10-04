#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <iostream>
#include <cmath>
#include <limits>
#include "Arma.h"
#include "Jugador.h"
#include "Enemigo.h"
#include "EncuentroAleatorio.h"
#define private public
#include "Juego.h"
#undef private

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

ObjetoEncuentro* siguiente(Juego& juego, TipoEncuentro tipo)
{
    for (const auto& entrada : juego._encuentros.obtenerZonas())
    {
        for (const ObjetoEncuentro& objeto : entrada.second)
        {
            if (objeto.presente && !objeto.usado && objeto.tipo == tipo)
            {
                juego.jugador.setPosicion(objeto.posicion);
                return juego._encuentros.buscarCercano(objeto.posicion, tipo);
            }
        }
    }
    return nullptr;
}
}

int main()
{
    bool correcto = true;
    EncuentroAleatorio mapa(42);
    EncuentroAleatorio otro(42);
    mapa.generarAlrededor({0.0f, 0.0f});
    mapa.generarAlrededor({-8000.0f, 6000.0f});
    otro.generarAlrededor({-8000.0f, 6000.0f});
    otro.generarAlrededor({0.0f, 0.0f});
    correcto &= comprobar(mapa.obtenerZonas().size() == otro.obtenerZonas().size(), "zonas unicas");
    for (const auto& entrada : mapa.obtenerZonas())
    {
        const auto& zona = entrada.second;
        const auto& replica = otro.obtenerZonas().at(entrada.first);
        for (std::size_t i = 0; i < zona.size(); ++i)
        {
            correcto &= comprobar(zona[i].posicion.x == replica[i].posicion.x &&
                zona[i].posicion.y == replica[i].posicion.y && zona[i].presente == replica[i].presente, "independencia de orden exploracion");
            for (std::size_t j = i + 1; j < zona.size(); ++j)
            {
                correcto &= comprobar(std::hypot(zona[i].posicion.x - zona[j].posicion.x,
                    zona[i].posicion.y - zona[j].posicion.y) >= 200.0f, "separacion objetos");
            }
        }
    }
    mapa.generarAlrededor({-1.0f, -1.0f});
    correcto &= comprobar(mapa.obtenerZonas().count({-3, -3}) == 1, "floor en posiciones negativas");
    Posicion posicion{};
    bool hayCofre = false;
    for (const auto& entrada : mapa.obtenerZonas())
    {
        for (const ObjetoEncuentro& objeto : entrada.second)
        {
            if (objeto.tipo == TipoEncuentro::Cofre && objeto.presente)
            {
                posicion = objeto.posicion;
                hayCofre = true;
            }
            else if (!objeto.presente)
            {
                correcto &= comprobar(mapa.buscarCercano(objeto.posicion, TipoEncuentro::Cofre) == nullptr,
                    "slot ausente no permite interactuar");
            }
        }
    }
    correcto &= comprobar(hayCofre, "semilla de prueba tiene cofres");
    ObjetoEncuentro* original = mapa.buscarCercano(posicion, TipoEncuentro::Cofre);
    correcto &= comprobar(original != nullptr &&
        mapa.buscarCercano({posicion.x + 101.0f, posicion.y}, TipoEncuentro::Cofre) == nullptr,
        "radio de interaccion");
    original->usado = true;
    mapa.generarAlrededor({20000.0f, -20000.0f});
    mapa.generarAlrededor({0.0f, 0.0f});
    correcto &= comprobar(original->usado && original->posicion.x == posicion.x &&
        mapa.buscarCercano(posicion, TipoEncuentro::Cofre) == nullptr, "estado y puntero persistentes");
    const auto cantidad = mapa.obtenerZonas().size();
    mapa.generarAlrededor({std::numeric_limits<float>::quiet_NaN(), 0.0f});
    correcto &= comprobar(mapa.obtenerZonas().size() == cantidad, "ignora coordenadas invalidas");

    EncuentroAleatorio muestra(123);
    for (int i = 0; i < 100; ++i)
    {
        muestra.generarAlrededor({i * 6000.0f, 0.0f});
    }
    int presentes = 0;
    int posibles = 0;
    int estaciones = 0;
    for (const auto& entrada : muestra.obtenerZonas())
    {
        estaciones += entrada.second[0].presente ? 1 : 0;
        if (!entrada.second[0].presente)
        {
            correcto &= comprobar(muestra.buscarCercano(entrada.second[0].posicion,
                TipoEncuentro::Estacion) == nullptr, "estacion ausente no permite interactuar");
        }
        for (std::size_t i = 1; i < 3; ++i)
        {
            ++posibles;
            presentes += entrada.second[i].presente ? 1 : 0;
        }
    }
    const float frecuencia = static_cast<float>(presentes) / posibles;
    correcto &= comprobar(frecuencia > 0.22f && frecuencia < 0.28f,
        "cofres aparecen aproximadamente 25% por ubicacion");
    const float frecuenciaEstacion = static_cast<float>(estaciones) / muestra.obtenerZonas().size();
    correcto &= comprobar(frecuenciaEstacion > 0.12f && frecuenciaEstacion < 0.18f,
        "estaciones aparecen aproximadamente 15% por zona");

    Juego juego;
    juego.ventana.setVisible(false);
    juego._generador.seed(314);
    juego._encuentros = EncuentroAleatorio(42);
    juego._encuentros.generarAlrededor(juego.jugador.getPosicion());
    correcto &= comprobar(juego._encuentros.obtenerZonas().size() == 25, "zonas generadas al iniciar");
    const ObjetoEncuentro& visible = *siguiente(juego, TipoEncuentro::Estacion);
    juego.jugador.setPosicion({visible.posicion.x + 80.0f, visible.posicion.y});
    juego._escenario.actualizar(juego.jugador.getPosicion());
    juego._estacionMejoras.establecerPosicion({visible.posicion.x + 700.0f, visible.posicion.y});
    juego.ventana.setView(juego._escenario.obtenerVista());
    juego.ventana.clear(sf::Color::Magenta);
    juego.dibujarEstacionMejoras(visible);
    sf::Texture captura(juego.ventana.getSize());
    captura.update(juego.ventana);
    const sf::Vector2i pixel = juego.ventana.mapCoordsToPixel(
        {visible.posicion.x, visible.posicion.y - 15.0f});
    const sf::Image imagen = captura.copyToImage();
correcto &= comprobar(imagen.getPixel({static_cast<unsigned int>(pixel.x),
        static_cast<unsigned int>(pixel.y)}) == sf::Color(30, 125, 100),
        "pantalla de estacion se dibuja en su posicion real");
    ObjetoEncuentro* cofre = siguiente(juego, TipoEncuentro::Cofre);
    juego.intentarAbrirCofre();
    correcto &= comprobar(!cofre->usado && juego._costoCofre == 15, "sin saldo no consume ni aumenta precio");
    juego.jugador.ganarChatarra(1000);
    const Posicion lugar = cofre->posicion;
    juego.jugador.setPosicion({lugar.x + 101.0f, lugar.y});
    juego.intentarAbrirCofre();
    correcto &= comprobar(juego.jugador.getChatarra() == 1000, "lejos no cobra");
    juego.jugador.setPosicion(lugar);
    juego.intentarAbrirCofre();
    juego.intentarAbrirCofre();
    correcto &= comprobar(cofre->usado && juego._costoCofre == 24 &&
        juego.jugador.getChatarra() == 985 && !juego._estacionMejoras.menuAbierto(), "cofre cobra una vez y aumenta 60%");
    siguiente(juego, TipoEncuentro::Cofre);
    juego.intentarAbrirCofre();
    correcto &= comprobar(juego._costoCofre == 39 && juego.jugador.getChatarra() == 961,
        "segundo cofre usa precio global independiente");
    siguiente(juego, TipoEncuentro::Cofre);
    juego.intentarAbrirCofre();
    correcto &= comprobar(juego._costoCofre == 63 && juego.jugador.getChatarra() == 922,
        "redondeo hacia arriba 39 a 63");
    ObjetoEncuentro* estacion = siguiente(juego, TipoEncuentro::Estacion);
    juego.intentarComprarMejora();
    juego.elegirMejoraEstacion(3);
    correcto &= comprobar(!estacion->usado && juego._estacionMejoras.menuAbierto(), "opcion invalida no consume");
    juego.elegirMejoraEstacion(0);
    juego.intentarComprarMejora();
    correcto &= comprobar(estacion->usado && !juego._estacionMejoras.menuAbierto() &&
        juego._estacionMejoras.obtenerCosto() == 16 && juego._costoCofre == 63,
        "una compra estacion y precios independientes");
    ObjetoEncuentro* pendiente = siguiente(juego, TipoEncuentro::Estacion);
    const int saldo = juego.jugador.getChatarra();
    juego.intentarComprarMejora();
    juego.actualizar();
    juego.intentarAbrirCofre();
    correcto &= comprobar(!pendiente->usado && juego.jugador.getChatarra() == saldo - 16,
        "menu cobra una vez bloquea cofre");
    juego.reiniciarPrueba();
    correcto &= comprobar(!pendiente->usado && estacion->usado && cofre->usado &&
        juego.jugador.getChatarra() == saldo && juego._costoCofre == 63,
        "R reembolsa pendiente conserva mapa y precio");
    juego.jugador.setPosicion(lugar);
    juego.actualizarEncuentros(10000.0f);
    correcto &= comprobar(cofre->usado && cofre->posicion.x == lugar.x, "ningun temporizador regenera objetos");
    juego.dibujar();

    juego._metralletaAdquirida = true;
    juego._armaSeleccionada = Juego::TipoArmaPrueba::Pistola;
    juego._danoCofres = {4, 0};
    juego._cadenciaCofres = {4, 0};
    ObjetoEncuentro* agotado = siguiente(juego, TipoEncuentro::Cofre);
    juego.intentarAbrirCofre();
    correcto &= comprobar(!agotado->usado && juego._costoCofre == 63 &&
        juego.jugador.getChatarra() == saldo, "sin recompensas no cobra ni consume");
    juego._danoCofres[0] = 3;
    juego.jugador.recibirDano(9999);
    juego.intentarAbrirCofre();
    correcto &= comprobar(!agotado->usado, "muerto no abre");
    juego.jugador.reiniciar();
    juego.jugador.setPosicion(agotado->posicion);
    juego._costoCofre = std::numeric_limits<int>::max();
    juego.jugador.ganarChatarra(std::numeric_limits<int>::max() - juego.jugador.getChatarra());
    juego.intentarAbrirCofre();
    correcto &= comprobar(agotado->usado && juego._costoCofre == std::numeric_limits<int>::max() &&
        juego.jugador.getChatarra() == 0 && juego._danoCofres[0] == 4, "precio grande satura y recompensa elegible");
    juego.ventana.close();
    if (correcto)
    {
        std::cout << "Zonas: generacion, persistencia, uso unico, precios y reembolso correctos\n";
    }
    return correcto ? 0 : 1;
}
