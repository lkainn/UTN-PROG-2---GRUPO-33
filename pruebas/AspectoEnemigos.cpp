#include "EntornoDatosPrueba.h"
#include "Enemigo.h"
#include "SistemaColisiones.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
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

sf::Image representar(const AspectoEnemigos& aspecto, const Enemigo& enemigo)
{
    sf::RenderTexture superficie({256, 256});
    superficie.clear(sf::Color::Transparent);
    aspecto.dibujar(superficie, enemigo);
    superficie.display();
    return superficie.getTexture().copyToImage();
}

bool iguales(const sf::Image& a, const sf::Image& b)
{
    const auto size = a.getSize();
    return b.getSize() == size && std::equal(a.getPixelsPtr(),
        a.getPixelsPtr() + size.x * size.y * 4, b.getPixelsPtr());
}

void descriptor(const std::filesystem::path& carpeta, const std::string& texto)
{
    std::filesystem::create_directories(carpeta);
    std::ofstream salida(carpeta / "animacion.txt");
    salida << texto;
}
}

int main()
{
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    bool correcto = true;
    const std::filesystem::path raiz = std::filesystem::exists("ProyectoTPI.cbp") ? "." : "../..";
    const auto salida = raiz / "bin/Debug/mordaza_revision";
    std::filesystem::create_directories(salida);
    AspectoEnemigos aspecto;
    if (!comprobar(aspecto.animacionDisponible(), "atlas completo disponible"))
    {
        return 1;
    }
    for (const sf::Texture& textura : aspecto._paquetes[0].texturas)
    {
        correcto &= comprobar(!textura.isSmooth(), "sin suavizado de sprites");
    }
    std::vector<Enemigo> enemigos;
    enemigos.push_back(crearEnemigo(BASICO, 100, 100));
    const auto id = enemigos[0].obtenerIdInstancia();
    const FormaColision lejos{1000, 1000, 48, 48};
    aspecto.actualizar(enemigos, lejos, 0.025f);
    const FormaColision forma = enemigos[0].obtenerFormaColision();
    correcto &= comprobar(forma.ancho == 56 && forma.alto == 56, "colision original 56 por 56");
    const sf::Image reposo = representar(aspecto, enemigos[0]);
    enemigos[0].actualizar({1000, 100}, 0.025f);
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.at(id).fase > 0 &&
                          aspecto._estados.at(id).direccion == 2,
                          "avance real y direccion derecha");
    correcto &= comprobar(!iguales(reposo, representar(aspecto, enemigos[0])), "marcha cambia pose");
    const float fase = aspecto._estados.at(id).fase;
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.at(id).fase == fase &&
                          !aspecto._estados.at(id).enMovimiento,
                          "reposo conserva fase y detiene patas");
    const sf::Image pausa = representar(aspecto, enemigos[0]);
    aspecto.actualizar(enemigos, lejos, 0.0f);
    aspecto.actualizar(enemigos, lejos, -1.0f);
    aspecto.actualizar(enemigos, lejos, std::numeric_limits<float>::quiet_NaN());
    correcto &= comprobar(iguales(pausa, representar(aspecto, enemigos[0])), "tiempo invalido no altera pose");
    const std::array<Posicion, 8> movimientos{{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                            {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        const Posicion p = enemigos[0].obtenerPosicion();
        enemigos[0].actualizar({p.x + movimientos[direccion].x * 100,
                               p.y + movimientos[direccion].y * 100}, 0.025f);
        aspecto.actualizar(enemigos, lejos, 0.025f);
        correcto &= comprobar(aspecto._estados.at(id).direccion == direccion,
                              "ocho orientaciones reales al desplazarse");
    }
    correcto &= comprobar(aspecto.obtenerDireccion(std::sin(24*3.14159265f/180),
        std::cos(24*3.14159265f/180), 0) == 0, "histeresis del limite angular");
    enemigos[0].actualizar({2000, 2000}, 10.0f);
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.at(id).fase == 0 &&
                          !aspecto._estados.at(id).enMovimiento, "teletransporte no acelera el ciclo");
    enemigos.clear();
    enemigos.push_back(crearEnemigo(BASICO, 100, 100));
    aspecto.reiniciar();
    const auto contactoId = enemigos[0].obtenerIdInstancia();
    const FormaColision contacto{104, 104, 48, 48};
    aspecto.actualizar(enemigos, contacto, 0.025f);
    const sf::Image pinzaAbierta = representar(aspecto, enemigos[0]);
    aspecto.actualizar(enemigos, contacto, 0.075f);
    correcto &= comprobar(!iguales(pinzaAbierta, representar(aspecto, enemigos[0])),
                          "contacto cierra la pinza");
    correcto &= comprobar(enemigos[0].estaVivo() && enemigos[0].obtenerDano() == 10,
                          "aspecto no altera vida ni dano");
    const auto estadoContacto = aspecto._estados.at(contactoId);
    aspecto.actualizar(enemigos, contacto, 0);
    correcto &= comprobar(aspecto._estados.at(contactoId).tiempoContacto == estadoContacto.tiempoContacto,
                          "pausa congela pinza");
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(!aspecto._estados.at(contactoId).enContacto, "separacion termina pinza");
    for (int i = 0; i < 100; ++i)
    {
        enemigos.push_back(crearEnemigo(BASICO, 100+i, 100));
    }
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.size() == enemigos.size() &&
                          aspecto._estados.count(contactoId) == 1,
                          "realojar vector no pierde identidad visual");
    enemigos[0].recibirDanio(10000);
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.count(contactoId) == 0, "muerte retira estado antes de dibujar");
    enemigos.clear();
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.empty(), "ausentes liberan estado");
    aspecto.reiniciar();
    correcto &= comprobar(aspecto._estados.empty(), "reinicio limpia estados");
    sf::RenderTexture superficie({256, 256});
    correcto &= comprobar(!aspecto.dibujar(superficie,
        Enemigo(99, 100, 100, 56, 56, 0, 100, 10, "Desconocido", 5, 3)),
        "variante desconocida conserva dibujo anterior");
    const auto negativos = salida / ("atlas_invalido_" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    AspectoEnemigos ausente((negativos / "ausente").string().c_str());
    correcto &= comprobar(!ausente.animacionDisponible(), "recurso ausente permite respaldo");
    descriptor(negativos, "MORDAZA_ATLAS 1\n192 4 7 8 16 8\n96 129 0.9\n57.6 0.604767708\n");
    AspectoEnemigos incompleto(negativos.string().c_str());
    correcto &= comprobar(!incompleto.animacionDisponible(), "paquete incompleto no se activa");
    const auto atlas = raiz / "recursos/enemigos/mordaza_atlas";
    for (int i = 0; i < 8; ++i)
    {
        std::filesystem::copy_file(atlas / ("cuerpo_" + std::to_string(i) + ".png"),
            negativos / ("cuerpo_" + std::to_string(i) + ".png"),
            std::filesystem::copy_options::overwrite_existing);
    }
    sf::Image vacia({768, 1344}, sf::Color::Transparent);
    if (!vacia.saveToFile(negativos / "cuerpo_7.png"))
    {
        return 1;
    }
    AspectoEnemigos vacio(negativos.string().c_str());
    correcto &= comprobar(!vacio.animacionDisponible(), "cuadros vacios rechazan todo el paquete");
    descriptor(negativos, "MORDAZA_ATLAS 1\n192 4 7 8 16 8\n96 129 -1\n57.6 0.6\n");
    AspectoEnemigos escalaInvalida(negativos.string().c_str());
    correcto &= comprobar(!escalaInvalida.animacionDisponible(), "escala invalida se rechaza");
    descriptor(negativos, "MORDAZA_ATLAS 999\n");
    AspectoEnemigos versionInvalida(negativos.string().c_str());
    correcto &= comprobar(!versionInvalida.animacionDisponible(), "version desconocida se rechaza");

    Juego juego;
    juego.ventana.setVisible(false);
    juego._estado = Juego::EstadoJuego::Jugando;
    juego._gestorOleadas.obtenerEnemigos().clear();
    const Posicion jugador = juego.jugador.getPosicion();
    auto& horda = juego._gestorOleadas.obtenerEnemigos();
    for (int i = 0; i < 8; ++i)
    {
        const float angulo = i*3.14159265f/4;
        horda.push_back(crearEnemigo(BASICO, jugador.x+std::sin(angulo)*230-28,
                                    jugador.y+std::cos(angulo)*190-28));
    }
    juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), 0.025f);
    juego.relojFotogramas.restart();
    sf::sleep(sf::milliseconds(25));
    juego.actualizar();
    correcto &= comprobar(juego._aspectoEnemigos._estados.size() >= 8,
                          "actualizacion del juego sincroniza atlas");
    juego._estado = Juego::EstadoJuego::Pausa;
    const auto estadoAntesPausa = juego._aspectoEnemigos._estados;
    juego.actualizar();
    bool congelado = juego._aspectoEnemigos._estados.size() == estadoAntesPausa.size();
    for (const auto& entrada : estadoAntesPausa)
    {
        congelado = congelado && juego._aspectoEnemigos._estados.at(entrada.first).fase == entrada.second.fase;
    }
    correcto &= comprobar(congelado, "pausa real del juego congela marcha");
    juego._estado = Juego::EstadoJuego::Jugando;
    juego._estacionMejoras._menuAbierto = true;
    juego.actualizar();
    correcto &= comprobar(juego._aspectoEnemigos._estados.begin()->second.fase ==
        estadoAntesPausa.at(juego._aspectoEnemigos._estados.begin()->first).fase,
        "menu de estacion congela animacion");
    juego._estacionMejoras._menuAbierto = false;
    for (int cuadro = 0; cuadro < 32; ++cuadro)
    {
        juego.actualizarEnemigos(0.02f);
        juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), 0.02f);
        juego.dibujar();
        sf::Texture captura(juego.ventana.getSize());
        captura.update(juego.ventana);
        if (!captura.copyToImage().saveToFile(salida / ("juego_" + std::to_string(cuadro) + ".png")))
        {
            return 1;
        }
    }
    juego.reiniciarPrueba();
    correcto &= comprobar(juego._aspectoEnemigos._estados.empty(), "R limpia animacion integrada");
    std::cout << (correcto ? "ASPECTO_ENEMIGOS_OK\n" : "ASPECTO_ENEMIGOS_FALLO\n");
    return correcto ? 0 : 1;
}
