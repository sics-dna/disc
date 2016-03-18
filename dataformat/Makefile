# Makefile

ifdef OPT
OFLAGS = -O3
else
# Mac OS: uncomment if problems with linking libraries not compiled for 64 bits architecture
#OFLAGS = -m32 -g
OFLAGS = -m64 -g
endif

CXX = ./our-g++
# CXXFLAGS = $(OFLAGS) -DGZLIB -Wall $(PG) ${DEFINES}
CXXFLAGS = $(OFLAGS) -Wall ${DEFINES}
CC = gcc
CFLAGS = $(OFLAGS)  -DGZLIB -Wall ${DEFINES}

OBJ1 = readtokens.o table.o
OBJ2 = format.o formatdispatch.o formatbinary.o formatdiscr.o formatcont.o formatsymbol.o formattime.o formatunknown.o
OBJ3 = data.o datafile.o datadispatch.o

all: libdataformat.a

makeformat: makeformat.o ${OBJ1} ${OBJ2} ${OBJ3}

libdataformat.a: ${OBJ1} ${OBJ2} ${OBJ3}
	rm -f libdataformat.a
	ar clq libdataformat.a $^
	ranlib libdataformat.a

clean:
	rm -f ${OBJ1} ${OBJ2} ${OBJ3}

depend: 
	rm -f make.depend
	${CC} -MM *.cc ${CFLAGS} > make.depend

include make.depend
