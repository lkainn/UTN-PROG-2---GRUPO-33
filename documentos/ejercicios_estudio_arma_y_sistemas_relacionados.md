# Ejercicios de estudio - Arma y sistemas relacionados

Estos ejercicios fueron separados de la guia principal para poder utilizarlos
como material de practica del Proyecto TPI. Conviene intentar resolver cada uno
antes de consultar las respuestas.

## Ejercicio 1: cooldown

Un arma tiene `velocidadAtaque = 4.0f`.

1. ¿Cuanto dura su intervalo entre disparos?
2. Si acaba de disparar y pasan `0.10` segundos, ¿puede volver a disparar?

## Ejercicio 2: direccion horizontal

El jugador esta en `(200, 300)` y el enemigo en `(500, 300)`.

1. ¿Cuanto valen `diferenciaX` y `diferenciaY`?
2. ¿Cual es la direccion normalizada?

## Ejercicio 3: direccion diagonal

El origen es `(0, 0)` y el objetivo `(30, 40)`.

1. ¿Cual es la distancia?
2. ¿Cual es la direccion normalizada?
3. Si la velocidad es `200`, ¿cuanto valen `velocidadX` y `velocidadY`?

## Ejercicio 4: arreglos paralelos

Si muere `enemigosPrueba[2]`, ¿en que elemento debe guardarse su tiempo de
reaparicion?

## Ejercicio 5: seguimiento de estado

Dos enemigos mueren con un segundo de diferencia y cada uno tarda tres segundos
en reaparecer. Explica por que no deberian usar una unica variable de tiempo.

## Ejercicio 6: cambio pequeno

Busca en `Juego.cpp` la constante `SEGUNDOS_REAPARICION`. Predice que ocurriria
si cambiara de `3.0f` a `5.0f`. Luego identifica que partes del codigo no seria
necesario modificar.

## Respuestas

### Respuesta 1

```text
1 / 4 = 0,25 segundos
```

Despues de `0.10` segundos todavia faltan `0.15`, por lo que el arma no puede
volver a disparar.

### Respuesta 2

```text
diferenciaX = 500 - 200 = 300
diferenciaY = 300 - 300 = 0
direccion = (1, 0)
```

### Respuesta 3

```text
distancia = sqrt(30² + 40²) = 50
direccion = (30 / 50, 40 / 50) = (0,6, 0,8)
velocidad = (0,6 * 200, 0,8 * 200) = (120, 160)
```

### Respuesta 4

Debe guardarse en:

```cpp
tiemposReaparicion[2]
```

Los arreglos paralelos relacionan sus elementos mediante el mismo indice.

### Respuesta 5

Una sola variable no puede representar dos tiempos restantes diferentes. Cada
enemigo necesita conservar su propio temporizador para reaparecer en el momento
correspondiente.

### Respuesta 6

Los enemigos tardarian cinco segundos en reaparecer. No seria necesario cambiar
el ciclo, el arreglo de temporizadores ni la funcion que reconstruye enemigos,
porque esas partes consultan la misma constante.
