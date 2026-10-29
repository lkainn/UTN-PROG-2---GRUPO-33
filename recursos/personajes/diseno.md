# R-17, Remache

Robot recolector con acero gris, cobre oxidado, hombrera naranja, visor verde
menta y mochila con iman. La apariencia usa pintura saltada, bordes con metal
expuesto, oxido localizado, rayaduras y suciedad en botas y pinzas.

## Representacion en el juego

AspectoJugador carga primero el paquete remache_atlas: ocho vistas independientes,
marcha de 24 cuadros y reposo. Las vistas conservan el lado anatomico de la
hombrera y del iman; no se reflejan horizontalmente. La orientacion sigue el
desplazamiento real. Al detenerse conserva la ultima vista y usa el primer cuadro
de reposo. La muerte conserva esa vista con tinte oscuro; el impacto usa tinte
rojo. El reinicio vuelve a la vista de tres cuartos hacia abajo y a la derecha.

La fase de carrera avanza por distancia euclidea recorrida, igual en los ocho
sentidos. Una zancada visual equivale aproximadamente a 151,10 px: a la velocidad
inicial de 5 px por actualizacion y 60 FPS se reproducen unos dos ciclos por
segundo. Las piernas alternan a media fase, con flexion y elevacion del pie.
La escala y el pivote son comunes
a todos los cuadros: 144 px de alto en el reposo de referencia. Los cambios de
direccion conservan la fase; la pausa no avanza la pose. Los saltos de mas de
128 px y las posiciones no finitas reinician el ciclo visual.

El jugador conserva su velocidad cardinal y colision de 48 x 48. La direccion
de entrada se normaliza para que la diagonal tenga la misma rapidez; las teclas
opuestas se cancelan. Las armas mantienen sus tamanos y reglas de combate. El ritmo visible se adapta
a la velocidad efectiva del juego; los ocho giros discretos no constituyen
un giro continuo del modelo.

## Manos y armas

Ambos brazos usan una postura de apuntado con IK sin estiramiento. El cuerpo
completo se orienta al enemigo vivo visible mas cercano dentro del alcance
de alguna de las armas, antes de calcular los anclajes de ambos slots. El
apuntado se actualiza incluso durante la recarga y para el arma que aun no
tiene alcance para disparar. Sin objetivo, cuerpo y armas siguen el movimiento.
Al retroceder respecto del objetivo se recorre el ciclo en sentido inverso.

Cada cuadro incluye dos puntos de agarre dentro de las pinzas. El slot 0 usa
la mano cercana y el slot 1 la lejana, segun la profundidad de cada vista. El
arma lejana se dibuja antes del cuerpo y la cercana despues. Los dedos visibles,
exportados con la oclusion del cuerpo completo, se superponen a las empunaduras.
Los proyectiles siguen naciendo en la boca de la misma arma que se dibuja.
Los ocho giros son discretos y los sprites no resuelven apoyos adaptativos en
el mundo: la prioridad de esta version es la lectura de los pasos y del apuntado.

## Recursos y respaldo

El paquete usa celdas de 192 x 192, paleta fija de 64 colores, transparencia
binaria, contorno exterior de un pixel y filtrado sin suavizado. Los archivos
animacion.txt, cuerpo_0.png a cuerpo_7.png y dedos_0.png a dedos_7.png forman un
conjunto indivisible. La carga valida todos los metadatos y las dimensiones
antes de activar la animacion. Las texturas y los datos pertenecen por valor
a la clase, mediante contenedores de tamano fijo, sin reservas manuales.

Si el paquete falta o es invalido, se utiliza remache_v1.png y su animacion
anterior por recortes. Esa imagen es el concepto de 1216 x 1293, no una hoja de
sprites. Si tampoco existe, se conserva el cuadrado verde original.

Las rutas admiten ejecucion desde la raiz del proyecto y desde bin/Debug.
Ejecutar pruebas/probar_aspecto_jugador.bat para comprobar vistas, fase,
manos, muzzles, respaldo, escala, pausa y reinicio. La muestra armada queda
en bin/Debug/remache_revision/remache_ocho_vistas_armado.png.
