#include "AspectoArmas.h"
#include "EntornoDatosPrueba.h"
#include "Arma.h"
#include "Enemigo.h"
#include "Jugador.h"
#include "GestorOleadas.h"
#include "EncuentroAleatorio.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
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
bool comprobar(bool condicion, const char* caso)
{
    if (!condicion)
    {
        std::cerr << "Fallo: " << caso << '\n';
    }
    return condicion;
}

sf::IntRect limitesVisibles(const sf::Image& imagen)
{
    int izquierda = static_cast<int>(imagen.getSize().x);
    int arriba = static_cast<int>(imagen.getSize().y);
    int derecha = -1;
    int abajo = -1;
    for (unsigned int y = 0; y < imagen.getSize().y; ++y)
    {
        for (unsigned int x = 0; x < imagen.getSize().x; ++x)
        {
            if (imagen.getPixel({x, y}).a >= 32)
            {
                izquierda = std::min(izquierda, static_cast<int>(x));
                arriba = std::min(arriba, static_cast<int>(y));
                derecha = std::max(derecha, static_cast<int>(x));
                abajo = std::max(abajo, static_cast<int>(y));
            }
        }
    }
    if (derecha < 0)
    {
        return {};
    }
    return {{izquierda, arriba}, {derecha - izquierda + 1, abajo - arriba + 1}};
}
}

