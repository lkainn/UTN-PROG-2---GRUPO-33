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
| Enemigos.dat | Estadisticas de las cuatro variantes, incluido el jefe. |
| Partidas.dat | Historial acumulativo de partidas finalizadas. |

Los cuatro archivos se resuelven en datos/ de la raiz, tanto al ejecutar desde
el proyecto como desde bin/Debug. Fuera de esas ubicaciones se utiliza datos/
del directorio de trabajo. La carpeta se crea antes de escribir. Si falta un
catalogo, se generan los valores iniciales; un perfil ausente empieza en
nivel 1, con cero puntos y cero chatarra.

El perfil y las mejoras sobreviven a una nueva partida. La vida se restaura,
la experiencia parcial vuelve a cero y las armas, las ofertas, las oleadas,
los encuentros y los recursos temporales se reinician. La normalizacion del
movimiento, la animacion de Remache y el apuntado se mantienen.

## Controles de prueba

- U compra un nivel permanente de vida por 10 de chatarra, hasta el limite
  del catalogo. Se valida el limite antes de cobrar. Si falla el guardado del
  catalogo se devuelve el costo.
- N restablece el perfil y las mejoras e inicia una partida completamente
  nueva. Conserva el historial de Partidas.dat.
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
directamente, sin pasar por los scripts.

Enemigos.dat se conserva como contenido inicial del juego. Jugadores.dat,
Mejoras.dat y Partidas.dat son archivos locales de progreso y no se versionan.
