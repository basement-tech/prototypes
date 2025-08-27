CC = gcc
CFLAGS = -v

SRC = neofilecreator.c
OUT = neofilecreator.exe

OBJ = $(SRC:.c=.o) $(LIB)


all: $(OUT)

neofilecreator.o: neofilecreator.c
	cc -c neofilecreator.c -o neofilecreator.o

neofilecreator.exe: neofilecreator.o
	cc -o neofilecreator.exe neofilecreator.o

clean:
	rm -f *.o *.exe