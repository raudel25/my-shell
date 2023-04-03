dev:
	mkdir -p build && gcc -o build/my_sh main.c execute.c decod.c builtin.c execute.h decod.h builtin.h list.c list.h glist.c glist.h && cd build && ./my_sh

build:
	mkdir -p build && gcc -o build/my_sh main.c execute.c decod.c builtin.c execute.h decod.h builtin.h list.c list.h glist.c glist.h