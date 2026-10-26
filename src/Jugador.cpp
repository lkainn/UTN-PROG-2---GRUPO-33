#include "Jugador.h"
#include "Configuracion.h"
#include "ArchivoJugadores.h"
#include "ArchivoMejoras.h"
#include <limits>


Jugador::Jugador() {
    // Inicializa la posicion centrada
    _posicion = { Configuracion::ANCHO_VENTANA / 2.0f, Configuracion::ALTO_VENTANA / 2.0f };

    // Leer el perfil básico guardado (Nivel y Chatarra)
    ArchivoJugadores archivo;
    RegistroJugador reg = archivo.leer();
    _nivel = reg.nivelMaximo;
    _chatarra = reg.chatarraTotal;

    // Establecer estadisticas base escaladas por el nivel actual
    _velocidad = 5.0f;
    _vidaMaxima = 100 + (_nivel - 1) * 20;
    _experienciaSiguienteNivel = 100 + (_nivel - 1) * 50;

    // Leer el catalogo de mejoras y aplicar los incrementos
    ArchivoMejoras gestorMejoras;
    std::vector<Mejora> catalogo = gestorMejoras.leerCatalogo();

    for (const Mejora& m : catalogo) {
        if (m.idMejora == 1) { // Vida Máxima
            _vidaMaxima += (m.nivelActual * 20);
        }
        else if (m.idMejora == 2) { // Velocidad
            _velocidad += (m.nivelActual * 1.0f);
        }
    }

    // Inicializar el estado de la partida actual
    _vida = _vidaMaxima;
    _experiencia = 0;
    _vivo = true;

    // Configuracion cuadrado para pruebas
    _formaGrafica.setSize({ 48.0f, 48.0f });
    _formaGrafica.setFillColor(sf::Color(50, 200, 50));
    _formaGrafica.setOrigin(_formaGrafica.getGeometricCenter());
    _formaGrafica.setPosition({ _posicion.x, _posicion.y });
}

void Jugador::procesarEntrada() {
    if (!_vivo) return;
    // Movimientos personaje WASD o flechas
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) { _posicion.y -= _velocidad;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) { _posicion.y += _velocidad;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) { _posicion.x -= _velocidad;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) { _posicion.x += _velocidad;
    }
}

void Jugador::validarLimites() {
    float radio = _formaGrafica.getSize().x / 2.0f;

    if (_posicion.x - radio < 0.0f) _posicion.x = radio;

    if (_posicion.x + radio > Configuracion::ANCHO_VENTANA) _posicion.x = Configuracion::ANCHO_VENTANA - radio;

    if (_posicion.y - radio < 0.0f) _posicion.y = radio;

    if (_posicion.y + radio > Configuracion::ALTO_VENTANA) _posicion.y = Configuracion::ALTO_VENTANA - radio;
}

// Sincroniza el grafico; por defecto tambien conserva los limites de la ventana.
void Jugador::actualizar(bool limitarALaVentana)
{
    if (!_vivo)
    {
        return;
    }
    // TEMPORAL: conservar el limite original salvo en la prueba de camara de Juego.
    if (limitarALaVentana)
    {
        validarLimites();
    }
    _formaGrafica.setPosition({_posicion.x, _posicion.y});
}

void Jugador::recibirDano(int cantidad) {  // Se resta el dano en la variable
    if (!_vivo || cantidad <= 0) return;  // Se valida si el jugador esta muerto o si la cantidad recibida no es valida

    _vida -= cantidad;
    if (_vida <= 0) {
    _vida = 0;
    _vivo = false;
    _formaGrafica.setFillColor(sf::Color(100, 100, 100));  // Color gris al morir prueba
    }
}

void Jugador::curar(int cantidad) {
    if (!_vivo || cantidad <= 0) return;
    _vida += cantidad;
    if (_vida > _vidaMaxima) {
        _vida = _vidaMaxima;
    }
}

// Algoritmo para procesar la subida de niveles
void Jugador::verificarSubidaNivel() {
    while (_experiencia >= _experienciaSiguienteNivel) {
    _experiencia -= _experienciaSiguienteNivel;
    _nivel++;
    _experienciaSiguienteNivel += 50;
    _vidaMaxima += 20;
    _vida = _vidaMaxima; // Aumenta vida completa al subir de nivel

    ArchivoJugadores archivo;
    RegistroJugador reg = {_nivel, 0, _chatarra};
    archivo.guardar(reg);
    }
}

void Jugador::ganarExperiencia(int cantidad) {
    if (!_vivo || cantidad <= 0) return;  // Se valida si el jugador esta muerto o si la cantidad recibida no es valida
    _experiencia += cantidad;
    verificarSubidaNivel();
}

void Jugador::ganarChatarra(int cantidad) {
    if (!_vivo || cantidad <=0) return;
    _chatarra += cantidad;

    ArchivoJugadores archivo;
    RegistroJugador reg = {_nivel, 0, _chatarra};
    archivo.guardar(reg);
 }

