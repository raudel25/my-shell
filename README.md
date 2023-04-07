# my-shell

Este proyecto tiene como objetivo la creación de un shell para `Linux` en `c`.

### Dependencias

El poryecto se encuentra desarrollado en c 17, y depende de la biblioteca 
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

Adicionalmente si cuenta con `cmake` puede entnotrar la configuración en el archivo
`CMakeLists.txt`.

Los detalles de las funcionalidades y la ejcución se detallan dentro del propio
programa mediante el comando `help`.