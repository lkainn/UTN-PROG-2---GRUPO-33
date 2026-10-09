#include "Arma.h"
#include "AspectoArmas.h"
#include "AspectoProyectiles.h"
#include "Enemigo.h"
#include "Jugador.h"
#include "GestorOleadas.h"
#include "EncuentroAleatorio.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#define private public
#include "Juego.h"
#undef private

namespace
{
constexpr float DISPERSION = 0.6981317f;

bool comprobar(bool condicion, const char* caso)
{
    if (!condicion)
    {
        std::cerr << "Fallo: " << caso << '\n';
    }
    return condicion;
}

Enemigo crearObjetivo(float x, float y, int vida, float tamanio = 56.0f)
{
    return Enemigo(1, x, y, tamanio, tamanio, 0.0f, vida, 10, "Objetivo", 10, 5);
}
}

int main()
{
    bool correcto = true;
    Arma escopeta(3, "Escopeta", 12, 0.8f, 300.0f, 5, 5, DISPERSION);
    std::vector<Proyectil> perdigones;
    correcto &= comprobar(!escopeta.intentarDispararRafaga(perdigones,
        {0.0f, 0.0f}, {301.0f, 0.0f}, 650.0f, 8.0f, 8.0f) &&
        perdigones.empty() && escopeta.puedeDisparar(), "alcance invalido no consume descarga");
    correcto &= comprobar(!escopeta.intentarDispararRafaga(perdigones,
        {0.0f, 0.0f}, {200.0f, 0.0f}, 650.0f, 0.0f, 8.0f) &&
        perdigones.empty() && escopeta.puedeDisparar(), "dimensiones invalidas no consumen descarga");
    correcto &= comprobar(escopeta.intentarDispararRafaga(perdigones,
        {0.0f, 0.0f}, {200.0f, 0.0f}, 650.0f, 8.0f, 8.0f) && perdigones.size() == 5,
        "cinco perdigones por una descarga");
    for (std::size_t indice = 0; indice < perdigones.size(); ++indice)
    {
        const float esperado = -DISPERSION / 2.0f + DISPERSION * indice / 4.0f;
        correcto &= comprobar(std::abs(perdigones[indice].obtenerAngulo() - esperado) < 0.0001f &&
            std::abs(perdigones[indice].obtenerVelocidad() - 650.0f) < 0.001f &&
            perdigones[indice].obtenerDano() == 12 && perdigones[indice].obtenerIdArma() == 3,
            "abanico simetrico con dano y velocidad propios");
        perdigones[indice].actualizar(1.0f);
        const Posicion posicion = perdigones[indice].obtenerPosicion();
        correcto &= comprobar(!perdigones[indice].estaActivo() &&
            std::abs(std::hypot(posicion.x + 4.0f, posicion.y + 4.0f) - 300.0f) < 0.001f,
            "cada perdigon termina a 300 pixeles");
    }
    escopeta.actualizar(1.24f);
    correcto &= comprobar(!escopeta.intentarDispararRafaga(perdigones,
        {0.0f, 0.0f}, {200.0f, 0.0f}, 650.0f, 8.0f, 8.0f) && perdigones.size() == 5,
        "recarga no produce una descarga parcial");
    escopeta.actualizar(0.02f);
    correcto &= comprobar(escopeta.intentarDispararRafaga(perdigones,
        {0.0f, 0.0f}, {200.0f, 0.0f}, 650.0f, 8.0f, 8.0f) && perdigones.size() == 10,
        "siguiente descarga tras 1.25 segundos conserva balas anteriores");

    Proyectil energia({0.0f, 0.0f}, 1000.0f, 0.0f, 40.0f, 10.0f, 60, 4, 3);
    correcto &= comprobar(energia.registrarImpacto(10) && energia.estaActivo() &&
        !energia.registrarImpacto(10), "energia atraviesa primer enemigo sin repetirlo");
    correcto &= comprobar(energia.registrarImpacto(20) && energia.estaActivo() &&
        energia.registrarImpacto(30) && !energia.estaActivo() && !energia.registrarImpacto(40),
        "energia se agota en tercer enemigo distinto");
    Proyectil comun({0.0f, 0.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 10);
    correcto &= comprobar(comun.registrarImpacto(1) && !comun.estaActivo(),
        "proyectiles anteriores conservan un solo impacto");

    Juego juego;
    juego.ventana.setVisible(false);
    auto& enemigos = juego._gestorOleadas.obtenerEnemigos();
    // Alta en inventario sin sincronizar bonos para aislar el combate base.
    juego._inventarioArmas.aplicarOferta({3, RarezaMejora::Comun, false});
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Escopeta);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 3,
        "escopeta adquirida seleccionable");
    for (int indice = 0; indice < 5; ++indice)
    {
        const float angulo = -DISPERSION / 2.0f + DISPERSION * indice / 4.0f;
        const float distancia = indice == 2 ? 190.0f : 200.0f;
        enemigos.push_back(crearObjetivo(960.0f + std::cos(angulo) * distancia - 4.0f,
                                        540.0f + std::sin(angulo) * distancia - 4.0f, 12, 8.0f));
    }
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 5, "Juego genera descarga completa");
    juego.actualizarProyectiles(0.3f);
    bool todosMuertos = true;
    for (const Enemigo& enemigo : enemigos)
    {
        todosMuertos &= !enemigo.estaVivo();
    }
    correcto &= comprobar(todosMuertos && juego._proyectiles.empty() && juego.recursosSuelo.size() == 10,
        "abanico alcanza cinco objetivos y entrega cinco recompensas");

    juego.reiniciarPrueba();
    juego._inventarioArmas.aplicarOferta({4, RarezaMejora::Comun, false}, true);
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::CanonEnergia);
    for (int indice = 0; indice < 4; ++indice)
    {
        enemigos.push_back(crearObjetivo(1060.0f + indice * 100.0f, 512.0f, 60));
    }
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1 &&
        juego._proyectiles.front().obtenerIdArma() == 4, "energia crea un proyectil propio");
    juego.actualizarProyectiles(0.5f);
    correcto &= comprobar(!enemigos[0].estaVivo() && !enemigos[1].estaVivo() &&
        !enemigos[2].estaVivo() && enemigos[3].estaVivo() && juego._proyectiles.empty() &&
        juego.recursosSuelo.size() == 6, "subpasos atraviesan tres objetivos sin saltarlos ni tocar cuarto");
    juego.actualizarProyectiles(0.5f);
    correcto &= comprobar(juego.recursosSuelo.size() == 6, "energia eliminada no duplica recompensas");
    juego._canonEnergiaPrueba.actualizar(1.99f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.empty(), "energia recarga durante dos segundos");
    juego._canonEnergiaPrueba.actualizar(0.02f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1, "energia vuelve a disparar al terminar recarga");

    juego.reiniciarPrueba();
    enemigos.push_back(crearObjetivo(100.0f, 100.0f, 1));
    enemigos.push_back(crearObjetivo(1050.0f, 512.0f, 180));
    juego._proyectiles.emplace_back(Posicion{1050.0f, 520.0f}, 0.0f, 0.0f,
                                   40.0f, 10.0f, 60, 4, 3);
    juego.actualizarProyectiles(0.01f);
    const std::uint64_t idSobreviviente = juego._identificadoresEnemigos[1];
    enemigos[0].recibirDanio(1);
    juego.actualizarEnemigos(0.01f);
    correcto &= comprobar(enemigos.size() == 1 && juego._identificadoresEnemigos[0] == idSobreviviente,
        "compactacion conserva identidad del sobreviviente");
    const std::size_t capacidadAnterior = enemigos.capacity();
    for (std::size_t indice = 0; indice <= capacidadAnterior; ++indice)
    {
        enemigos.push_back(crearObjetivo(1500.0f, 100.0f, 180));
    }
    correcto &= comprobar(enemigos.capacity() > capacidadAnterior, "prueba provoca realocacion de enemigos");
    for (int fotograma = 0; fotograma < 5; ++fotograma)
    {
        juego.actualizarProyectiles(0.01f);
    }
    enemigos[0].recibirDanio(119);
    correcto &= comprobar(enemigos[0].estaVivo(), "sin dano repetido tras compactacion y realocacion");
    enemigos[0].recibirDanio(1);
    correcto &= comprobar(!enemigos[0].estaVivo(), "energia aplico exactamente 60 de dano una vez");

    juego.reiniciarPrueba();
    enemigos.push_back(crearObjetivo(1810.0f, 512.0f, 60));
    Proyectil limitado({940.0f, 535.0f}, 1000.0f, 0.0f, 40.0f, 10.0f, 60, 4, 3);
    limitado.limitarAlcance(800.0f);
    juego._proyectiles.push_back(limitado);
    juego.actualizarProyectiles(2.0f);
    correcto &= comprobar(juego._proyectiles.empty() && enemigos[0].estaVivo() && juego.recursosSuelo.empty(),
        "energia respeta alcance y no impacta tras agotarse");

    juego._inventarioArmas.aplicarOferta({3, RarezaMejora::Comun, false});
    juego._inventarioArmas.aplicarOferta({4, RarezaMejora::Rara, false});
    juego.aplicarBonificacionesArmas();
    correcto &= comprobar(juego._escopetaPrueba.obtenerDano() == 14 &&
        std::abs(juego._canonEnergiaPrueba.obtenerVelocidadAtaque() - 0.56f) < 0.001f &&
        juego.armaPrueba.obtenerDano() == 25, "bonos por rareza afectan solo al arma correspondiente");
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::CanonEnergia);
    juego.dibujar();
    juego.reiniciarPrueba();
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 4 &&
        juego._escopetaPrueba.puedeDisparar() && juego._canonEnergiaPrueba.puedeDisparar() &&
        juego._proyectiles.empty() && juego._identificadoresEnemigos.empty(),
        "reinicio limpia disparos e identidades y restaura recargas");
    juego.jugador.recibirDano(999);
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Escopeta);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.empty(), "jugador muerto no dispara escopeta");

    sf::RenderTexture destino({160, 160});
    AspectoArmas armasSinTexturas("recursos/no_existe");
    AspectoProyectiles proyectilesSinTexturas("recursos/no_existe");
    for (int idArma : {3, 4})
    {
        const sf::Color color = idArma == 3 ? sf::Color(190, 130, 60) : sf::Color(70, 220, 255);
        destino.clear(sf::Color::Transparent);
        correcto &= comprobar(juego._aspectoArmas.texturaDisponible(idArma) &&
            juego._aspectoProyectiles.texturaDisponible(idArma), "arma y proyectil nuevos cargan PNG");
        armasSinTexturas.dibujar(destino, {80.0f, 80.0f}, 0.0f, idArma);
        destino.display();
        correcto &= comprobar(destino.getTexture().copyToImage().getPixel({90, 80}) == color,
            "arma nueva utiliza respaldo propio si falta PNG");
        destino.clear(sf::Color::Transparent);
        Proyectil dibujo({60.0f, 60.0f}, 100.0f, 0.0f, 8.0f, 8.0f, 12, idArma);
        proyectilesSinTexturas.dibujar(destino, dibujo);
        destino.display();
        const sf::Color colorBala = idArma == 3 ? sf::Color(255, 200, 110) : sf::Color(70, 220, 255);
        correcto &= comprobar(destino.getTexture().copyToImage().getPixel({64, 64}) == colorBala,
            "respaldo del proyectil conserva identidad independiente de dimensiones");
    }
    Juego mejorado;
    mejorado.ventana.setVisible(false);
    for (int indice = 0; indice < 3; ++indice)
    {
        correcto &= comprobar(mejorado._inventarioArmas.aplicarOferta({4, RarezaMejora::Legendaria, true}),
            "canon adquiere y acumula tres especiales");
    }
    mejorado.aplicarBonificacionesArmas();
    mejorado.seleccionarSlotArma(1);
    auto& objetivosMejorados = mejorado._gestorOleadas.obtenerEnemigos();
    for (int indice = 0; indice < 7; ++indice)
    {
        objetivosMejorados.push_back(crearObjetivo(1050.0f + indice * 60.0f, 512.0f, 105));
    }
    mejorado.actualizarDisparoAutomatico();
    mejorado.actualizarProyectiles(0.6f);
    for (int indice = 0; indice < 6; ++indice)
    {
        correcto &= comprobar(!objetivosMejorados[indice].estaVivo(), "canon mejorado atraviesa seis enemigos desde Juego");
    }
    correcto &= comprobar(objetivosMejorados[6].estaVivo() && mejorado._proyectiles.empty() &&
        mejorado.recursosSuelo.size() == 12, "septimo enemigo intacto y seis recompensas unicas");
    mejorado.ventana.close();
    juego.ventana.close();
    std::cout << (correcto ? "Armas nuevas: abanico, perforacion, identidades, recargas y render correctos\n" : "Hay fallos\n");
    return correcto ? 0 : 1;
}
