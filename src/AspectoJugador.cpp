#include "AspectoJugador.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <string>
#include <utility>

namespace
{
constexpr float ALTO_PERSONAJE = 144.0f;
constexpr float DISTANCIA_CICLO = 100.0f;
constexpr float PI = 3.14159265f;
constexpr float BALANCEO_HOMBRO = 3.0f;
constexpr float COMPENSACION_CODO = 0.8f;
constexpr std::array<sf::Vector2f, 2> HOMBROS{{{235.0f, 565.0f}, {930.0f, 440.0f}}};
constexpr std::array<sf::Vector2f, 2> CODOS{{{180.0f, 785.0f}, {1030.0f, 660.0f}}};
constexpr std::array<sf::Vector2f, 2> PALMAS{{{205.0f, 990.0f}, {1125.0f, 855.0f}}};

void dibujarPieza(sf::RenderTarget& destino, const sf::Texture& textura,
                  const sf::Transform& transformacion, sf::Color color,
                  std::initializer_list<sf::Vector2f> contorno)
{
    std::array<sf::Vertex, 12> vertices{};
    if (contorno.size() < 3 || contorno.size() > vertices.size())
    {
        return;
    }
    std::size_t indice = 0;
    for (sf::Vector2f punto : contorno)
    {
        vertices[indice].position = punto;
        vertices[indice].texCoords = punto;
        vertices[indice].color = color;
        ++indice;
    }
    sf::RenderStates estados;
    estados.texture = &textura;
    estados.transform = transformacion;
    destino.draw(vertices.data(), indice, sf::PrimitiveType::TriangleFan, estados);
}

sf::Transform girarPieza(sf::Transform padre, sf::Vector2f pivote, float grados)
{
    padre.rotate(sf::degrees(grados), pivote);
    return padre;
}
}

AspectoJugador::AspectoJugador(const char* carpeta)
{
    _usaAtlas = cargarAtlas(carpeta);
    _disponible = _usaAtlas || cargar(carpeta);
}

bool AspectoJugador::cargarAtlas(const char* carpeta)
{
    const std::string ruta = std::string(carpeta) + "/remache_atlas";
    return cargarAtlasDesde(ruta) || cargarAtlasDesde("../../" + ruta);
}

