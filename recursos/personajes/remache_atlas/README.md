# Atlas de Remache

El paquete completo requiere animacion.txt y los ocho pares cuerpo_N.png y
dedos_N.png. No se recortan las celdas individualmente ni se reflejan las vistas.

Direcciones: 0 abajo, 1 abajo-derecha, 2 derecha, 3 arriba-derecha, 4 arriba,
5 arriba-izquierda, 6 izquierda y 7 abajo-izquierda. Los giros diagonales se
renderizan con la misma camara oblicua que los cardinales.

Cada cuerpo mide 1152 x 1536: seis columnas por ocho filas de celdas 192 x 192.
Las primeras cuatro filas contienen marcha y las siguientes cuatro, reposo.
Cada clip tiene 24 cuadros; el juego utiliza el cuadro 0 al estar quieto.

Cada textura de dedos mide 384 x 256: doce columnas por ocho filas de celdas
32 x 32. Las seis primeras columnas son la mano cercana (slot 0) y las seis
restantes la lejana (slot 1). Los dedos ya respetan la oclusion del cuerpo.

animacion.txt comienza con REMACHE_ATLAS y version 1. Luego registra celda,
cuadros, columnas, direcciones y celda de dedos; origen X/Y y escala; zancada
en pixeles y factor de proyeccion de referencia de la camara. La fase usa
distancia euclidea del juego, sin multiplicar el movimiento vertical por ese
factor. Siguen 384 registros ordenados
por direccion, clip y cuadro. Cada registro contiene esos tres indices y,
para cada mano, palma X/Y y esquina X/Y del recorte de dedos dentro de la
celda del cuerpo. Todas las coordenadas se expresan antes de escalar.

AspectoJugador aplica el mismo pivote, escala, direccion y cuadro al cuerpo,
los dedos y los puntos de agarre. La carga valida el conjunto completo antes
de activarlo; un paquete invalido utiliza el dibujo anterior como respaldo.
