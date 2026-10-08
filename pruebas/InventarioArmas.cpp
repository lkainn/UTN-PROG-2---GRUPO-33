#include "InventarioArmas.h"
#include "Arma.h"
#include <array>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#define private public
#include "EstacionMejoras.h"
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

bool iguales(const OfertaArma& primera, const OfertaArma& segunda)
{
    return primera.idArma == segunda.idArma && primera.rareza == segunda.rareza &&
        primera.especial == segunda.especial;
}
}

int main()
{
    bool correcto = true;
    InventarioArmas inventario;
    correcto &= comprobar(inventario.obtenerIdSlot(0) == 1 && inventario.obtenerIdSlot(1) == 0 &&
        inventario.obtenerIdSlot(2) == 0 && !inventario.poseeArma(0), "inicio e indices invalidos");
    correcto &= comprobar(!inventario.aplicarOferta({0, RarezaMejora::Comun, false}) &&
        !inventario.aplicarOferta({2, static_cast<RarezaMejora>(99), false}) &&
        !inventario.aplicarOferta({2, RarezaMejora::Comun, true}) &&
        !inventario.aplicarOferta({2, RarezaMejora::Legendaria, false}), "ofertas invalidas no mutan inventario");
    correcto &= comprobar(inventario.aplicarOferta({1, RarezaMejora::Comun, false}) &&
        inventario.obtenerBonificaciones(1).porcentajeDano == 5, "mejora pistola sin ocupar slot");
    correcto &= comprobar(inventario.aplicarOferta({2, RarezaMejora::Legendaria, true}) &&
        inventario.obtenerIdSlot(1) == 2 && inventario.obtenerBonificaciones(2).porcentajeDano == 25 &&
        inventario.obtenerBonificaciones(2).porcentajeCadencia == 18 &&
        inventario.obtenerBonificaciones(2).especiales == 1, "adquisicion incluye beneficios legendarios");
    correcto &= comprobar(inventario.aplicarOferta({2, RarezaMejora::Rara, true}) &&
        inventario.obtenerBonificaciones(2).porcentajeDano == 35 &&
        inventario.obtenerBonificaciones(2).porcentajeCadencia == 18, "especial raro sustituye cadencia y suma sobre base");
    correcto &= comprobar(!inventario.aplicarOferta({3, RarezaMejora::Epica, true}) &&
        inventario.obtenerIdSlot(0) == 1 && inventario.obtenerIdSlot(1) == 2 &&
        inventario.obtenerBonificaciones(3).especiales == 0, "reemplazo requiere confirmar y no aplica parcialmente");
    correcto &= comprobar(inventario.aplicarOferta({3, RarezaMejora::Epica, true}, true) &&
        inventario.obtenerIdSlot(0) == 3 && inventario.obtenerIdSlot(1) == 2 &&
        inventario.obtenerBonificaciones(1).porcentajeDano == 0 && !inventario.poseeArma(1),
        "solo reemplaza pistola y descarta sus bonos");
    correcto &= comprobar(!inventario.puedeRecibirArma(1) && !inventario.puedeRecibirArma(4) &&
        !inventario.aplicarOferta({4, RarezaMejora::Comun, false}, true) &&
        inventario.aplicarOferta({3, RarezaMejora::Comun, false}), "slots fijos solo reciben mejoras propias");
    for (int indice = 0; indice < 3; ++indice)
    {
        correcto &= comprobar(inventario.aplicarOferta({3, RarezaMejora::Legendaria, true}), "especial escopeta hasta limite");
    }
    correcto &= comprobar(!inventario.puedeMejorarEspecial(3) &&
        !inventario.aplicarOferta({3, RarezaMejora::Legendaria, true}), "escopeta limita cuatro extras");

    InventarioArmas energia;
    for (int indice = 0; indice < 3; ++indice)
    {
        correcto &= comprobar(energia.aplicarOferta({4, RarezaMejora::Legendaria, true}), "energia acepta hasta tres extras");
    }
    correcto &= comprobar(!energia.puedeMejorarEspecial(4) &&
        !energia.aplicarOferta({4, RarezaMejora::Legendaria, true}), "energia limita a seis impactos");

    EstacionMejoras estacion;
    InventarioArmas inicial;
    std::mt19937 generador(314);
    estacion.abrirMenu(inicial, generador);
    const auto ofertas = estacion._ofertas;
    const auto estadoGenerador = generador;
    estacion.abrirMenu(inicial, generador);
    correcto &= comprobar(generador == estadoGenerador, "abrir menu abierto no consume sorteo");
    estacion.cancelarCompra();
    estacion.abrirMenu(inicial, generador);
    for (std::size_t indice = 0; indice < 3; ++indice)
    {
        correcto &= comprobar(iguales(estacion.obtenerOferta(indice), ofertas[indice]), "cancelacion conserva ofertas");
    }
    correcto &= comprobar(generador == estadoGenerador && !estacion.obtenerOferta(3).esValida() &&
        estacion.describirOpcion(3).empty(), "reabrir no resortea e indice invalido es seguro");
    estacion.cancelarCompra();
    estacion.abrirMenu(inventario, generador);
    for (std::size_t indice = 0; indice < 3; ++indice)
    {
        correcto &= comprobar(inventario.puedeAplicarOferta(estacion.obtenerOferta(indice)), "cache descarta ofertas que inventario ya no acepta");
    }
    estacion.cancelarCompra();
    estacion.establecerPosicion({-1.0f, -1.0f});
    estacion.abrirMenu(inicial, generador);
    correcto &= comprobar(estacion._ofertasGuardadas.count({-1, -1}) == 1, "clave de zona negativa usa floor");
    estacion._costo = std::numeric_limits<int>::max();
    correcto &= comprobar(estacion.confirmarEleccion(0, inicial) &&
        estacion.obtenerCosto() == std::numeric_limits<int>::max(), "precio no desborda");

    std::array<int, 4> frecuencias{};
    std::array<int, 4> especiales{};
    for (int indice = 0; indice < 100000; ++indice)
    {
        const OfertaArma oferta = estacion.generarOferta(1, inicial, generador);
        ++frecuencias[static_cast<std::size_t>(oferta.rareza)];
        especiales[static_cast<std::size_t>(oferta.rareza)] += oferta.especial ? 1 : 0;
        correcto &= comprobar(oferta.esValida(), "sorteo solo produce ofertas validas");
        const OfertaArma agotada = estacion.generarOferta(4, energia, generador);
        correcto &= comprobar(agotada.rareza != RarezaMejora::Legendaria && !agotada.especial,
            "especial agotado excluye legendarias y especiales vacios");
    }
    const std::array<float, 4> esperadas{{0.60f, 0.25f, 0.12f, 0.03f}};
    for (std::size_t indice = 0; indice < 4; ++indice)
    {
        correcto &= comprobar(std::abs(frecuencias[indice] / 100000.0f - esperadas[indice]) < 0.01f,
            "frecuencias de rareza 60/25/12/3");
    }
    correcto &= comprobar(especiales[0] == 0 && especiales[3] == frecuencias[3] &&
        std::abs(static_cast<float>(especiales[1]) / frecuencias[1] - 0.15f) < 0.02f &&
        std::abs(static_cast<float>(especiales[2]) / frecuencias[2] - 0.50f) < 0.02f,
        "especial comun nunca, raro15, epico50, legendario siempre");

    Arma canon(4, "Energia", 60, 0.5f, 800.0f, 5, 1, 0.0f, 3);
    Proyectil anterior({0.0f, 0.0f}, 0.0f, 0.0f, 0.0f, 0.0f, 0);
    canon.intentarDisparar(anterior, {0.0f, 0.0f}, {100.0f, 0.0f}, 1000.0f, 40.0f, 10.0f);
    canon.configurarMejoras(25, 18, 3);
    canon.reiniciar();
    Proyectil mejorado({0.0f, 0.0f}, 0.0f, 0.0f, 0.0f, 0.0f, 0);
    correcto &= comprobar(canon.intentarDisparar(mejorado, {0.0f, 0.0f}, {100.0f, 0.0f}, 1000.0f, 40.0f, 10.0f) &&
        mejorado.obtenerDano() == 75 && anterior.obtenerDano() == 60 &&
        std::abs(canon.obtenerVelocidadAtaque() - 0.59f) < 0.001f && canon.obtenerCantidadProyectiles() == 1,
        "porcentajes sobre base y disparos anteriores inalterados");
    for (int indice = 1; indice <= 6; ++indice)
    {
        correcto &= comprobar(mejorado.registrarImpacto(indice) && !mejorado.registrarImpacto(indice), "seis impactos distintos sin repetir");
        if (indice <= 3)
        {
            correcto &= comprobar(anterior.registrarImpacto(indice), "disparo anterior conserva tres impactos");
        }
    }
    correcto &= comprobar(!mejorado.estaActivo() && !anterior.estaActivo(), "se agotan los limites propios de cada disparo");
    Arma pistola(1, "Pistola", 25, 2.0f, 600.0f, 5);
    pistola.configurarMejoras(5, 4, 999);
    std::vector<Proyectil> disparos;
    correcto &= comprobar(pistola.intentarDispararRafaga(disparos, {0.0f, 0.0f}, {100.0f, 0.0f}, 800.0f, 28.0f, 14.0f) &&
        disparos.size() == 5 && disparos.front().obtenerAngulo() < 0.0f && disparos.back().obtenerAngulo() > 0.0f,
        "proyectiles extra limitados y abanico de diez grados");
    Arma escopeta(3, "Escopeta", 12, 0.8f, 300.0f, 5, 5, 0.6981317f);
    escopeta.configurarMejoras(0, 0, 999);
    correcto &= comprobar(escopeta.obtenerCantidadProyectiles() == 9, "escopeta no supera nueve perdigones");
    std::cout << (correcto ? "Inventario y rarezas: pruebas correctas\n" : "Hay fallos\n");
    return correcto ? 0 : 1;
}
