# Escopeta y canon de energia: prototipo de combate

Se agregaron las mecanicas de las dos armas. Sus armas y
proyectiles utilizan PNG transparentes. La integracion se explica en
[la guia de graficos](guia_graficos_armas_nuevas.md).
El balance es provisional y debe probarse jugando.
La tabla muestra las bases: las ofertas pueden aumentar dano/cadencia y
llevar la escopeta hasta nueve perdigones o la energia hasta seis impactos.

| Arma | ID | Dano por proyectil | Disparos/s | Alcance | Velocidad | Descarga |
|---|---|---|---|---|---|---|
| Pistola | 1 | 25 | 2 | 600 px | 800 px/s | Una bala, un impacto |
| Metralleta | 2 | 6 | 8 | 600 px | 800 px/s | Una bala, un impacto |
| Escopeta | 3 | 12 | 0.8 | 300 px | 650 px/s | Cinco perdigones, un impacto cada uno |
| Canon de energia | 4 | 60 | 0.5 | 800 px | 1000 px/s | Un proyectil, hasta tres enemigos distintos |

## Controles de prueba

1 y 2 seleccionan los slots del inventario, tambien con teclado numerico.
Las armas nuevas se adquieren con beneficios en las estaciones;
3 y 4 ya no las habilitan directamente. Ver
[adquisicion y rarezas](guia_adquisicion_rareza_armas.md).
El menu de estacion conserva sus controles 1/2/3 y bloquea cambios de arma.
La rareza pertenece a cada oferta, no al arma de forma permanente.

Cada arma conserva y actualiza su propia recarga aun cuando no esta equipada.
R limpia los disparos, reinicia las cuatro recargas y selecciona el slot 1 actual.
Como en el prototipo anterior, conserva las bonificaciones ya confirmadas.

## Descarga de escopeta

`Arma::intentarDispararRafaga` valida la descarga mediante el disparo central
y consume una sola recarga. Crea cinco copias con desvios de -20, -10, 0,
10 y 20 grados respecto del objetivo automatico. Cada perdigon conserva el
dano, origen, velocidad y alcance propios. No usa cinco llamadas que consuman
cinco recargas. Un objetivo invalido o fuera de alcance no crea perdigones.
Los proyectiles que ya estaban en vuelo se conservan al agregar otra descarga.

La recarga base es de 1.25 segundos. El alcance corto y la dispersion hacen
que a distancia no siempre impacten los cinco perdigones sobre un enemigo.
Los bonos existentes de dano afectan cada perdigon.

## Perforacion del canon

`Proyectil::registrarImpacto` registra un identificador temporal por enemigo.
Devuelve false si ese enemigo ya recibio dano del mismo proyectil. El canon
se desactiva al tercer impacto distinto; las otras armas conservan un impacto.
Los registros se guardan por valor en un vector pequeno administrado por RAII.
No se guardan punteros ni referencias a enemigos.

`Juego` administra un vector paralelo de identificadores temporales para las
instancias de enemigos. Son distintos del ID de catalogo 1/2/3 de Enemigo:
dos enemigos basicos deben poder recibir impactos separados.

Antes de actualizar oleadas se conservan los identificadores de los vivos
en su mismo orden; despues se asignan nuevos identificadores a los enemigos
agregados. Esto depende del comportamiento actual de GestorOleadas: agrega al
final y quita muertos con remove_if sin reordenar sobrevivientes. Si su
responsable cambia esa regla, se debe coordinar esta integracion. Las pruebas
que reemplazan enemigos directamente deben reiniciar la partida antes de
crear un grupo nuevo; cambiar solo posiciones o agregar al final conserva IDs.

Para energia, Juego intercala movimiento y colisiones en subpasos de hasta
8 pixeles. Esto permite detectar objetivos intermedios aunque el tiempo de
actualizacion sea grande. Al agotar alcance no se comprueban impactos nuevos.
Los proyectiles tambien se eliminan al salir de la vista, como anteriormente.
La recompensa se entrega solo cuando el impacto convierte un enemigo vivo en
muerto. No se guarda la partida en disco ni se cambia el formato de archivos.

## Identidad y presentacion

Cada proyectil creado por Arma conserva su ID de arma de origen. Cambiar el
arma equipada no cambia el dibujo ni las propiedades de disparos anteriores.
Los proyectiles antiguos creados sin ID conservan su inferencia visual por
dimensiones para compatibilidad con las pruebas.

Escopeta: textura de acero/cobre y perdigones ambar de 8x8. Canon: textura
con nucleo cian y plasma de 40x10. Pistola y metralleta conservan sus PNG.
Todos apuntan al objetivo automatico usando la misma orientacion que el arma.
La colision sigue siendo rectangular y alineada a los ejes; el giro es visual.

## Archivos y pruebas

Se modifican Arma, Proyectil, Juego, AspectoArmas y AspectoProyectiles,
pertenecientes a combate e integracion. No se modifican Jugador, Enemigo,
GestorOleadas ni el modulo de mejoras.

`pruebas/probar_armas_nuevas.bat` verifica el abanico, cadencia unica, alcance,
impacto de cinco perdigones, perforacion y limite de tres, ausencia de dano
repetido tras compactacion y realocacion de enemigos, recompensas, recarga,
bonos, reinicio, carga de PNG y respaldo sin imagenes. Las pruebas originales de Arma,
Proyectil y Juego siguen comprobando las armas anteriores.
