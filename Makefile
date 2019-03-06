
OBJ = fftcp.o ffsvr.o ffevent.o ffclt.o
PROGNAME = ffsvr

all: ffsvr

fftcp.o: fftcp.c fftcp.h
ffsvr.o: ffsvr.c fftcp.h ffevent.h ffclt.h
ffclt.o: ffclt.c ffclt.h
ffevent.o: ffevent.c ffevent.h

ffsvr: $(OBJ)
	gcc -o $(PROGNAME) $(OBJ) -g

.c.o:
	gcc -c -g $<

.PHONY: clean
clean:
	rm ffsvr *.o
	rm -rf *.dSYM