int main()
{
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    bool correcto = true;
    AspectoArmas aspecto;
    AspectoArmas respaldo("recursos/no_existe");
    correcto &= comprobar(aspecto.texturaDisponible(false) && aspecto.texturaDisponible(true),
        "carga ambos PNG desde raiz o bin/Debug");
    correcto &= comprobar(!respaldo.texturaDisponible(false) && !respaldo.texturaDisponible(true),
        "faltantes admiten respaldo");
    sf::RenderTexture superficie({160, 160});
    constexpr float PI = 3.14159265f;
    const std::array<float, 4> angulos{{0.0f, PI / 2.0f, PI, -PI / 2.0f}};
    sf::RenderTexture muestra({640, 560});
    muestra.clear(sf::Color(48, 41, 33));
    sf::Font fuente;
    const bool fuenteDisponible = fuente.openFromFile("recursos/Tuffy.ttf") ||
        fuente.openFromFile("../../recursos/Tuffy.ttf");
    for (int fila = 0; fila < 4; ++fila)
    {
        const int idArma = fila + 1;
        const std::array<int, 4> anchos{{48, 72, 60, 76}};
        const std::array<const char*, 4> nombres{{"PISTOLA 48 px", "METRALLETA 72 px",
            "ESCOPETA 60 px", "CANON ENERGIA 76 px"}};
        correcto &= comprobar(aspecto.texturaDisponible(idArma) && !respaldo.texturaDisponible(idArma),
            "textura por ID y respaldo disponibles");
        if (fuenteDisponible)
        {
            sf::Text texto(fuente, nombres[fila], 18);
            texto.setPosition({12.0f, 8.0f + fila * 140.0f});
            muestra.draw(texto);
        }
        for (std::size_t direccion = 0; direccion < angulos.size(); ++direccion)
        {
            superficie.clear(sf::Color::Transparent);
            aspecto.dibujar(superficie, {80.0f, 80.0f}, angulos[direccion], idArma);
            superficie.display();
            const sf::Image imagen = superficie.getTexture().copyToImage();
            const sf::IntRect limites = limitesVisibles(imagen);
            const int largo = direccion % 2 == 0 ? limites.size.x : limites.size.y;
            const int corto = direccion % 2 == 0 ? limites.size.y : limites.size.x;
            correcto &= comprobar(std::abs(largo - anchos[fila]) <= 2 && corto > 0 && corto < largo,
                "cuatro orientaciones conservan ancho visible y proporcion");
            correcto &= comprobar(imagen.getPixel({0, 0}).a == 0 && imagen.getPixel({159, 159}).a == 0,
                "render conserva transparencia exterior");

            const Posicion centro{80.0f + direccion * 160.0f, 80.0f + fila * 140.0f};
            sf::RectangleShape jugador({48.0f, 48.0f});
            jugador.setOrigin({24.0f, 24.0f});
            jugador.setPosition({centro.x, centro.y});
            jugador.setFillColor(sf::Color(50, 200, 50));
            muestra.draw(jugador);
            aspecto.dibujar(muestra, centro, angulos[direccion], idArma);
        }
        superficie.clear(sf::Color::Transparent);
        respaldo.dibujar(superficie, {80.0f, 80.0f}, PI, idArma);
        superficie.display();
        correcto &= comprobar(limitesVisibles(superficie.getTexture().copyToImage()).size.x == anchos[fila],
            "respaldo visible y orientado");
    }
    superficie.clear(sf::Color::Transparent);
    correcto &= comprobar(!aspecto.texturaDisponible(0) && !aspecto.texturaDisponible(5),
        "ID invalido no accede fuera del contenedor");
    aspecto.dibujar(superficie, {80.0f, 80.0f}, std::numeric_limits<float>::quiet_NaN(), false);
    superficie.display();
    correcto &= comprobar(limitesVisibles(superficie.getTexture().copyToImage()).size.x == 0,
        "angulo no finito no contamina render");
    muestra.display();
    std::filesystem::create_directories(obtenerCarpetaSalidasPrueba());
    correcto &= comprobar(muestra.getTexture().copyToImage().saveToFile(obtenerCarpetaSalidasPrueba() + "/armas_vista.png"),
        "muestra a escala de juego guardada");

    Juego juego;
    juego.ventana.setVisible(false);
    auto& enemigos = juego._gestorOleadas.obtenerEnemigos();
    enemigos.assign(1, crearEnemigo(BASICO, 1100.0f, 512.0f));
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1 && std::abs(juego._angulosArmas[0]) < 0.15f,
        "apunta y dispara al objetivo derecho");
    enemigos[0] = crearEnemigo(BASICO, 932.0f, 700.0f);
    juego.actualizarDisparoAutomatico();
    const FormaColision objetivo = enemigos[0].obtenerFormaColision();
    const Posicion bocaEnMano = juego._aspectoArmas.obtenerPuntaEnMano(
        juego.obtenerAnclajeArma(0), juego._angulosArmas[0], 1, juego._armasReflejadas[0]);
    const float diferenciaX = objetivo.x + objetivo.ancho / 2.0f - bocaEnMano.x;
    const float diferenciaY = objetivo.y + objetivo.alto / 2.0f - bocaEnMano.y;
    const float eje = juego._angulosArmas[0];
    correcto &= comprobar(juego._proyectiles.size() == 1 &&
        std::abs(diferenciaX * std::sin(eje) - diferenciaY * std::cos(eje)) < 0.001f &&
        diferenciaX * std::cos(eje) + diferenciaY * std::sin(eje) > 0.0f,
        "sigue objetivo durante cooldown sin disparo adicional");
    const Posicion vistaSinObjetivo = juego._aspectoJugador.obtenerDireccionVista();
    const std::size_t disparosSinObjetivo = juego._proyectiles.size();
    enemigos.clear();
    juego.actualizarDisparoAutomatico();
    const Posicion vistaActual = juego._aspectoJugador.obtenerDireccionVista();
    correcto &= comprobar(vistaActual.x == vistaSinObjetivo.x && vistaActual.y == vistaSinObjetivo.y &&
        juego._proyectiles.size() == disparosSinObjetivo,
        "sin enemigos conserva la vista y no dispara");
    const Posicion centroSinObjetivo = juego.jugador.getPosicion();
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        const int idArma = juego._inventarioArmas.obtenerIdSlot(slot);
        if (idArma == 0)
        {
            continue;
        }
        const Posicion boca = juego._aspectoArmas.obtenerPuntaEnMano(juego.obtenerAnclajeArma(slot),
            juego._angulosArmas[slot], idArma, juego._armasReflejadas[slot]);
        const float x = centroSinObjetivo.x + vistaSinObjetivo.x * 1000.0f - boca.x;
        const float y = centroSinObjetivo.y + vistaSinObjetivo.y * 1000.0f - boca.y;
        const float distancia = std::hypot(x, y);
        const float angulo = juego._angulosArmas[slot];
        correcto &= comprobar(distancia > 0.0f &&
            std::abs(x * std::sin(angulo) - y * std::cos(angulo)) < distancia * 0.00001f &&
            x * std::cos(angulo) + y * std::sin(angulo) > 0.0f,
            "sin enemigos el arma apunta en la direccion del personaje desde su boca");
    }
    juego._inventarioArmas.aplicarOferta({2, RarezaMejora::Comun, false});
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    juego.dibujar();
    juego.reiniciarPrueba();
    correcto &= comprobar(juego._angulosArmas[0] == 0.0f && juego._angulosArmas[1] == 0.0f &&
        !juego._armasReflejadas[0] && !juego._armasReflejadas[1] &&
        juego.obtenerArmaSeleccionada().obtenerId() == 1,
        "reinicio restaura orientacion y pistola");
    juego.ventana.close();
    std::cout << (correcto ? "Armas: render, orientacion, respaldo y reinicio correctos\n" : "Hay fallos\n");
    return correcto ? 0 : 1;
}
