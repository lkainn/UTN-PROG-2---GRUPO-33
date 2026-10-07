# Adquisición y mejoras de armas por rareza

## Inventario y adquisición

Hay dos slots. Al iniciar, el primero contiene la pistola y el segundo está
vacío. Elegir un arma nueva incorpora el arma con todos los beneficios
indicados en su tarjeta.

| Estado | Resultado de elegir un arma distinta |
|---|---|
| Slot 2 vacío | Ocupa el slot 2, que queda fijo |
| Slot 2 ocupado y pistola en slot 1 | Puede reemplazar solamente la pistola |
| Pistola reemplazada | Ambos slots fijos; solo ofertas de armas poseídas |

Una oferta del arma que ya se posee mejora ese ejemplar, sin ocupar otro slot.
No se poseen duplicados. La rareza pertenece a la oferta: un arma adquirida
con una común puede recibir después una mejora legendaria.

El reemplazo requiere otra confirmación. La pantalla detalla los porcentajes
y proyectiles extra que se perderán con la pistola. Confirmar fija el slot 1,
conserva el slot 2 y elimina los bonos de la pistola. Volver a las tarjetas
conserva el inventario y las ofertas. La pistola descartada no vuelve a ofrecerse.

## Rarezas y beneficios

Cada opción sortea su propia rareza. Los tipos de arma elegibles se mezclan
con igual oportunidad: inicialmente hay tres tipos distintos entre los cuatro
disponibles. Con dos slots fijos solo hay dos tipos elegibles, por lo que una
misma arma puede aparecer en más de una tarjeta.

| Rareza | Peso de aparición | Daño base | Cadencia base | Especial |
|---|---:|---:|---:|---|
| Común | 60% | +5% | +4% | No |
| Rara | 25% | +10% | +8% | 15% de posibilidad |
| Épica | 12% | +18% | +12% | 50% de posibilidad |
| Legendaria | 3% | +25% | +18% | Garantizado |

En rara o épica, el especial sustituye el aumento de cadencia. En legendaria,
se entregan daño, cadencia y especial juntos. La tarjeta muestra los beneficios
efectivamente sorteados. Los porcentajes se suman sobre las bases: dos comunes
dan +10% de daño y +8% de cadencia; no multiplican dos veces el valor mejorado.
El daño entero redondea hacia arriba y los acumuladores saturan en `INT_MAX`.

| Arma | Especial de una oferta | Límite total |
|---|---|---:|
| Pistola | +1 proyectil por descarga | 5 proyectiles |
| Metralleta | +1 proyectil por descarga | 5 proyectiles |
| Escopeta | +1 perdigón por descarga | 9 perdigones |
| Cañón de energía | +1 enemigo distinto atravesado | 6 impactos |

Pistola y metralleta usan un abanico de 10 grados con proyectiles adicionales;
la escopeta conserva el de 40 grados. Cada descarga consume una sola recarga.
El cañón conserva un proyectil y no repite daño sobre el mismo enemigo.

Al llegar al límite especial se excluye legendaria para esa arma. Se sortean
común/rara/épica con pesos 60/25/12 sobre un total de 97, sin especiales. Así
no se promete una legendaria con un beneficio vacío.

## Estaciones, precios y controles

F interactúa con la estación o cofre disponible más cercano dentro de 100 px.
Abrir una estación cobra una vez y pausa el combate. Hay tres tarjetas con
nombre, rareza, beneficios, imagen y destino de la compra. Los colores
distinguen las rarezas. La oferta comprada se equipa automáticamente.

Elegir consume esa estación y aumenta el siguiente precio un 60%, redondeando
hacia arriba: 10, 16, 26, 42, 68… No hay recargo por rareza en esta versión.
Cancelar devuelve el pago, sin consumir la estación ni incrementar el precio.

Las ofertas se conservan por zona durante la ejecución. Cancelar y reabrir no
vuelve a sortear. Si otras compras cambian el inventario o agotan un especial,
se regeneran únicamente las ofertas guardadas que ya no son válidas. Hay una
estación posible por zona de 1200 px; las claves usan coordenadas enteras
obtenidas con `floor`, también para las zonas negativas.

| Contexto | Control |
|---|---|
| Combate | 1 / 2 seleccionan slots, también con teclado numérico |
| Estación | 1 / 2 / 3 eligen una oferta |
| Confirmación de reemplazo | 1 confirma; 2 vuelve a las mismas tarjetas |
| Menú abierto | Escape cancela y devuelve el pago sin reiniciar |
| Reinicio de prueba | R devuelve el pago pendiente y reinicia la simulación |
| Fuera del menú | Escape cierra la ventana |

3 y 4 ya no habilitan armas sin adquirirlas. Solo dispara el arma activa;
el disparo simultáneo de las dos armas queda para la siguiente etapa.

R conserva el comportamiento del reinicio de prueba: mantiene inventario,
bonos, progresión del personaje, precios y encuentros usados; limpia enemigos
y disparos, reinicia recargas y selecciona el slot 1 actual. No restaura la
pistola descartada. Cerrar y ejecutar de nuevo inicia un estado nuevo.
Todavía no se guardan estos datos en archivos binarios.

## Cofres para el personaje

Los cofres no entregan armas, daño ni cadencia. Otorgan blindaje (+20 vida
máxima y actual) o experiencia (+40), con igual probabilidad cuando ambos
beneficios son elegibles. La experiencia puede activar la subida de nivel
existente. Se usan `Jugador::aumentarVidaMaxima` y `ganarExperiencia`, sin
editar Jugador ni agregar atributos que su interfaz todavía no expone.

Hay guardas para evitar aumentos que puedan desbordar enteros. Si experiencia
no es elegible se entrega blindaje; si no puede aumentarse la vida máxima,
no se cobra el cofre. Los precios independientes siguen en 15, 24, 39, 63,
101… Cada cofre se abre una sola vez.

## Responsabilidades y memoria

- `InventarioArmas.h/.cpp`: slots, bonos por ID, validación, adquisición y
  reemplazo. Usa `std::array` por valor, sin reservas manuales.
- `EstacionMejoras.h/.cpp`: ofertas, sorteo, descripción, menú, precio y caché.
  El `std::map` administra su memoria automáticamente, crece según estaciones
  visitadas y se libera al destruir el objeto.
- `Arma.h/.cpp`: porcentajes sobre base y especiales. Juego utiliza
  `configurarMejoras` con los porcentajes reales del inventario.
- `Proyectil.cpp`: permite hasta seis impactos. Daño, identidad y límite de
  impacto se fijan al crearlo: los disparos que están en vuelo no cambian al
  mejorar o reemplazar el arma. El vector de identificadores usa RAII.
- `Juego.h/.cpp`: pago, confirmación, selección por slot, sincronización de
  bonos, cofres y presentación. Conserva cuatro objetos Arma como bases del
  catálogo; el inventario determina cuáles se pueden equipar.