bool AspectoJugador::cargarAtlasDesde(const std::string& carpeta)
{
    std::ifstream archivo(carpeta + "/animacion.txt");
    std::string formato;
    int version = 0;
    int celda = 0;
    int cuadros = 0;
    int columnas = 0;
    int direcciones = 0;
    int celdaDedos = 0;
    sf::Vector2f origen{};
    float escala = 0.0f;
    float largoPaso = 0.0f;
    float proyeccion = 0.0f;
    if (!(archivo >> formato >> version >> celda >> cuadros >> columnas >> direcciones >> celdaDedos
          >> origen.x >> origen.y >> escala >> largoPaso >> proyeccion) ||
        formato != "REMACHE_ATLAS" || version != 1 || celda != 192 || cuadros != 24 ||
        columnas != 6 || direcciones != 8 || celdaDedos != 32 ||
        !std::isfinite(origen.x) || !std::isfinite(origen.y) ||
        origen.x < 0.0f || origen.x > 192.0f || origen.y < 0.0f || origen.y > 192.0f ||
        !std::isfinite(escala) || escala < 0.25f || escala > 4.0f ||
        !std::isfinite(largoPaso) || largoPaso < 1.0f || largoPaso > 1024.0f ||
        !std::isfinite(proyeccion) || proyeccion < 0.1f || proyeccion > 1.0f)
    {
        return false;
    }
    std::array<DireccionesAtlas, 2> poses{};
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        for (int clip = 0; clip < 2; ++clip)
        {
            for (int cuadro = 0; cuadro < 24; ++cuadro)
            {
                int direccionLeida = -1;
                int clipLeido = -1;
                int cuadroLeido = -1;
                if (!(archivo >> direccionLeida >> clipLeido >> cuadroLeido) ||
                    direccionLeida != direccion || clipLeido != clip || cuadroLeido != cuadro)
                {
                    return false;
                }
                for (ManoAtlas& mano : poses[clip][direccion][cuadro])
                {
                    if (!(archivo >> mano.palma.x >> mano.palma.y
                          >> mano.recorteDedos.x >> mano.recorteDedos.y) ||
                        !std::isfinite(mano.palma.x) || !std::isfinite(mano.palma.y) ||
                        mano.palma.x < 0.0f || mano.palma.x > 192.0f ||
                        mano.palma.y < 0.0f || mano.palma.y > 192.0f ||
                        mano.recorteDedos.x < 0 || mano.recorteDedos.x > 191 ||
                        mano.recorteDedos.y < 0 || mano.recorteDedos.y > 191)
                    {
                        return false;
                    }
                }
            }
        }
    }
    std::string sobrante;
    if (archivo >> sobrante)
    {
        return false;
    }
    std::array<sf::Texture, 8> cuerpos;
    std::array<sf::Texture, 8> dedos;
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        const std::string cuerpo = carpeta + "/cuerpo_" + std::to_string(direccion) + ".png";
        const std::string agarre = carpeta + "/dedos_" + std::to_string(direccion) + ".png";
        std::ifstream archivoCuerpo(cuerpo, std::ios::binary);
        std::ifstream archivoDedos(agarre, std::ios::binary);
        if (!archivoCuerpo.good() || !archivoDedos.good() ||
            !cuerpos[direccion].loadFromFile(cuerpo) || !dedos[direccion].loadFromFile(agarre) ||
            cuerpos[direccion].getSize() != sf::Vector2u(1152, 1536) ||
            dedos[direccion].getSize() != sf::Vector2u(384, 256))
        {
            return false;
        }
        cuerpos[direccion].setSmooth(false);
        dedos[direccion].setSmooth(false);
    }
    _cuerposAtlas = std::move(cuerpos);
    _dedosAtlas = std::move(dedos);
    _posesAtlas = poses;
    _origenAtlas = origen;
    _escalaAtlas = escala;
    _largoPasoAtlas = largoPaso;
    return true;
}

void AspectoJugador::actualizarAtlas(Posicion posicion)
{
    _enMovimiento = false;
    _ultimoMovimiento = {};
    _avanceUltimo = 0.0f;
    if (!std::isfinite(posicion.x) || !std::isfinite(posicion.y))
    {
        _tienePosicion = false;
        _distanciaCiclo = 0.0f;
        return;
    }
    if (_tienePosicion)
    {
        const float x = posicion.x - _posicionAnterior.x;
        const float y = posicion.y - _posicionAnterior.y;
        const float distancia = std::hypot(x, y);
        if (std::isfinite(distancia) && distancia >= 0.001f && distancia <= 128.0f)
        {
            float angulo = std::atan2(x, y);
            if (angulo < 0.0f)
            {
                angulo += 2.0f * PI;
            }
            _direccionAtlas = static_cast<int>(std::floor(angulo / (PI / 4.0f) + 0.5f)) % 8;
            _distanciaCiclo = std::fmod(_distanciaCiclo + distancia, _largoPasoAtlas);
            _ultimoMovimiento = {x, y};
            _avanceUltimo = distancia;
            _enMovimiento = true;
        }
        else
        {
            _distanciaCiclo = 0.0f;
        }
    }
    _posicionAnterior = posicion;
    _tienePosicion = true;
}

