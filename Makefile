#compiler
CXX=g++

#compiler flags
COPTI= -O3
CDEBG= -g -O0
CWARN= -Wall -Wextra
CBAMH= -D_LIBBAM
CSHV1=
CVERS= -std=c++11

#linker flags
LOPTI= -O3
LDEBG= -g -O0
LSTDD= -lm -lhts -lpthread
LSTDS= -Wl,-Bstatic -Wl,-Bdynamic -lm -lhts -lpthread
LCL3S= -Wl,-Bstatic -Wl,-Bdynamic -lm -lhts -lpthread
LBAMD= -lbam -lz

#executable file
EFILE= bin/vcfPL2gen

#header files
HFILE= $(shell find src -name *.h)

#source files
CFILE= $(shell find src -name *.cpp)

#source path
VPATH= $(shell for file in `find src -name *.cpp`; do echo $$(dirname $$file); done)

#include path
ISTDP= -Isrc
IBAMP= -Ilib
ICL3P= -I/users/delaneau/BOOST/include

#library path
LSTDP= -Llib

#object files
OFILE= $(shell for file in `find src -name *.cpp`; do echo obj/$$(basename $$file .cpp).o; done)
OBOST=

#default
all: dynamic

#dynamic release
dynamic: CFLAG=$(COPTI) $(CWARN) $(CSHV1) $(CVERS)
dynamic: LFLAG=$(LOPTI) $(LSTDD)
dynamic: IFLAG=$(ISTDP)
dynamic: $(EFILE)

#static release
static: CFLAG=$(COPTI) $(CWARN) $(CSHV1) $(CVERS)
static: LFLAG=$(LOPTI) $(LSTDS)
static: IFLAG=$(ISTDP)
static: $(EFILE)

#cluster release
cluster: CFLAG=$(COPTI) $(CWARN) $(CSHV1) $(CVERS)
cluster: LFLAG=$(LOPTI) $(LCL3S)
cluster: IFLAG=$(ISTDP) $(ICL3P)
cluster: OBOST=~/BOOST/lib/libboost_iostreams.a ~/BOOST/lib/libboost_program_options.a
cluster: $(EFILE)

debug: CFLAG=$(CWARN) $(CSHV1) $(CVERS) $(CDEBG)
debug: LFLAG=$(LSTDD) $(LDEBG)
debug: IFLAG=$(ISTDP)
debug: $(EFILE)


$(EFILE): $(OFILE)
	$(CXX) $^ $(OBOST) -o $@ $(LFLAG)

obj/%.o: %.cpp $(HFILE)
	$(CXX) -o $@ -c $< $(CFLAG) $(IFLAG)

clean: 
	rm -f obj/*.o $(EFILE)

test:
	cp $(EFILE) ~/$(EFILE).v10

oxford:
	cp $(EFILE) ~/bin/.

install:
	cp $(EFILE) /usr/local/bin/.
