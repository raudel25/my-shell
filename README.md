# my-shell

Este proyecto tiene como objetivo la creación de un shell para `Linux` en `c`.

### Dependencias

El proyecto se encuentra desarrollado en `c 17`, y depende de la biblioteca
<a href="https://tiswww.case.edu/php/chet/readline/rltop.html">readline</a>.
Para instalarla debe seguir los siguientes pasos.

Si está utilizando Ubuntu o Debian, puede instalar la biblioteca readline con el siguiente comando en la terminal:

```
sudo apt-get install libreadline-dev
```

Si está utilizando una distribución basada en Red Hat, como Fedora o CentOS, puede instalar la biblioteca readline con el siguiente comando en la terminal:

```
sudo dnf install readline-devel
```

Si está utilizando una distribución basada en Arch Linux, como Manjaro o Antergos, puede instalar la biblioteca readline con el siguiente comando en la terminal:

```
sudo pacman -S readline
```

### Ejecutar el Proyecto

Para ejecutarlo de contar en su pc con el compilador de `c`, y ejecutar en su
terminal el siguiente comando:

```
make
```

Adicionalmente si cuenta con `cmake` puede encontrar la configuración en el archivo
`CMakeLists.txt`.

### Funcionalidades:

- basic: funcionalidades básicas
- pipes: implementación de múltiples tuberías
- background: permite correr procesos en el background
- spaces: los comandos pueden estar separados por cualquier cantidad de espacios
- history: se almacena un historial de comandos
- ctrl+c: finaliza el proceso actual
- chain: permite ejecutar múltiples comandos en una sola línea y comandos de forma condicional
- conditional: permite ejecutar comandos de forma condicional
- variables: permite almacenar variables
- format: permite dar un formato específico al comando introducido por el usuario (comillas y paréntesis)

### Comandos:

- cd: cambia de directorio
- exit: finaliza la ejecución del shell
- fg: trae hacia el foreground el último proceso enviado al background
- jobs: lista todos los procesos en el background
- history: muestra el historial de comandos
- again: ejecuta un comando almacenado en el historial
- true: representa un comando que siempre se ejecuta con éxito
- false: representa un comando que nunca se ejecuta con éxito
- get: muestra el valor de las variables
- set: modifica el valor de una variable
- unset: elimina una variable

Los detalles de las funcionalidades y la ejecución se detallan dentro del propio
programa mediante el comando `help`.
