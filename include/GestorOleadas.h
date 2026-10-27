#pragma once

#include "ArchivoEnemigos.h"
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

        // En que segundo aparece el jefe, y cuanto antes se avisa para que el
        // jugador no se lo encuentre de golpe.
        static const float SEGUNDOS_APARECE_JEFE;
        static const float SEGUNDOS_AVISO_JEFE;

        // Mientras el jefe vive, los enemigos comunes aparecen a este ritmo
        // respecto del normal: 2.0 significa la mitad de seguido. Mantiene la
        // presion sin hacer la pelea imposible.
        static const float FACTOR_INTERVALO_CON_JEFE;

        // Al construirse lee el catalogo de datos/Enemigos.dat. Si el
        // archivo no existe lo crea con los valores de fabrica.
        GestorOleadas();

        // Genera, mueve y limpia. En ese orden.
        void actualizar(Posicion posicionJugador, float dt);

        // Vacia la lista y reinicia el reloj de partida.
        void reiniciar();

        // Consulta de solo lectura para dibujar y revisar colisiones.
        const std::vector<Enemigo>& obtenerEnemigos() const;

        // Para aplicar dano individual. No modificar la lista durante un recorrido.
        std::vector<Enemigo>& obtenerEnemigos();

        std::size_t cantidadEnemigos() const;

        // Cuantos murieron en toda la partida. Lo cuenta el gestor al sacarlos
        // del vector, que es el unico lugar donde una muerte ocurre una sola
        // vez: si lo contara quien aplica el dano habria que recordar a quien
        // ya se conto para no sumarlo dos veces.
        int enemigosEliminados() const;

        // Dano a todos los vivos de una vez. Es para probar el modulo sin
        // proyectiles: en el juego el dano lo aplica quien detecta el impacto.
        void danarATodos(int cantidad);

        // Consultas para la interfaz y para la clase Partida.
        float tiempoDePartida() const;   // segundos desde que empezo
        int oleadaActual() const;        // 1, 2, 3... un numero por tramo
        float intervaloActual() const;   // cada cuanto aparece uno ahora
        std::size_t maximoVivosActual() const;

        // Estado del jefe. Quien dibuja decide como avisar y como mostrar la
        // victoria: el gestor solo informa en que momento esta la partida.
        bool jefeEnCamino() const;       // ya se puede avisar, todavia no salio
        float segundosParaElJefe() const;  // 0 si ya aparecio o ya fue vencido
        bool jefeEstaVivo() const;       // esta en el mapa y le queda vida
        bool jefeFueDerrotado() const;   // condicion de victoria de la partida

    private:
        std::vector<Enemigo> enemigos;
        float tiempoPartida;        // segundos desde el inicio de la partida
        float tiempoAcumulado;      // segundos desde la ultima aparicion
        std::mt19937 generador;     // numeros al azar para angulo y variante

        // Estadisticas de cada variante, leidas una sola vez al empezar.
        std::vector<RegistroEnemigo> catalogo;

        int totalEliminados;        // acumulado de la partida

        bool jefeGenerado;          // ya aparecio en esta partida
        bool jefeDerrotado;         // lo mataron: la partida se gana
        std::uint64_t idInstanciaJefe;  // para reconocerlo dentro del vector

        // Cuanto avanzo la curva de dificultad: 0 al empezar, 1 al llegar a
        // SEGUNDOS_DIFICULTAD_MAXIMA. Los valores se interpolan con esto.
        float progresoDificultad() const;

        // Elige un punto al azar del anillo alrededor del centro dado.
        Posicion posicionDeAparicion(Posicion centro);

        // Sortea entre las variantes ya habilitadas por el tiempo de partida.
        TipoEnemigo variantePorAparecer();

        // Busca la fila del catalogo que corresponde a una variante. Devuelve
        // nullptr si el archivo no la trae.
        const RegistroEnemigo* registroDe(TipoEnemigo tipo) const;

        // Crea un enemigo de esa variante usando el catalogo, o los valores
        // escritos en crearEnemigo() si el catalogo no la tiene.
        Enemigo fabricar(TipoEnemigo tipo, float x, float y) const;

        void generarJefeSiCorresponde(Posicion posicionJugador);
        void generarSiCorresponde(Posicion posicionJugador, float dt);
        void moverEnemigos(Posicion posicionJugador, float dt);
        void detectarJefeDerrotado();
        void quitarMuertos();
};
