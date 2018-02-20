CXX=g++
CXXFLAGS = -Wall -Wno-unused -g -I../dataformat
CC = $(CXX)
CFLAGS = $(CXXFLAGS)

OBJS = anomalydetector.o isc_mixture.o isc_component.o isc_micromodel_poissongamma.o isc_micromodel_gaussian.o isc_micromodel_multigaussian.o isc_micromodel_markovgaussian.o hmatrix.o gamma.o hgf.o  

all 	: libisc.a

#----------------------------------------------------------------------

libisc.a: $(OBJS)
	rm -f libisc.a
	ar clq libisc.a $^
	ranlib libisc.a

clean:
	rm -f $(OBJS) libisc.a
