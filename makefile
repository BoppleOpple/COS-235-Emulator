build/emulator: build/intermediate/emulator.o build/intermediate/stringUtils.o build/intermediate/list.o build/intermediate/assembler.o build/intermediate/program.o build/intermediate/printFuncs.o build/intermediate/cache.o
	mkdir -p build
	gcc build/intermediate/emulator.o build/intermediate/stringUtils.o build/intermediate/list.o build/intermediate/assembler.o build/intermediate/program.o build/intermediate/printFuncs.o build/intermediate/cache.o -o build/emulator -Wall

build/intermediate/emulator.o: src/emulator.c
	mkdir -p build/intermediate
	gcc src/emulator.c -c -o build/intermediate/emulator.o -Wall

build/intermediate/printFuncs.o: src/printFuncs.c src/printFuncs.h
	mkdir -p build/intermediate
	gcc src/printFuncs.c -c -o build/intermediate/printFuncs.o -Wall

build/intermediate/cache.o: src/cache.c src/cache.h
	mkdir -p build/intermediate
	gcc src/cache.c -c -o build/intermediate/cache.o -Wall

build/intermediate/program.o: src/program.c src/program.h
	mkdir -p build/intermediate
	gcc src/program.c -c -o build/intermediate/program.o -Wall

build/intermediate/assembler.o: src/assembler.c src/assembler.h
	mkdir -p build/intermediate
	gcc src/assembler.c -c -o build/intermediate/assembler.o -Wall

build/intermediate/stringUtils.o: src/stringUtils.c src/stringUtils.h
	mkdir -p build/intermediate
	gcc src/stringUtils.c -c -o build/intermediate/stringUtils.o -Wall

build/intermediate/list.o: src/list.c src/list.h
	mkdir -p build/intermediate
	gcc src/list.c -c -o build/intermediate/list.o -Wall

clean:
	rm -rf build
