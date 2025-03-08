build/emulator: build/intermediate/emulator.o build/intermediate/stringUtils.o build/intermediate/list.o build/intermediate/assembler.o
	mkdir -p build
	gcc build/intermediate/emulator.o build/intermediate/stringUtils.o build/intermediate/list.o build/intermediate/assembler.o -o build/emulator -Wall

build/intermediate/emulator.o: src/emulator.c
	mkdir -p build/intermediate
	gcc src/emulator.c -c -o build/intermediate/emulator.o -Wall

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
