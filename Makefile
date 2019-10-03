
OBJ = fftcp.o ffsvr.o ffevent.o ffclt.o ffstr.o ffhttp.o
PROGNAME = ffsvr

all: ffsvr

fftcp.o: fftcp.c fftcp.h
ffhttp.o: ffhttp.c ffhttp.h ffstr.h
ffsvr.o: ffsvr.c fftcp.h ffevent.h ffclt.h ffstr.h ffhttp.h
ffclt.o: ffclt.c ffclt.h ffstr.h ffhttp.h
ffevent.o: ffevent.c ffevent.h
ffstr.o: ffstr.c ffstr.h

ffsvr: $(OBJ)
	gcc -o $(PROGNAME) $(CFLAGS) $(OBJ) -g 

.c.o:
	gcc -c -g $<

.PHONY: clean
clean:
	rm ffsvr *.o
	rm -rf *.dSYM
