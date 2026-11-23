#include "AspectoEnemigos.h"

#include "SistemaColisiones.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <utility>

namespace
{
constexpr int COLUMNAS = 4;
constexpr int FILAS = 7;
constexpr int CUADROS_MARCHA = 16;
constexpr int INICIO_REPOSO = 16;
constexpr int INICIO_CONTACTO = 20;
constexpr int CUADROS_CONTACTO = 8;
constexpr float PI = 3.14159265359f;
constexpr float DURACION_CONTACTO = 0.30f;
constexpr float INTERVALO_CONTACTO = 0.75f;
constexpr float BASE_VISUAL = 80.0f;

bool formaValida(FormaColision forma)
{
    return std::isfinite(forma.x) && std::isfinite(forma.y) &&
           std::isfinite(forma.ancho) && std::isfinite(forma.alto) &&
           forma.ancho > 0.0f && forma.alto > 0.0f;
}
}

AspectoEnemigos::AspectoEnemigos(const char* carpeta, const char* carpetaZancudo,
                                const char* carpetaYunque, const char* carpetaDesguazador)
{
    const std::string ruta(carpeta);
    const std::string rutaZancudo(carpetaZancudo);
    _paquetes[0].disponible = cargarDesde(0, ruta, "MORDAZA_ATLAS") ||
        cargarDesde(0, "../../" + ruta, "MORDAZA_ATLAS");
    _paquetes[1].disponible = cargarDesde(1, rutaZancudo, "ZANCUDO_ATLAS") ||
        cargarDesde(1, "../../" + rutaZancudo, "ZANCUDO_ATLAS");
    const std::string rutaYunque(carpetaYunque);
    _paquetes[2].disponible = cargarDesde(2, rutaYunque, "YUNQUE_ATLAS") ||
        cargarDesde(2, "../../" + rutaYunque, "YUNQUE_ATLAS");
    const std::string rutaDesguazador(carpetaDesguazador);
    _paquetes[3].disponible = cargarDesde(3, rutaDesguazador, "DESGUAZADOR_ATLAS") ||
        cargarDesde(3, "../../" + rutaDesguazador, "DESGUAZADOR_ATLAS");
}

bool AspectoEnemigos::cargarDesde(int indice, const std::string& carpeta,
                                 const char* formatoEsperado)
{
    const int celdaEsperada = indice == 3 ? 256 : 192;
    std::ifstream archivo(carpeta + "/animacion.txt");
    std::string formato;
    int version = 0;
    int celda = 0;
    int columnas = 0;
    int filas = 0;
    int direcciones = 0;
    int marcha = 0;
    int contacto = 0;
    sf::Vector2f origen{};
    float escala = 0.0f;
    float zancada = 0.0f;
    float proyeccion = 0.0f;
    if (!(archivo >> formato >> version >> celda >> columnas >> filas >> direcciones
          >> marcha >> contacto >> origen.x >> origen.y >> escala >> zancada >> proyeccion) ||
        formato != formatoEsperado || version != 1 || celda != celdaEsperada ||
        columnas != COLUMNAS || filas != FILAS || direcciones != 8 ||
        marcha != CUADROS_MARCHA || contacto != CUADROS_CONTACTO ||
        !std::isfinite(origen.x) || !std::isfinite(origen.y) ||
        origen.x < 0 || origen.x > celda || origen.y < 0 || origen.y > celda ||
        !std::isfinite(escala) || escala < 0.25f || escala > 2.0f ||
        !std::isfinite(zancada) || zancada < 4.0f || zancada > 512.0f ||
        !std::isfinite(proyeccion) || proyeccion < 0.1f || proyeccion > 1.0f)
    {
        return false;
    }
    std::string sobrante;
    if (archivo >> sobrante)
    {
        return false;
    }
    std::array<sf::Texture, 8> texturas;
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        const std::string ruta = carpeta + "/cuerpo_" + std::to_string(direccion) + ".png";
        std::ifstream entrada(ruta, std::ios::binary);
        sf::Image imagen;
        if (!entrada.good() || !imagen.loadFromFile(ruta) ||
            imagen.getSize() != sf::Vector2u(celda * COLUMNAS, celda * FILAS))
        {
            return false;
        }
        for (int cuadro = 0; cuadro < COLUMNAS * FILAS; ++cuadro)
        {
            if (cuadro > INICIO_REPOSO && cuadro < INICIO_CONTACTO)
            {
                continue;
            }
            bool visible = false;
            for (int y = 0; y < celda; ++y)
            {
                for (int x = 0; x < celda; ++x)
                {
                    if (imagen.getPixel({static_cast<unsigned>(cuadro % COLUMNAS * celda + x),
                                         static_cast<unsigned>(cuadro / COLUMNAS * celda + y)}).a > 0)
                    {
                        visible = true;
                        break;
                    }
                }
                if (visible)
                {
                    break;
                }
            }
            if (!visible)
            {
                return false;
            }
        }
        if (!texturas[direccion].loadFromImage(imagen))
        {
            return false;
        }
        texturas[direccion].setSmooth(false);
    }
    PaqueteAtlas& paquete = _paquetes[indice];
    paquete.celda = celda;
    paquete.texturas = std::move(texturas);
    paquete.origen = origen;
    paquete.escala = escala;
    paquete.zancada = zancada;
    paquete.proyeccion = proyeccion;
    return true;
}

