
package: objs/meval.o ./objs ./lib
	ar -rcs ./lib/libmeval.a ./objs/meval.o

install: /usr/local/lib/meval/libmeval.so /usr/local/include/meval/meval.h

/usr/local/lib/meval/libmeval.so: lib/libmeval.so
	mkdir -p /usr/local/lib/meval/
	cp ./lib/libmeval.so /usr/local/lib/meval/
	chmod 644 /usr/local/lib/meval/libmeval.so

/usr/local/include/meval/meval.h: ./include/meval/meval.h
	mkdir -p /usr/local/include/meval/
	cp ./include/meval/meval.h /usr/local/include/meval/
	chmod 644 /usr/local/include/meval/meval.h

lib/libmeval.so: src/meval.c include/meval/meval.h
	$(CC) -Wall -Wpedantic -O3 -I./include src/meval.c -o lib/libmeval.so -shared

shared: lib/libmeval.so
	$(CC) -Wall -Wpedantic -O3 -I./include src/meval.c -o lib/libmeval.so -shared
	$(CC) -Wall -Wpedantic -O3 ./src/repl.c -o ./bin/meval-shared -Wl,-rpath="$(LIB_DIR)" -I./include -L./lib -lmeval -lm

shared-install: install
	$(CC) -Wall -Wpedantic -Os -s ./src/repl.c -o ./bin/meval-shared -lmeval -lm
	cp ./bin/meval-shared /usr/local/bin/meval
	chmod 755 /usr/local/bin/meval

shared-local: lib/libmeval.so
	$(CC) -Wall -Wpedantic -O3 -I./include src/meval.c -o lib/libmeval.so -shared
	$(CC) -Wall -Wpedantic -O3 ./src/repl.c -o ./bin/meval-shared -Wl,-rpath=./lib -I./include -L./lib -lmeval -lm

objs/meval.o: src/meval.c ./objs
	$(CC) -Wall -Wpedantic -O3 -c -s -I./include src/meval.c -o objs/meval.o

repl: src/repl.c ./bin
	$(CC) ./src/repl.c -g -o bin/meval-repl-db -Wall -Wpedantic -fsanitize=address -DMEVAL_DB_ENABLED -DMEVAL_OPT_ALLOW_MISSING_OPEN_BRACKET=0 src/meval.c -Wall -Wpedantic -I./include -lm
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
