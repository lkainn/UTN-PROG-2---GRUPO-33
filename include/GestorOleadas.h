#pragma once

#include "Enemigo.h"
#include "TiposCompartidos.h"

#include <cstddef>
#include <random>
#include <vector>

// Administra los enemigos de una partida: cuando aparecen, donde, de que
// variante, y los va limpiando cuando mueren. La dificultad sube con el
// tiempo transcurrido, no con la cantidad de enemigos eliminados.
//
// Quien la usa (Juego, o la prueba del modulo) solo necesita:
//   1. llamar a actualizar() una vez por fotograma
//   2. recorrer obtenerEnemigos() para dibujarlos y para las colisiones
//   3. llamar a reiniciar() cuando arranca una partida nueva
//
// No depende de SFML: trabaja con Posicion y con segundos.
class GestorOleadas {
    public:
        // Los enemigos aparecen en un anillo alrededor del jugador, a esta
        // distancia. Tiene que ser mayor que media pantalla para que no se
        // los vea aparecer de la nada.
        static const float DISTANCIA_APARICION;

        // Duracion de cada oleada. Solo sirve para numerarlas y mostrarlas:
        // la dificultad es continua, no salta de golpe al cambiar de oleada.
        static const float SEGUNDOS_POR_OLEADA;

        // A los cuantos segundos de partida se alcanza la dificultad maxima.
        // Despues de ese momento los valores se mantienen en su techo.
        static const float SEGUNDOS_DIFICULTAD_MAXIMA;

        // Cada cuantos segundos aparece un enemigo: al empezar y al final de
        // la curva de dificultad.
        static const float INTERVALO_APARICION_INICIAL;
        static const float INTERVALO_APARICION_MINIMO;

        // Techo de enemigos vivos al mismo tiempo, al empezar y al final.
        // Sin este limite el vector crece sin parar y el juego se arrastra.
        static const std::size_t MAXIMO_VIVOS_INICIAL;
        static const std::size_t MAXIMO_VIVOS_FINAL;

        // A partir de que segundo empieza a aparecer cada variante.
        static const float SEGUNDOS_APARECE_RAPIDO;
        static const float SEGUNDOS_APARECE_RESISTENTE;

        GestorOleadas();

        // Genera, mueve y limpia. En ese orden.
        void actualizar(Posicion posicionJugador, float dt);

        // Vacia la lista y reinicia el reloj de partida.
        void reiniciar();

        // Para dibujar y para las colisiones. Es solo lectura: los enemigos
        // se modifican unicamente desde actualizar().
        const std::vector<Enemigo>& obtenerEnemigos() const;

        std::size_t cantidadEnemigos() const;

        // Dano a todos los vivos de una vez. Es para probar el modulo sin
        // proyectiles: en el juego el dano lo aplica quien detecta el impacto.
        void danarATodos(int cantidad);

        // Consultas para la interfaz y para la clase Partida.
        float tiempoDePartida() const;   // segundos desde que empezo
        int oleadaActual() const;        // 1, 2, 3... un numero por tramo
        float intervaloActual() const;   // cada cuanto aparece uno ahora
        std::size_t maximoVivosActual() const;

    private:
        std::vector<Enemigo> enemigos;
        float tiempoPartida;        // segundos desde el inicio de la partida
        float tiempoAcumulado;      // segundos desde la ultima aparicion
        std::mt19937 generador;     // numeros al azar para angulo y variante

        // Cuanto avanzo la curva de dificultad: 0 al empezar, 1 al llegar a
        // SEGUNDOS_DIFICULTAD_MAXIMA. Los valores se interpolan con esto.
        float progresoDificultad() const;

        // Elige un punto al azar del anillo alrededor del centro dado.
        Posicion posicionDeAparicion(Posicion centro);

        // Sortea entre las variantes ya habilitadas por el tiempo de partida.
        TipoEnemigo variantePorAparecer();

        void generarSiCorresponde(Posicion posicionJugador, float dt);
        void moverEnemigos(Posicion posicionJugador, float dt);
        void quitarMuertos();
};