void AspectoJugador::orientarHacia(Posicion origen, Posicion objetivo)
{
    const float x = objetivo.x - origen.x;
    const float y = objetivo.y - origen.y;
    const float distancia = std::hypot(x, y);
    if (!std::isfinite(distancia) || distancia < 0.001f)
    {
        return;
    }
    if (!_usaAtlas)
    {
        _miraIzquierda = x < 0.0f;
        return;
    }
    float angulo = std::atan2(x, y);
    if (angulo < 0.0f)
    {
        angulo += 2.0f * PI;
    }
    _direccionAtlas = static_cast<int>(std::floor(angulo / (PI / 4.0f) + 0.5f)) % 8;
    if (_ultimoMovimiento.x * x + _ultimoMovimiento.y * y < 0.0f)
    {
        // Al retroceder, recorrer el ciclo al reves sin reiniciar su fase.
        _distanciaCiclo = std::fmod(_distanciaCiclo - 2.0f * _avanceUltimo, _largoPasoAtlas);
        if (_distanciaCiclo < 0.0f)
        {
            _distanciaCiclo += _largoPasoAtlas;
        }
    }
    _avanceUltimo = 0.0f;
}

Posicion AspectoJugador::obtenerDireccionVista() const
{
    if (!_usaAtlas)
    {
        return {_miraIzquierda ? -1.0f : 1.0f, 0.0f};
    }
    const float angulo = _direccionAtlas * PI / 4.0f;
    return {std::sin(angulo), std::cos(angulo)};
}

int AspectoJugador::obtenerCuadroAtlas(bool vivo) const
{
    return vivo && _enMovimiento ? std::min(23, static_cast<int>(_distanciaCiclo / _largoPasoAtlas * 24.0f)) : 0;
}

int AspectoJugador::obtenerClipAtlas(bool vivo) const
{
    return vivo && _enMovimiento ? 0 : 1;
}

void AspectoJugador::dibujarAtlas(sf::RenderTarget& destino, Posicion posicion,
                                  bool vivo, sf::Color color) const
{
    const int cuadro = obtenerCuadroAtlas(vivo);
    const int clip = obtenerClipAtlas(vivo);
    sf::Sprite figura(_cuerposAtlas[_direccionAtlas],
        sf::IntRect({(cuadro % 6) * 192, (clip * 4 + cuadro / 6) * 192}, {192, 192}));
    figura.setOrigin(_origenAtlas);
    figura.setScale({_escalaAtlas, _escalaAtlas});
    figura.setPosition({posicion.x, posicion.y});
    figura.setColor(color);
    destino.draw(figura);
}

bool AspectoJugador::cargar(const char* carpeta)
{
    const std::string ruta = std::string(carpeta) + "/remache_v1.png";
    const std::array<std::string, 2> rutas{{ruta, "../../" + ruta}};
    sf::Image imagen;
    bool cargada = false;
    for (const std::string& candidata : rutas)
    {
        std::ifstream archivo(candidata, std::ios::binary);
        if (archivo.good() && imagen.loadFromFile(candidata))
        {
            cargada = true;
            break;
        }
    }
    if (!cargada)
    {
        return false;
    }
    const sf::Vector2u tamanio = imagen.getSize();
    unsigned int izquierda = tamanio.x;
    unsigned int arriba = tamanio.y;
    unsigned int derecha = 0;
    unsigned int abajo = 0;
    for (unsigned int y = 0; y < tamanio.y; ++y)
    {
        for (unsigned int x = 0; x < tamanio.x; ++x)
        {
            if (imagen.getPixel({x, y}).a >= 32)
            {
                izquierda = std::min(izquierda, x);
                arriba = std::min(arriba, y);
                derecha = std::max(derecha, x);
                abajo = std::max(abajo, y);
            }
        }
    }
    if (izquierda == tamanio.x || !_textura.loadFromImage(imagen))
    {
        return false;
    }
    _recorte = sf::IntRect({static_cast<int>(izquierda), static_cast<int>(arriba)},
        {static_cast<int>(derecha - izquierda + 1), static_cast<int>(abajo - arriba + 1)});
    _textura.setSmooth(false);
    return true;
}

