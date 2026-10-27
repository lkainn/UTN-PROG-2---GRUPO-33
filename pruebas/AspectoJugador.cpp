#include "AspectoJugador.h"
#include "EntornoDatosPrueba.h"
#include "AspectoArmas.h"
#include "Jugador.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
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

bool copiarArchivo(const std::filesystem::path& origen, const std::filesystem::path& destino)
{
    std::ifstream entrada(origen, std::ios::binary);
    std::ofstream salida(destino, std::ios::binary | std::ios::trunc);
    salida << entrada.rdbuf();
    return entrada.good() && salida.good();
}

sf::Image representar(const AspectoJugador& aspecto, bool vivo = true, bool herido = false, bool dedos = false)
{
    sf::RenderTexture superficie({256, 256});
    superficie.clear(sf::Color::Transparent);
    aspecto.dibujar(superficie, {128, 128}, vivo, herido);
    if (dedos)
    {
        aspecto.dibujarAgarre(superficie, {128, 128}, 0, vivo, herido);
        aspecto.dibujarAgarre(superficie, {128, 128}, 1, vivo, herido);
    }
    superficie.display();
    return superficie.getTexture().copyToImage();
}

bool iguales(const sf::Image& a, const sf::Image& b)
{
    for (unsigned int y = 0; y < 256; ++y)
    {
        for (unsigned int x = 0; x < 256; ++x)
        {
            if (a.getPixel({x, y}) != b.getPixel({x, y}))
            {
                return false;
            }
        }
    }
    return true;
}

sf::Image esperado(const sf::Texture& textura, int clip, int cuadro, sf::Vector2f origen, float escala)
{
    sf::RenderTexture superficie({256, 256});
    superficie.clear(sf::Color::Transparent);
    sf::Sprite figura(textura, sf::IntRect({cuadro % 6 * 192, (clip * 4 + cuadro / 6) * 192}, {192, 192}));
    figura.setPosition({128, 128});
    figura.setOrigin(origen);
    figura.setScale({escala, escala});
    superficie.draw(figura);
    superficie.display();
    return superficie.getTexture().copyToImage();
}
}