int AspectoEnemigos::obtenerDireccion(float x, float y, int anterior) const
{
    if (std::hypot(x, y) < 0.001f)
    {
        return anterior;
    }
    const float angulo = std::atan2(x, y);
    const float previo = anterior * PI / 4.0f;
    const float diferencia = std::remainder(angulo - previo, 2.0f * PI);
    // Un margen de tres grados evita alternar vistas en un borde del sector.
    if (std::abs(diferencia) <= PI / 8.0f + PI / 60.0f)
    {
        return anterior;
    }
    return (static_cast<int>(std::floor(angulo / (PI / 4.0f) + 0.5f)) + 8) % 8;
}

void AspectoEnemigos::actualizar(const std::vector<Enemigo>& enemigos,
                                FormaColision jugador, float segundos)
{
    if (!std::isfinite(segundos) || segundos <= 0.0f ||
        !formaValida(jugador))
    {
        return;
    }
    for (auto& entrada : _estados)
    {
        entrada.second.presente = false;
    }
    for (const Enemigo& enemigo : enemigos)
    {
        const FormaColision forma = enemigo.obtenerFormaColision();
        const int idEnemigo = enemigo.obtenerIdEnemigo();
        if (!animacionDisponible(idEnemigo) || !enemigo.estaVivo() || !formaValida(forma))
        {
            continue;
        }
        const Posicion posicion = enemigo.obtenerPosicion();
        const PaqueteAtlas& paquete = _paquetes[idEnemigo - 1];
        auto resultado = _estados.try_emplace(enemigo.obtenerIdInstancia());
        EstadoVisual& estado = resultado.first->second;
        if (resultado.second)
        {
            estado.anterior = posicion;
            estado.direccion = obtenerDireccion(
                jugador.x + jugador.ancho / 2.0f - forma.x - forma.ancho / 2.0f,
                jugador.y + jugador.alto / 2.0f - forma.y - forma.alto / 2.0f, 1);
        }
        const float x = posicion.x - estado.anterior.x;
        const float y = posicion.y - estado.anterior.y;
        const float distancia = std::hypot(x, y);
        estado.enMovimiento = distancia > 0.001f && distancia < 128.0f;
        if (estado.enMovimiento)
        {
            estado.direccion = obtenerDireccion(x, y, estado.direccion);
            const float angulo = estado.direccion * PI / 4.0f;
            const float horizontal = std::sin(angulo);
            const float vertical = std::cos(angulo) * paquete.proyeccion;
            const float pasoProyectado = paquete.zancada * std::hypot(horizontal, vertical);
            estado.fase = std::fmod(estado.fase + distancia / pasoProyectado, 1.0f);
        }
        else if (distancia >= 128.0f)
        {
            estado.fase = 0.0f;
        }
        const bool contacto = SistemaColisiones::hayInterseccion(forma, jugador);
        estado.tiempoContacto = contacto && estado.enContacto ?
            std::fmod(estado.tiempoContacto + segundos, INTERVALO_CONTACTO) : 0.0f;
        estado.enContacto = contacto;
        estado.anterior = posicion;
        estado.presente = true;
    }
    for (auto entrada = _estados.begin(); entrada != _estados.end();)
    {
        if (!entrada->second.presente)
        {
            entrada = _estados.erase(entrada);
        }
        else
        {
            ++entrada;
        }
    }
}

int AspectoEnemigos::obtenerCuadro(const EstadoVisual& estado) const
{
    if (estado.enContacto && estado.tiempoContacto < DURACION_CONTACTO)
    {
        return INICIO_CONTACTO + std::min(CUADROS_CONTACTO - 1,
            static_cast<int>(estado.tiempoContacto / DURACION_CONTACTO * CUADROS_CONTACTO));
    }
    return estado.enMovimiento ? std::min(CUADROS_MARCHA - 1,
        static_cast<int>(estado.fase * CUADROS_MARCHA)) : INICIO_REPOSO;
}

bool AspectoEnemigos::dibujar(sf::RenderTarget& destino, const Enemigo& enemigo) const
{
    const FormaColision forma = enemigo.obtenerFormaColision();
    const int idEnemigo = enemigo.obtenerIdEnemigo();
    if (!animacionDisponible(idEnemigo) ||
        !enemigo.estaVivo() || !formaValida(forma))
    {
        return false;
    }
    const auto entrada = _estados.find(enemigo.obtenerIdInstancia());
    const EstadoVisual inicial{};
    const EstadoVisual& estado = entrada != _estados.end() ? entrada->second : inicial;
    const PaqueteAtlas& paquete = _paquetes[idEnemigo - 1];
    const int cuadro = obtenerCuadro(estado);
    sf::Sprite figura(paquete.texturas[estado.direccion], sf::IntRect(
        {cuadro % COLUMNAS * paquete.celda, cuadro / COLUMNAS * paquete.celda},
        {paquete.celda, paquete.celda}));
    figura.setOrigin(paquete.origen);
    figura.setScale({paquete.escala, paquete.escala});
    figura.setPosition({forma.x + forma.ancho / 2.0f,
                        forma.y + forma.alto / 2.0f + BASE_VISUAL});
    destino.draw(figura);
    return true;
}

void AspectoEnemigos::reiniciar()
{
    _estados.clear();
}

bool AspectoEnemigos::animacionDisponible(int idEnemigo) const
{
    return idEnemigo >= 1 && idEnemigo <= static_cast<int>(_paquetes.size()) &&
           _paquetes[idEnemigo - 1].disponible;
}