void AspectoJugador::actualizar(Posicion posicion)
{
    if (_usaAtlas)
    {
        actualizarAtlas(posicion);
        return;
    }
    if (!std::isfinite(posicion.x) || !std::isfinite(posicion.y))
    {
        _tienePosicion = false;
        _distanciaCiclo = 0.0f;
        _amplitudMovimiento = 0.0f;
        return;
    }
    if (_tienePosicion)
    {
        const float desplazamientoX = posicion.x - _posicionAnterior.x;
        const float desplazamientoY = posicion.y - _posicionAnterior.y;
        const float distancia = std::hypot(desplazamientoX, desplazamientoY);
        if (!std::isfinite(distancia) || distancia > 128.0f || distancia < 0.001f)
        {
            _distanciaCiclo = 0.0f;
            _amplitudMovimiento = 0.0f;
        }
        else
        {
            if (desplazamientoX != 0.0f)
            {
                _miraIzquierda = desplazamientoX < 0.0f;
            }
            _distanciaCiclo = std::fmod(_distanciaCiclo + distancia, DISTANCIA_CICLO);
            _amplitudMovimiento = std::min(1.0f, _amplitudMovimiento + distancia / 16.0f);
        }
    }
    _posicionAnterior = posicion;
    _tienePosicion = true;
}

void AspectoJugador::reiniciar(Posicion posicion)
{
    _direccionAtlas = 1;
    _enMovimiento = false;
    _miraIzquierda = false;
    _tienePosicion = false;
    _distanciaCiclo = 0.0f;
    _amplitudMovimiento = 0.0f;
    actualizar(posicion);
}

void AspectoJugador::dibujar(sf::RenderTarget& destino, Posicion posicion,
                            bool vivo, bool herido) const
{
    if (!std::isfinite(posicion.x) || !std::isfinite(posicion.y))
    {
        return;
    }
    sf::Color color = _disponible ? sf::Color::White : sf::Color(50, 200, 50);
    if (!vivo)
    {
        color = sf::Color(100, 100, 100);
    }
    else if (herido)
    {
        color = sf::Color(255, 110, 110);
    }
    if (_usaAtlas)
    {
        dibujarAtlas(destino, posicion, vivo, color);
        return;
    }
    // Los contornos pertenecen al dibujo aprobado de 1216 x 1293.
    if (_disponible && _amplitudMovimiento > 0.0f && vivo &&
        _textura.getSize() == sf::Vector2u(1216, 1293))
    {
        dibujarArticulado(destino, posicion, color);
        return;
    }
    sf::Vector2f tamanio{48.0f, 48.0f};
    if (_disponible)
    {
        tamanio = {ALTO_PERSONAJE * _recorte.size.x / _recorte.size.y, ALTO_PERSONAJE};
    }
    sf::RectangleShape figura(tamanio);
    figura.setOrigin({tamanio.x / 2.0f, tamanio.y / 2.0f});
    figura.setPosition({posicion.x, posicion.y});
    if (_disponible)
    {
        figura.setTexture(&_textura);
        figura.setTextureRect(_recorte);
        figura.setScale({_miraIzquierda ? -1.0f : 1.0f, 1.0f});
    }
    figura.setFillColor(color);
    destino.draw(figura);
}

float AspectoJugador::obtenerPaso(bool vivo) const
{
    return vivo ? std::sin(_distanciaCiclo / DISTANCIA_CICLO * 2.0f * PI) * _amplitudMovimiento : 0.0f;
}

sf::Transform AspectoJugador::obtenerTransformacionCuerpo(Posicion posicion, bool vivo) const
{
    const float fase = _distanciaCiclo / DISTANCIA_CICLO * 2.0f * PI;
    const float amplitud = vivo ? _amplitudMovimiento : 0.0f;
    const float escala = ALTO_PERSONAJE / _recorte.size.y;
    sf::Transform raiz;
    raiz.translate({posicion.x, posicion.y});
    raiz.scale({_miraIzquierda ? -escala : escala, escala});
    raiz.translate({-_recorte.position.x - _recorte.size.x / 2.0f,
                    -_recorte.position.y - _recorte.size.y / 2.0f});

    sf::Transform cuerpo = raiz;
    cuerpo.translate({0.0f, -8.0f * (1.0f - std::cos(2.0f * fase)) * amplitud});
    return girarPieza(cuerpo, {640.0f, 750.0f}, obtenerPaso(vivo) * 1.2f);
}

