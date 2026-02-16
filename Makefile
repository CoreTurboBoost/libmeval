
package: objs/meval.o ./objs ./lib
	ar -rcs ./lib/libmeval.a ./objs/meval.o

shared:
	$(CC) -Wall -Wpedantic -O3 -I./include src/meval.c -o lib/libmeval.so -shared
	$(CC) -Wall -Wpedantic -O3 ./src/repl.c -o ./bin/meval-shared -Wl,-rpath="$(LIB_DIR)" -I./include -L./lib -lmeval -lm

shared-install:
	$(CC) -Wall -Wpedantic -O3 -I./include src/meval.c -o lib/libmeval.so -shared
	mkdir -p /usr/local/lib/meval/
	cp ./lib/libmeval.so /usr/local/lib/meval/
	$(CC) -Wall -Wpedantic -O3 ./src/repl.c -o ./bin/meval-shared -Wl,-rpath=/usr/local/lib/meval -I./include -L./lib -lmeval -lm

shared-local:
	$(CC) -Wall -Wpedantic -O3 -I./include src/meval.c -o lib/libmeval.so -shared
	$(CC) -Wall -Wpedantic -O3 ./src/repl.c -o ./bin/meval-shared -Wl,-rpath=./lib -I./include -L./lib -lmeval -lm

objs/meval.o: src/meval.c ./objs
	$(CC) -Wall -Wpedantic -O3 -c -s -I./include src/meval.c -o objs/meval.o

repl: src/repl.c ./bin
	$(CC) ./src/repl.c -g -o bin/meval-repl-db -Wall -Wpedantic -fsanitize=address -DMEVAL_DB_ENABLED src/meval.c -Wall -Wpedantic -I./include -lm
repl-rel: src/repl.c ./bin
	$(CC) ./src/repl.c -s -O3 -o bin/meval-repl -Wall -Wpedantic -fsanitize=address src/meval.c -Wall -Wpedantic -I./include -lm
repl-rel-static: src/repl.c ./bin
	$(CC) -static ./src/repl.c -s -O3 -o bin/meval-repl-static -Wall -Wpedantic src/meval.c -Wall -Wpedantic -I./include -lm

clean:
	rm ./objs/*.o
	rm ./lib/*.a
	rm ./bin/*

./objs:
	mkdir objs
./lib:
	mkdir lib
./bin:
	mkdir bin

.PHONY: clean package repl repl-rel repl-rel-static 