int main()
{
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    bool correcto = true;
    const std::filesystem::path raiz = std::filesystem::exists("recursos/personajes/remache_atlas/animacion.txt") ? "." : "../..";
    const std::filesystem::path recursos = raiz / "recursos/personajes";
    const std::filesystem::path atlas = recursos / "remache_atlas";
    const std::filesystem::path salida = raiz / "bin/Debug/remache_revision";
    std::filesystem::create_directories(salida);
    std::ifstream datos(atlas / "animacion.txt");
    std::string linea;
    std::getline(datos, linea);
    std::getline(datos, linea);
    sf::Vector2f origen{};
    float escala = 0.0f;
    float paso = 0.0f;
    float proyeccion = 0.0f;
    datos >> origen.x >> origen.y >> escala >> paso >> proyeccion;
    if (!comprobar(datos.good() && paso > 1 && proyeccion > 0, "contrato exportado disponible"))
    {
        return 1;
    }
    AspectoJugador aspecto;
    if (!comprobar(aspecto.animacionDisponible() && aspecto.texturaDisponible(), "atlas completo cargado"))
    {
        return 1;
    }
    AspectoArmas armas;
    aspecto.reiniciar({0, 0});
    const sf::Image inicial = representar(aspecto);
    int arriba = 256;
    int abajo = -1;
    for (unsigned int y = 0; y < 256; ++y)
    {
        for (unsigned int x = 0; x < 256; ++x)
        {
            if (inicial.getPixel({x, y}).a > 0)
            {
                arriba = std::min(arriba, static_cast<int>(y));
                abajo = std::max(abajo, static_cast<int>(y));
            }
        }
    }
    correcto &= comprobar(abajo - arriba + 1 >= 143 && abajo - arriba + 1 <= 145, "alto de 144 px en reposo");
    correcto &= comprobar(inicial.getPixel({0, 0}).a == 0, "transparencia");
    correcto &= comprobar(!iguales(inicial, representar(aspecto, true, true)), "tinte de impacto");
    correcto &= comprobar(!iguales(inicial, representar(aspecto, false)), "tinte de muerte");
    correcto &= comprobar(iguales(representar(aspecto, false), representar(aspecto, false, true)), "prioridad de muerte");
    Jugador jugador;
    correcto &= comprobar(jugador.getFormaColision().ancho == 48 && jugador.getFormaColision().alto == 48, "colision intacta");

    const std::array<Posicion, 8> sentidos{{{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
    sf::RenderTexture muestra({1280, 650});
    muestra.clear(sf::Color(45, 44, 39));
    sf::Texture suelo;
    if (suelo.loadFromFile(raiz / "recursos/entorno/suelo_chatarra_v1.png"))
    {
        suelo.setRepeated(true);
        muestra.draw(sf::Sprite(suelo, sf::IntRect({0, 0}, {1280, 650})));
    }
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        sf::Texture textura;
        correcto &= comprobar(textura.loadFromFile(atlas / ("cuerpo_" + std::to_string(direccion) + ".png")), "vista exportada");
        const Posicion sentido = sentidos[direccion];
        const float norma = std::hypot(sentido.x, sentido.y);
        for (int cuadro : {0, 6, 12, 18, 23})
        {
            const float avance = paso * (cuadro + 0.25f) / 24;
            aspecto.reiniciar({0, 0});
            aspecto.actualizar({sentido.x * avance / norma / 2, sentido.y * avance / norma / 2});
            aspecto.actualizar({sentido.x * avance / norma, sentido.y * avance / norma});
            const sf::Image imagen = representar(aspecto);
            correcto &= comprobar(iguales(imagen, esperado(textura, 0, cuadro, origen, escala)), "vista y fase corresponden al atlas sin espejo");
            correcto &= comprobar(iguales(imagen, representar(aspecto)), "pausa no avanza");
            correcto &= comprobar(iguales(imagen, representar(aspecto, true, false, true)), "dedos coinciden con cuerpo y oclusion");
            const Posicion cercana = aspecto.obtenerMano({128, 128}, 0);
            const Posicion lejana = aspecto.obtenerMano({128, 128}, 1);
            correcto &= comprobar(std::hypot(cercana.x - lejana.x, cercana.y - lejana.y) > 15, "manos distintas");
            for (int idArma = 1; idArma <= 4; ++idArma)
            {
                for (int slot = 0; slot < 2; ++slot)
                {
                    const Posicion mano = aspecto.obtenerMano({128, 128}, slot);
                    const Posicion objetivo{mano.x + sentido.x * 160, mano.y + sentido.y * 160};
                    const bool reflejada = sentido.x < 0;
                    const float angulo = armas.obtenerAnguloDesdeMano(mano, objetivo, idArma, reflejada);
                    const Posicion boca = armas.obtenerPuntaEnMano(mano, angulo, idArma, reflejada);
                    const float x = objetivo.x - boca.x;
                    const float y = objetivo.y - boca.y;
                    correcto &= comprobar(std::abs(x * std::sin(angulo) - y * std::cos(angulo)) < 0.003f &&
                        x * std::cos(angulo) + y * std::sin(angulo) > 0, "eje desde boca de ambas armas");
                }
            }
            if (direccion == 1)
            {
                correcto &= comprobar(imagen.saveToFile(salida / ("ciclo_" + std::to_string(cuadro) + ".png")), "guardar marcha");
            }
        }
        const Posicion centro{160.0f + direccion % 4 * 320, 160.0f + direccion / 4 * 325};
        const float angulo = std::atan2(sentido.y, sentido.x);
        armas.dibujarEnMano(muestra, aspecto.obtenerMano(centro, 1), angulo, 2, sentido.x < 0);
        aspecto.dibujar(muestra, centro, true);
        armas.dibujarEnMano(muestra, aspecto.obtenerMano(centro, 0), angulo, 1, sentido.x < 0);
        aspecto.dibujarAgarre(muestra, centro, 0, true);
        aspecto.dibujarAgarre(muestra, centro, 1, true);
        const float avanceFinal = paso * 23.25f / 24;
        const Posicion parada{sentido.x * avanceFinal / norma, sentido.y * avanceFinal / norma};
        aspecto.actualizar(parada);
        aspecto.actualizar(parada);
        correcto &= comprobar(iguales(representar(aspecto), esperado(textura, 1, 0, origen, escala)), "parada conserva vista y pivote");
    }
    muestra.display();
    correcto &= comprobar(muestra.getTexture().copyToImage().saveToFile(salida / "remache_ocho_vistas_armado.png"), "muestra armada");
    {
        AspectoJugador dividido;
        aspecto.reiniciar({0, 0});
        dividido.reiniciar({0, 0});
        aspecto.actualizar({paso * 0.26f, 0});
        for (int i = 1; i <= 4; ++i)
        {
            dividido.actualizar({paso * 0.26f * i / 4, 0});
        }
        correcto &= comprobar(iguales(representar(aspecto), representar(dividido)), "particiones equivalentes");
        aspecto.actualizar({paso * 0.76f, 0});
        aspecto.actualizar({paso * 1.26f, 0});
        correcto &= comprobar(iguales(representar(aspecto), representar(dividido)), "cierre tras una zancada");
        aspecto.actualizar({paso * 1.24f, 0});
        dividido.reiniciar({0, 0});
        dividido.actualizar({-paso * 0.28f, 0});
        correcto &= comprobar(iguales(representar(aspecto), representar(dividido)), "giro conserva fase");
    }
    aspecto.reiniciar({0, 0});
    aspecto.actualizar({500, 0});
    correcto &= comprobar(iguales(inicial, representar(aspecto)), "teletransporte sin pasos");
    aspecto.actualizar({std::numeric_limits<float>::quiet_NaN(), 0});
    aspecto.actualizar({505, 0});
    correcto &= comprobar(iguales(inicial, representar(aspecto)), "recuperacion tras NaN");
    aspecto.reiniciar({0, 0});
    correcto &= comprobar(iguales(inicial, representar(aspecto)), "reinicio");
    correcto &= comprobar(paso >= 150.0f && 300.0f / paso <= 2.0f,
                           "cadencia normal no supera dos ciclos por segundo");
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        sf::Texture textura;
        correcto &= comprobar(textura.loadFromFile(atlas / ("cuerpo_" + std::to_string(direccion) + ".png")), "vista de apuntado");
        aspecto.reiniciar({0, 0});
        aspecto.orientarHacia({0, 0}, sentidos[direccion]);
        correcto &= comprobar(iguales(representar(aspecto), esperado(textura, 1, 0, origen, escala)),
                               "el rig entero gira hacia el objetivo estando quieto");
        const Posicion mano = aspecto.obtenerMano({128, 128}, 0);
        aspecto.orientarHacia({0, 0}, sentidos[direccion]);
        correcto &= comprobar(aspecto.obtenerMano({128, 128}, 0).x == mano.x &&
                               aspecto.obtenerMano({128, 128}, 0).y == mano.y,
                               "apuntado repetido no desplaza la mano");
    }
    {
        sf::Texture textura;
        correcto &= comprobar(textura.loadFromFile(atlas / "cuerpo_6.png"), "vista para retroceso");
        aspecto.reiniciar({0, 0});
        aspecto.actualizar({paso * 0.26f, 0});
        aspecto.orientarHacia({0, 0}, {-1, 0});
        correcto &= comprobar(iguales(representar(aspecto), esperado(textura, 0, 17, origen, escala)),
                               "retroceder recorre el ciclo al reves sin reiniciarlo");
        const sf::Image retroceso = representar(aspecto);
        aspecto.orientarHacia({0, 0}, {-1, 0});
        correcto &= comprobar(iguales(retroceso, representar(aspecto)), "apuntar dos veces no invierte el paso dos veces");
    }
    sf::RenderTexture invalida({256, 256});
    invalida.clear(sf::Color::Transparent);
    aspecto.dibujar(invalida, {std::numeric_limits<float>::infinity(), 0}, true);
    invalida.display();
    correcto &= comprobar(invalida.getTexture().copyToImage().getPixel({128, 128}).a == 0, "ignorar posicion no finita");
    {
        AspectoJugador ausente("recursos/inexistente");
        correcto &= comprobar(!ausente.animacionDisponible() && !ausente.texturaDisponible() &&
            representar(ausente).getPixel({128, 128}) == sf::Color(50, 200, 50), "respaldo sin recursos");
    }
    const std::filesystem::path parcial = salida / "paquete_invalido";
    std::filesystem::create_directories(parcial / "remache_atlas");
    correcto &= comprobar(copiarArchivo(atlas / "animacion.txt", parcial / "remache_atlas/animacion.txt") &&
        copiarArchivo(recursos / "remache_v1.png", parcial / "remache_v1.png"), "preparar respaldo para paquete invalido");
    const sf::Image diminuta({1, 1}, sf::Color::White);
    correcto &= comprobar(diminuta.saveToFile(parcial / "remache_atlas/cuerpo_0.png") &&
        diminuta.saveToFile(parcial / "remache_atlas/dedos_0.png"), "preparar paquete invalido");
    {
        AspectoJugador incompleto(parcial.string().c_str());
        correcto &= comprobar(!incompleto.animacionDisponible() && incompleto.texturaDisponible(), "dimensiones invalidas activan respaldo completo");
    }
    std::ofstream truncado(parcial / "remache_atlas/animacion.txt", std::ios::trunc);
    truncado << "REMACHE_ATLAS 1\n192 24 6 8 32\n96 97.5 1\n52 0.605\n";
    truncado.close();
    {
        AspectoJugador incompleto(parcial.string().c_str());
        correcto &= comprobar(!incompleto.animacionDisponible() && incompleto.texturaDisponible(), "metadatos truncados activan respaldo");
    }
    std::cout << (correcto ? "REMACHE_ATLAS_OK\n" : "REMACHE_ATLAS_FALLO\n");
    return correcto ? 0 : 1;
}