sf::Transform AspectoJugador::obtenerTransformacionBrazo(Posicion posicion, std::size_t slot,
                                                        bool antebrazo, bool vivo) const
{
    const float paso = obtenerPaso(vivo);
    const float signo = slot == 0 ? -1.0f : 1.0f;
    const float giroHombro = signo * paso * BALANCEO_HOMBRO;
    sf::Transform brazo = girarPieza(obtenerTransformacionCuerpo(posicion, vivo),
                                     HOMBROS[slot], giroHombro);
    if (antebrazo)
    {
        // El codo compensa el paso para mantener firme el arma sostenida.
        brazo = girarPieza(brazo, CODOS[slot], -giroHombro * COMPENSACION_CODO);
    }
    return brazo;
}

Posicion AspectoJugador::obtenerMano(Posicion posicion, std::size_t slot, bool vivo) const
{
    if (slot > 1 || !std::isfinite(posicion.x) || !std::isfinite(posicion.y))
    {
        return posicion;
    }
    if (_usaAtlas)
    {
        const Posicion mano = _posesAtlas[obtenerClipAtlas(vivo)][_direccionAtlas][obtenerCuadroAtlas(vivo)][slot].palma;
        return {posicion.x + (mano.x - _origenAtlas.x) * _escalaAtlas,
                posicion.y + (mano.y - _origenAtlas.y) * _escalaAtlas};
    }
    if (!_disponible || _textura.getSize() != sf::Vector2u(1216, 1293))
    {
        const float alto = _disponible ? ALTO_PERSONAJE : 48.0f;
        const float lado = (slot == 0 ? -1.0f : 1.0f) * (_miraIzquierda ? -1.0f : 1.0f);
        return {posicion.x + lado * alto * 0.36f, posicion.y + alto * 0.24f};
    }
    const sf::Vector2f mano = obtenerTransformacionBrazo(posicion, slot, true, vivo)
        .transformPoint(PALMAS[slot]);
    return {mano.x, mano.y};
}

void AspectoJugador::dibujarAgarre(sf::RenderTarget& destino, Posicion posicion,
                                  std::size_t slot, bool vivo, bool herido) const
{
    if (slot > 1 || !_disponible ||
        !std::isfinite(posicion.x) || !std::isfinite(posicion.y))
    {
        return;
    }
    const sf::Color color = !vivo ? sf::Color(100, 100, 100) :
        (herido ? sf::Color(255, 110, 110) : sf::Color::White);
    if (_usaAtlas)
    {
        const int cuadro = obtenerCuadroAtlas(vivo);
        const int clip = obtenerClipAtlas(vivo);
        const sf::Vector2i recorte = _posesAtlas[clip][_direccionAtlas][cuadro][slot].recorteDedos;
        sf::Sprite figura(_dedosAtlas[_direccionAtlas], sf::IntRect(
            {(static_cast<int>(slot) * 6 + cuadro % 6) * 32, (clip * 4 + cuadro / 6) * 32}, {32, 32}));
        figura.setPosition({posicion.x + (recorte.x - _origenAtlas.x) * _escalaAtlas,
                            posicion.y + (recorte.y - _origenAtlas.y) * _escalaAtlas});
        figura.setScale({_escalaAtlas, _escalaAtlas});
        figura.setColor(color);
        destino.draw(figura);
        return;
    }
    if (_textura.getSize() != sf::Vector2u(1216, 1293))
    {
        return;
    }
    const sf::Transform mano = obtenerTransformacionBrazo(posicion, slot, true, vivo);
    if (slot == 0)
    {
        dibujarPieza(destino, _textura, mano, color,
            {{220, 935}, {295, 960}, {305, 1040}, {260, 1080}, {210, 1035}});
    }
    else
    {
        dibujarPieza(destino, _textura, mano, color,
            {{1145, 790}, {1205, 815}, {1205, 945}, {1150, 980}, {1115, 930}});
    }
}

