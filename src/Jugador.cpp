#include "Jugador.h"
#include "Configuracion.h"

Jugador::Jugador() {
    // Inicializa la posicion centrada
    _posicion = { Configuracion::ANCHO_VENTANA / 2.0f, Configuracion::ALTO_VENTANA / 2.0f };
    // Las estadisticas iniciales del personaje
    _velocidad = 5.0f;
    _vidaMaxima = 100;
    _vida = _vidaMaxima;
    _nivel = 1;
    _experiencia = 0;
    _experienciaSiguienteNivel = 100;
    _chatarra = 0;
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

//Se ejecuta en cada fotograma, revisa que las coordenadas del jugador no pase los limites de la ventana y sincroniza con el grafico
void Jugador::actualizar() {
if (!_vivo) return;
validarLimites();
_formaGrafica.setPosition({ _posicion.x, _posicion.y });
}

void Jugador::recibirDano(int cantidad) {  // Se resta el dano en la variable
    if (!_vivo) return;
    _vida -= cantidad;
    if (_vida <= 0) {
    _vida = 0;
    _vivo = false;
    _formaGrafica.setFillColor(sf::Color(100, 100, 100));  // Color gris al morir prueba
    }
}
// Algoritmo para procesar la subida de niveles
void Jugador::verificarSubidaNivel() {
    while (_experiencia >= _experienciaSiguienteNivel) {
    _experiencia -= _experienciaSiguienteNivel;
    _nivel++;
    _experienciaSiguienteNivel += 50;
    _vidaMaxima += 20;
    _vida = _vidaMaxima; // Aumenta vida al subir de nivel
    }
}

void Jugador::ganarExperiencia(int cantidad) {
    if (!_vivo) return;
    _experiencia += cantidad;
    verificarSubidaNivel();
}

void Jugador::ganarChatarra(int cantidad) {
    if (!_vivo) return;
    _chatarra += cantidad;
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

Posicion Jugador::getPosicion() const { return _posicion; }

FormaColision Jugador::getFormaColision() const {
    sf::Vector2f tam = _formaGrafica.getSize();
    return { _posicion.x - tam.x / 2.0f, _posicion.y - tam.y / 2.0f, tam.x, tam.y };
}

int Jugador::getVida() const { return _vida; }
int Jugador::getVidaMaxima() const { return _vidaMaxima; }
int Jugador::getNivel() const { return _nivel; }
int Jugador::getExperiencia() const { return _experiencia; }
int Jugador::getChatarra() const { return _chatarra; }
bool Jugador::estaVivo() const { return _vivo; }

void Jugador::setPosicion(Posicion pos) {
    _posicion = pos;
    _formaGrafica.setPosition({ _posicion.x, _posicion.y });
}

void Jugador::cargarEstado(int vida, int nivel, int exp, int chatarra) {
    _vida = vida;
    _nivel = nivel;
    _experiencia = exp;
    _chatarra = chatarra;
}
