
OBJ = fftcp.o ffsvr.o
PROGNAME = ffsvr

all: ffsvr

fftcp.o: fftcp.c fftcp.h
ffsvr.o: ffsvr.c fftcp.h
ffevent.o: ffevent.c ffevent.h

ffsvr: $(OBJ)
	gcc -o $(PROGNAME) $(OBJ) -g

.c.o:
	gcc -c -g $<

.PHONY: clean
clean:
	rm ffsvr *.o
	rm -rf *.dSYM
