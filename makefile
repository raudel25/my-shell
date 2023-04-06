.PHONY: dev
dev:
	mkdir -p build && gcc -o build/my_sh main.c execute.c decod.c builtin.c execute.h decod.h builtin.h list.c list.h glist.c glist.h -lreadline && cd build && ./my_sh

.PHONY: build
build:
	mkdir -p build && gcc -o build/my_sh main.c execute.c decod.c builtin.c execute.h decod.h builtin.h list.c list.h glist.c glist.h -lreadline

.PHONY: help
help:
	cd help && python3 build_help.py