void AspectoJugador::dibujarArticulado(sf::RenderTarget& destino, Posicion posicion,
                                      sf::Color color) const
{
    const float paso = obtenerPaso(true);
    const sf::Transform cuerpo = obtenerTransformacionCuerpo(posicion, true);

    const sf::Transform brazoLejano = obtenerTransformacionBrazo(posicion, 1, false, true);
    const sf::Transform antebrazoLejano = obtenerTransformacionBrazo(posicion, 1, true, true);
    dibujarPieza(destino, _textura, brazoLejano, color,
        {{875, 395}, {1010, 415}, {1110, 590}, {1080, 710}, {945, 705}, {865, 565}});
    dibujarPieza(destino, _textura, antebrazoLejano, color,
        {{965, 625}, {1125, 615}, {1205, 790}, {1210, 990}, {1080, 1020}, {995, 875}});

    const sf::Transform musloLejano = girarPieza(cuerpo, {775.0f, 780.0f}, -paso * 20.0f);
    const sf::Transform canillaLejana = girarPieza(musloLejano, {800.0f, 945.0f},
                                                 std::max(0.0f, -paso) * -30.0f);
    dibujarPieza(destino, _textura, musloLejano, color,
        {{650, 720}, {885, 710}, {950, 825}, {935, 980}, {750, 1020}, {650, 915}});
    dibujarPieza(destino, _textura, canillaLejana, color,
        {{650, 905}, {930, 885}, {1055, 1000}, {1095, 1110}, {1055, 1190}, {760, 1190}, {630, 1080}});

    // Las zonas rigidas se solapan sobre las raices de los miembros.
    dibujarPieza(destino, _textura, cuerpo, color,
        {{40, 0}, {665, 0}, {865, 195}, {730, 445}, {300, 620}, {40, 440}});
    dibujarPieza(destino, _textura, cuerpo, color,
        {{415, 130}, {850, 130}, {1030, 320}, {1035, 405}, {900, 710}, {780, 825}, {510, 825}, {330, 600}, {330, 350}});

    const sf::Transform musloCercano = girarPieza(cuerpo, {555.0f, 810.0f}, paso * 20.0f);
    const sf::Transform canillaCercana = girarPieza(musloCercano, {500.0f, 990.0f},
                                                  std::max(0.0f, paso) * 30.0f);
    dibujarPieza(destino, _textura, musloCercano, color,
        {{400, 740}, {650, 760}, {675, 915}, {555, 1050}, {365, 1005}, {375, 865}});
    dibujarPieza(destino, _textura, canillaCercana, color,
        {{360, 920}, {610, 930}, {650, 1160}, {650, 1293}, {280, 1293}, {275, 1140}, {300, 1020}});

    const sf::Transform brazoCercano = obtenerTransformacionBrazo(posicion, 0, false, true);
    const sf::Transform antebrazoCercano = obtenerTransformacionBrazo(posicion, 0, true, true);
    dibujarPieza(destino, _textura, brazoCercano, color,
        {{145, 530}, {310, 550}, {330, 690}, {285, 840}, {65, 825}, {85, 650}});
    dibujarPieza(destino, _textura, antebrazoCercano, color,
        {{90, 755}, {265, 755}, {320, 915}, {310, 1085}, {110, 1120}, {50, 1000}, {50, 875}});
    dibujarPieza(destino, _textura, cuerpo, color,
        {{160, 255}, {345, 255}, {490, 400}, {430, 570}, {290, 640}, {120, 550}});
}

bool AspectoJugador::texturaDisponible() const
{
    return _disponible;
}

bool AspectoJugador::animacionDisponible() const
{
    return _usaAtlas;
}