// Metodos para la estacion de prueba
bool Jugador::gastarChatarra(int cantidad)
{
    if (!_vivo || cantidad <= 0 || cantidad > _chatarra)
    {
        return false;
    }
    _chatarra -= cantidad;

    ArchivoJugadores archivo;
    RegistroJugador reg = {_nivel, 0, _chatarra};
    archivo.guardar(reg);

    return true;
}

void Jugador::aumentarVidaMaxima(int cantidad)
{
    if (_vivo && cantidad > 0 &&
        _vidaMaxima <= std::numeric_limits<int>::max() - cantidad)
    {
        _vidaMaxima += cantidad;
        _vida += cantidad;
    }
}

void Jugador::aumentarVelocidad(float cantidad) {
    if (cantidad > 0.0f) {
        _velocidad += cantidad;
    }
}

void Jugador::reiniciar() {
    _posicion = {
        Configuracion::ANCHO_VENTANA / 2.0f,
        Configuracion::ALTO_VENTANA / 2.0f
    };
    _vida = _vidaMaxima;
    _vivo = true;
    _formaGrafica.setFillColor(sf::Color(50, 200, 50));
    _formaGrafica.setPosition({_posicion.x, _posicion.y});
}

void Jugador::dibujar(sf::RenderWindow& ventana){
    ventana.draw(_formaGrafica);
}

Posicion Jugador::getPosicion() const {
    return _posicion; }

FormaColision Jugador::getFormaColision() const {
    sf::Vector2f tam = _formaGrafica.getSize();
    return { _posicion.x - tam.x / 2.0f, _posicion.y - tam.y / 2.0f, tam.x, tam.y };
}

int Jugador::getVida() const { return _vida; }
int Jugador::getVidaMaxima() const { return _vidaMaxima; }
int Jugador::getNivel() const { return _nivel; }
int Jugador::getExperiencia() const { return _experiencia; }
int Jugador::getExperienciaSiguienteNivel() const { return _experienciaSiguienteNivel; }
int Jugador::getChatarra() const { return _chatarra; }
float Jugador::getVelocidad() const { return _velocidad; }
bool Jugador::estaVivo() const { return _vivo; }

float Jugador::getPorcentajeVida() const {
    if (_vidaMaxima <= 0) return 0.0f;
    return static_cast<float>(_vida) / static_cast<float>(_vidaMaxima);
}

float Jugador::getPorcentajeExperiencia() const {
    if (_experienciaSiguienteNivel <= 0) return 0.0f;
    return static_cast<float>(_experiencia) / static_cast<float>(_experienciaSiguienteNivel);
}

void Jugador::setPosicion(Posicion pos) {
    _posicion = pos;
    _formaGrafica.setPosition({ _posicion.x, _posicion.y });
}

void Jugador::cargarEstado(int vida, int nivel, int exp, int chatarra) {  // Asigna la vida, nivel, experiencia y chatarra al personaje. Al cambiar de nivel cambian los topes del juego, se recalcula la salud maxima, la meta de experiencia, si esta vivo y su color
    // Guardar los datos básicos y evitar valores absurdos
    if (nivel < 1) {
        _nivel = 1;
    }
        else {
            _nivel = nivel;
}

    if (exp < 0) {
        _experiencia = 0;
}
        else {
            _experiencia = exp;
}
    if (chatarra < 0) {
        _chatarra = 0;
}
        else {
            _chatarra = chatarra;
}
// Recalcular los topes del personaje segun su nivel (+20 de vida max y +50 de exp por nivel)
    _vidaMaxima = 100 + (_nivel - 1) * 20;
    _experienciaSiguienteNivel = 100 + (_nivel - 1) * 50;

// Asegurar que la vida actual no supere la nueva vida maxima ni sea menor a 0
    if (vida > _vidaMaxima) {
        _vida = _vidaMaxima;
}
        else if (vida < 0) {
            _vida = 0;
}
        else {
            _vida = vida;
}
// Marcar si el personaje esta vivo o muerto y actualizar su color
    if (_vida > 0) {
        _vivo = true;
        _formaGrafica.setFillColor(sf::Color(50, 200, 50)); // Verde si esta vivo
}
        else {
            _vivo = false;
            _formaGrafica.setFillColor(sf::Color(100, 100, 100)); // Gris si esta muerto
        }
}

void Jugador::restablecerProgreso() {
    // Sobrescribir Jugadores.dat con los valores iniciales (Nivel 1, 0 puntos, 0 chatarra)
    ArchivoJugadores archivoJugador;
    archivoJugador.guardar({1, 0, 0});

    // Sobrescribir Mejoras.dat con el catálogo en nivel 0
    ArchivoMejoras gestorMejoras;
    std::vector<Mejora> catalogoBase;
    catalogoBase.push_back({1, "Vida Maxima", 0, 5, "Aumenta tu salud limite"});
    catalogoBase.push_back({2, "Velocidad", 0, 3, "Corres mas rapido"});
    gestorMejoras.guardarCatalogo(catalogoBase);

    _nivel = 1;
    _chatarra = 0;
    _velocidad = 5.0f;
    _vidaMaxima = 100;
    _vida = _vidaMaxima;
    _experiencia = 0;
    _experienciaSiguienteNivel = 100;
}
