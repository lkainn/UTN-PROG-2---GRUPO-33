#pragma once

// Estado y resultado de una partida.
//
// Lleva la cuenta de lo que hay que mostrar mientras se juega y de lo que hay
// que guardar al terminar: los mismos campos que el registro de Partidas.dat.
//
// No conoce a Jugador, a Enemigo ni a SFML: recibe numeros y devuelve numeros.
// Quien coordina el ciclo de juego le informa lo que va pasando y le pregunta
// si la partida sigue en curso.

enum class ResultadoPartida {
    EnCurso,
    Victoria,   // se derroto al jefe final
    Derrota     // la vida del jugador llego a cero
};

class Partida {
    public:
        // Cuanto suma al puntaje cada enemigo eliminado.
        static const int PUNTOS_POR_ENEMIGO;

        // Cuanto suma cada unidad de chatarra recogida.
        static const int PUNTOS_POR_CHATARRA;

        // Premio por derrotar al jefe. Separa claramente una partida ganada
        // de una sobrevivida mucho tiempo pero perdida.
        static const int PUNTOS_POR_VICTORIA;

        explicit Partida(int idJugador);

        // Una vez por fotograma, solo mientras la partida sigue en curso: al
        // terminar, la duracion queda congelada en el momento del desenlace.
        void actualizar(float dt);

        // Datos que informa el ciclo de juego a medida que ocurren.
        void registrarEliminados(int cantidad);
        void registrarChatarra(int cantidad);
        void registrarNivel(int nivel);

        // Desenlaces. El primero que llegue define el resultado: una vez
        // terminada, la partida ya no cambia.
        void marcarVictoria();
        void marcarDerrota();

        // Consultas.
        bool estaEnCurso() const;
        ResultadoPartida obtenerResultado() const;
        bool fueVictoria() const;

        int obtenerIdPartida() const;
        int obtenerIdJugador() const;
        int obtenerPuntaje() const;
        int obtenerNivelAlcanzado() const;
        int obtenerEnemigosEliminados() const;
        int obtenerChatarraObtenida() const;

        // Entero, como lo guarda Partidas.dat. Adentro se acumula en float
        // porque el delta time de cada fotograma es una fraccion de segundo.
        int obtenerDuracionSegundos() const;

        // Lo asigna quien escribe en Partidas.dat: es el numero de registro,
        // y recien se conoce al guardar.
        void asignarIdPartida(int id);

    private:
        int idPartida;
        int idJugador;
        int puntaje;
        int nivelAlcanzado;
        int enemigosEliminados;
        int chatarraObtenida;
        float duracionSegundos;
        ResultadoPartida resultado;

        // Recalcula el puntaje a partir de los contadores. Se llama cada vez
        // que uno cambia, asi el valor mostrado siempre esta al dia.
        void recalcularPuntaje();
};
