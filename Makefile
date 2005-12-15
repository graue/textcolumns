.ifdef TCC
CC = tcc
CFLAGS = -Xp -I/usr/include
.else
CC = cc
CFLAGS = -W -Wall -Os
.endif
LDFLAGS = -s
LIBS = -lcurses
OBJS = columns.o game.o screen.o

.PHONY: all clean install

all: columns

columns: $(OBJS)
	$(CC) $(OBJS) $(LIBS) $(LDFLAGS) -o $@

columns.o: columns.c columns.h
	$(CC) $(CFLAGS) -c $< -o $@

game.o: game.c columns.h
	$(CC) $(CFLAGS) -c $< -o $@

screen.o: screen.c columns.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o columns

install: columns
	cp columns /usr/games/columns

#	cp columns.6 /usr/share/man/man6/columns.6
