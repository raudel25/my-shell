dev:
	gcc -o myshell main.c execute.c decod.c && ./myshell

build:
	gcc -o myshell main.c execute.c decod.c