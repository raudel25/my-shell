dev:
	gcc -o my_shell main.c execute.c decod.c builtin.c execute.h decod.h builtin.h list.c list.h glist.c glist.h && ./my_shell

build:
	gcc -o my_shell main.c main.c execute.c decod.c builtin.c execute.h decod.h builtin.h list.c list.h glist.c glist.h