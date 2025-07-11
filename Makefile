
package: objs/meval.o
	ar -rcs ./lib/libmeval.a ./objs/meval.o

objs/meval.o: src/meval.c
	$(CC) -Wall -Wpedantic -O3 -c -s -I./include src/meval.c -o objs/meval.o

repl: src/repl.c
	$(CC) ./src/repl.c -g -o meval-repl-db -Wall -Wpedantic -fsanitize=address -DMEVAL_DB_ENABLED src/meval.c -Wall -Wpedantic -I./include -lm
repl-rel: src/repl.c
	$(CC) ./src/repl.c -g -o meval-repl -Wall -Wpedantic -fsanitize=address src/meval.c -Wall -Wpedantic -I./include -lm

clean:
	rm ./objs/*.o
	rm ./lib/*.a

./objs:
	mkdir objs
./lib:
	mkdir lib

.PHONY: clean package
