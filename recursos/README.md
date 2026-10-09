# Recursos graficos

`Tuffy.ttf` se utiliza para mostrar los indicadores numericos de dano.

La fuente fue tomada de los ejemplos oficiales incluidos con SFML. En el
archivo `examples/asset_licenses.md` de esa biblioteca figura como obra de
Thatcher Ulrich distribuida bajo dominio publico CC0.

`entorno/suelo_chatarra_v1.png` es la primera textura del deposito de chatarra
al aire libre: tierra, grava, aceite y pequenos restos mecanicos. Mide 1672 x 941 pixeles. `Escenario` la ajusta a 1920 x 1080
sin suavizado y la dibuja debajo de recursos y entidades. Es un fondo completo,
que se repite como mosaico anclado al mundo al desplazarse la camara,
sin obstaculos ni colisiones nuevas.

## Texturas de proyectiles
Pistola y metralleta utilizan PNG transparentes en recursos/proyectiles. AspectoProyectiles carga, selecciona la region visible y orienta los dibujos; respaldo con rectangulos si faltan archivos. Ver ../documentos/guia_proyectiles_texturas.md para dimensiones, limites y pruebas.
