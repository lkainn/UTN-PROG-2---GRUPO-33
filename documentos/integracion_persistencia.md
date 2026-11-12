# Persistencia integrada

Los aportes de Nicolas (82704d1 y 08e4b93) incorporan ArchivoJugadores,
ArchivoMejoras, la carga de progreso y las mejoras permanentes. Los aportes de
Ramiro (bcf1fe1 y 090c9de) incorporan ArchivoEnemigos, ArchivoPartidas y el uso
del catalogo por GestorOleadas. Se conserva el formato binario de esas ramas.

## Datos

| Archivo | Contenido y uso |
| --- | --- |
| Jugadores.dat | Perfil unico: nivel maximo, mejor puntaje y chatarra disponible. |
| Mejoras.dat | Catalogo de mejoras y niveles adquiridos; se aplica al construir Jugador. |
| Enemigos.dat | Estadisticas, recompensas, dimensiones de colision y baja logica de las cuatro variantes. |
| Partidas.dat | Historial acumulativo de partidas finalizadas. |
| Armas.dat | Catalogo de las cuatro armas: ID, nombre, dano, cadencia, alcance, nivel maximo y baja logica. |
| PartidaEnCurso.dat | Estado de combate para reanudar desde el menu principal. |

Los archivos se resuelven en datos/ de la raiz, tanto al ejecutar desde
el proyecto como desde bin/Debug. Fuera de esas ubicaciones se utiliza datos/
del directorio de trabajo. La carpeta se crea antes de escribir. Un perfil
ausente empieza en nivel 1, con cero puntos y cero chatarra.

Armas.dat y Enemigos.dat se incluyen con el proyecto y son la unica fuente
de estadisticas base. Si falta un catalogo o tiene datos invalidos, el juego
informa el error en la consola y termina. El registro de la pistola debe
estar activo porque es el arma inicial. Las otras armas dadas de baja
quedan fuera de las ofertas de las estaciones. El inventario, las
bonificaciones de rareza, las recargas y los disparos se guardan con la
partida en curso.

El gestor carga Enemigos.dat al iniciar y al reiniciar la partida. Los
enemigos rapidos y resistentes dados de baja quedan fuera de las oleadas.
El basico y el jefe deben estar activos. Se aceptan los catalogos del
formato anterior con todas las variantes activas y se escriben en el
formato nuevo al guardar.

El perfil y las mejoras sobreviven al reintentar desde la pantalla de resultado. La vida se restaura,
la experiencia parcial vuelve a cero y las armas, las ofertas, las oleadas,
los encuentros y los recursos temporales se reinician. La normalizacion del
movimiento, la animacion de Remache y el apuntado se mantienen.

## Controles de prueba

- U compra un nivel permanente de vida por 10 de chatarra, hasta el limite
  del catalogo. Se valida el limite antes de cobrar. Si falla el guardado del
  catalogo se devuelve el costo.
- Nueva partida restablece el perfil y las mejoras tras confirmar. Si hay
  una partida guardada, la descarta. El historial se conserva.
- R mantiene el reinicio rapido de la prueba; no borra el progreso.

Al ganar o perder, Juego guarda el resultado una sola vez, con su identificador,
y actualiza el mejor puntaje del perfil. La pantalla de resultado informa si el
historial pudo guardarse. Ganar o gastar chatarra no borra el record de puntaje.

## Validacion y pruebas

Los lectores verifican registros completos, rangos y terminadores de cadenas.
El catalogo de enemigos rechaza dimensiones o velocidades no finitas. Un
historial con bytes incompletos al final no admite nuevas escrituras hasta
corregirse, para evitar registros desalineados. No se cambia el formato de los
registros existentes ni se serializan contenedores o punteros.

Ejecutar pruebas/probar_persistencia.bat y pruebas/probar_partida_integrada.bat.
Los scripts de pruebas crean un directorio nuevo en bin/Debug/datos_pruebas
mediante TPI_CARPETA_DATOS, de modo que las pruebas no alteran el progreso real.
La variable tambien permite elegir una carpeta explicita para otros ensayos.
Los ejecutables de prueba tambien crean un entorno separado si se abren
directamente, sin pasar por los scripts. Copian los dos catalogos incluidos
para poder modificarlos sin tocar los del juego.

Jugadores.dat, Mejoras.dat, Partidas.dat y PartidaEnCurso.dat son archivos
locales de progreso.
Armas.dat se verifica con pruebas/probar_armas_nuevas.bat.
