build/emulator: build/intermediate/emulator.o build/intermediate/stringUtils.o build/intermediate/list.o build/intermediate/assembler.o build/intermediate/program.o
	mkdir -p build
	gcc build/intermediate/emulator.o build/intermediate/stringUtils.o build/intermediate/list.o build/intermediate/assembler.o build/intermediate/program.o -o build/emulator -Wall -g

build/intermediate/emulator.o: src/emulator.c
	mkdir -p build/intermediate
	gcc src/emulator.c -c -o build/intermediate/emulator.o -Wall -g

build/intermediate/program.o: src/program.c src/program.h
	mkdir -p build/intermediate
	gcc src/program.c -c -o build/intermediate/program.o -Wall -g

build/intermediate/assembler.o: src/assembler.c src/assembler.h
	mkdir -p build/intermediate
	gcc src/assembler.c -c -o build/intermediate/assembler.o -Wall -g

build/intermediate/stringUtils.o: src/stringUtils.c src/stringUtils.h
	mkdir -p build/intermediate
	gcc src/stringUtils.c -c -o build/intermediate/stringUtils.o -Wall -g

build/intermediate/list.o: src/list.c src/list.h
	mkdir -p build/intermediate
	gcc src/list.c -c -o build/intermediate/list.o -Wall -g

clean:
	rm -rf build
