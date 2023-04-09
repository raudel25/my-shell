.PHONY: dev
dev:
	mkdir -p build && gcc -o build/my_sh src/main.c src/list.c src/glist.c src/execute.c src/decode.c src/builtin.c -lreadline && cd build && ./my_sh

.PHONY: build
build:
	mkdir -p build && gcc -o build/my_sh src/main.c src/list.c src/glist.c src/execute.c src/decode.c src/builtin.c -lreadline

.PHONY: help
help:
	cd help && python3 build_help.py