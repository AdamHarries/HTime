C_SOURCES = resuse.c time.c
OBJECTS = $(patsubst %.c,%.o,$(C_SOURCES))

HS_SOURCES = main.hs

%.o: %.c
	gcc -O3 -c $(patsubst %.o,%.c,$@) 

all: $(OBJECTS) $(HS_SOURCES)
	hsc2hs HTime.hsc
	ghc --make -threaded $(OBJECTS) HTime.hs main.hs

test:$(OBJECTS)
	gcc $(OBJECTS) test.c -o test

clean:
	rm -f main
	rm -f HTime.hs
	rm -f *.o
	rm -f *.hi
