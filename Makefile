all: monte dis2gp molitest libs pearson
libs: libsample.so libsample2.so libsample3.so
maclibs: libsample.dylib libsample2.dylib libsample3.dylib

CC=gcc
MCC=gcc
#MCC=mpicc -I/opt/lam/include -DUSE_MMPI -DUSE_MPI

LD=ld -shared
MLD=gcc -dynamiclib

# For MINGW: 
#CC=/usr/bin/i586-mingw32msvc-gcc -DUSE_WIN32
#MCC=/usr/bin/i586-mingw32msvc-gcc -DUSE_WIN32
#LIBS=$(RNG_LIBS) -lm # for Win32

# For openwrt-xburst:
#CC=/home/jirka/src/qi/openwrt-xburst/staging_dir/toolchain-mipsel_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin/mipsel-openwrt-linux-uclibc-gcc
#MCC=/home/jirka/src/qi/openwrt-xburst/staging_dir/toolchain-mipsel_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin/mipsel-openwrt-linux-uclibc-gcc
#LD=/home/jirka/src/qi/openwrt-xburst/staging_dir/toolchain-mipsel_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin/mipsel-openwrt-linux-uclibc-ld -shared

RNG=
#RNG=-DUSE_SPRNG1 -DUSE_SPRNG -I/usr/include/sprng -I/opt/sprng/include -I../opt/sprng/include
#RNG=-DUSE_SPRNG2  #-DUSE_SPRNG #-I/opt/sprng/include
#RNG=-DUSE_SPRNG2  -I/usr/include/sprng -DUSE_SPRNG -I/opt/sprng/include
#RNG=-DUSE_SPRNG2  -DUSE_SPRNG -I/opt/sprng/include

#DEBUG=-g -O0 -DDEVEL_VERBOSE
DEBUG=-O3 

CFLAGS=-Wall -ansi -pedantic -DREPLACE_RINTL -DUSE_LSHARED $(RNG) $(DEBUG) -DPSGUI 

RNG_LIBS=
#RNG_LIBS=-L/opt/sprng/lib -L../opt/sprng/lib -llcg64
#RNG_LIBS=-L/opt/sprng/lib -lsprng
#RNG_LIBS=-lsprng

LIBS=$(RNG_LIBS) -lm -ldl 
#LIBS=$(RNG_LIBS) -lm -ldl -lgmp
#LIBS=$(RNG_LIBS) -L/opt/lam/lib -lmpi -lgmp -lm #-L/usr/local/lib -lefence

OBJECTS=monte.o knuth.o hisops.o cparam.o finput.o simul.o mgraph.o mgfx_ps.o mpifunc.o fem_math.o fem_mem.o correl.o eqs.o
LOBJECTS=lsample.o
LOBJECTS2=lsample2.o
LOBJECTS3=lsample3.o
LOBJECTS4=lsample4.o

monte: $(OBJECTS)
	$(MCC) $(CFLAGS) -o $(@) $(OBJECTS) $(LIBS)

monte.o: monte.c monte.h
	$(MCC) -c monte.c $(CFLAGS)

knuth.o: knuth.c monte.h
	$(MCC) -c knuth.c $(CFLAGS)

hisops.o: hisops.c monte.h
	$(MCC) -c hisops.c $(CFLAGS)

cparam.o: cparam.c monte.h
	$(MCC) -c cparam.c $(CFLAGS)

finput.o: finput.c monte.h
	$(MCC) -c finput.c $(CFLAGS)

simul.o: simul.c monte.h
	$(MCC) -c simul.c $(CFLAGS)

mgraph.o: mgraph.c monte.h
	$(MCC) -c mgraph.c $(CFLAGS)

mgfx_ps.o: mgfx_ps.c monte.h
	$(MCC) -c mgfx_ps.c $(CFLAGS)

mpifunc.o: mpifunc.c monte.h
	$(MCC) -c mpifunc.c $(CFLAGS)

correl.o: correl.c monte.h fem_math.o eqs.o
	$(MCC) -c correl.c $(CFLAGS)

eqs.o: eqs.c monte.h fem_math.o
	$(MCC) -c eqs.c $(CFLAGS)

test_corr: test_corr.c monte.h fem_math.o fem_mem.o correl.o eqs.o finput.o hisops.o knuth.o
	$(MCC) -o $(@) fem_math.o fem_mem.o correl.o test_corr.c eqs.o finput.o hisops.o knuth.o  $(CFLAGS) $(LIBS)

# library functions from the uFEM:

fem_mem.o: fem_mem.c fem.h fem_mem.h
	$(MCC) -c fem_mem.c $(CFLAGS)

fem_math.o: fem_math.c fem.h fem_mem.h fem_math.h
	$(MCC) -c fem_math.c $(CFLAGS)

# utilities: 

dis2gp: dis2gp.c hisops.o finput.o simul.o knuth.o correl.o fem_mem.o fem_math.o eqs.o
	$(CC) -o $(@) dis2gp.c hisops.o finput.o simul.o knuth.o correl.o fem_mem.o fem_math.o eqs.o $(CFLAGS) $(LIBS)

molitest: molitest.c hisops.o finput.o simul.o knuth.o correl.o fem_mem.o fem_math.o eqs.o
	$(CC) -o $(@) molitest.c hisops.o finput.o simul.o knuth.o correl.o fem_mem.o fem_math.o eqs.o $(CFLAGS) $(LIBS)

pearson: pearson.c
	$(CC) $(CFLAGS) pearson.c -o $(@) -lm

# libraries:
# 
lsample.o: lsample.c
	$(CC) -c -fPIC -rdynamic lsample.c $(CFLAGS)

lsample2.o: lsample2.c
	$(CC) -c -fPIC -rdynamic lsample2.c $(CFLAGS)

lsample3.o: lsample3.c
	$(CC) -c -fPIC -rdynamic lsample3.c $(CFLAGS)

lsample4.o: lsample4.c
	$(CC) -c -fPIC -rdynamic lsample4.c $(CFLAGS)


libsample.so: $(LOBJECTS)
	$(LD) $(LOBJECTS) -o $(@)

libsample2.so: $(LOBJECTS2)
	$(LD) $(LOBJECTS2) -o $(@)

libsample3.so: $(LOBJECTS3)
	$(LD) $(LOBJECTS3) -o $(@)

libsample4.so: $(LOBJECTS4)
	$(LD) $(LOBJECTS4) -o $(@)


# Mac OS X (Darwin) shared libraries:

libsample.dylib: $(LOBJECTS)
	$(MLD) $(LOBJECTS) -o $(@)

libsample2.dylib: $(LOBJECTS2)
	$(MLD) $(LOBJECTS2) -o $(@)

libsample3.dylib: $(LOBJECTS3)
	$(MLD) $(LOBJECTS3) -o $(@)

libsample4.dylib: $(LOBJECTS4)
	$(MLD) $(LOBJECTS4) -o $(@)

libfemcall.dylib: $(LOBJECTS5)
	$(MLD) $(LOBJECTS5) -o $(@)


clean:
	rm -f *.o core monte molitest test_corr dis2gp pearson *.so *.tk so_locations sample3-?.dis *.stt *.sim
