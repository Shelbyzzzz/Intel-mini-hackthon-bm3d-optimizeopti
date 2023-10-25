# C source code
CSRC	= io_png.c \
		mt19937ar.c
# C++ source code
CXXSRC	= main.cpp \
		bm3d.cpp \
		utilities.cpp \
		lib_transforms.cpp

#LDFLAGS += -L/usr/local/lib -lpng16 -lz -lfftw3
#CFLAGS += -I/usr/local/include -I/usr/include/libpng16
#CFLAGS += -I/usr/include
LDFLAGS += -L/usr/lib/x86_64-linux-gnu -lpng16 -lz -lfftw3f

#compile select 
CC=icpx -fsycl 
CXX=icpx -fsycl 

# all source code
SRC	= $(CSRC) $(CXXSRC)

# C objects
COBJ	= $(CSRC:.c=.o)
# C++ objects
CXXOBJ	= $(CXXSRC:.cpp=.o)
# all objects
OBJ	= $(COBJ) $(CXXOBJ)
# binary target
BIN	= BM3Ddenoising

# C optimization flags
COPT	= -O3 -ftree-vectorize -funroll-loops

# C++ optimization flags
CXXOPT	= $(COPT)

# C compilation flags
CFLAGS	= $(COPT) -Wall -Wextra \
	-Wno-write-strings -ansi -O3 -std=c++17 -I/home/u205993/fftw/include -L/home/u205993/fftw/lib -lfftw3f
# C++ compilation flags
CXXFLAGS	= $(CXXOPT) -Wall -Wextra \
	-Wno-write-strings -Wno-deprecated -ansi -O3 -std=c++17 -I/home/u205993/fftw/include -L/home/u205993/fftw/lib -lfftw3f
# link flags
LDFLAGS	= -lpng -I/home/u205993/fftw/include -L/home/u205993/fftw/lib -lfftw3f

# use openMP with `make OMP=1`
ifdef OMP
CFLAGS	+= -fopenmp
CXXFLAGS	+= -fopenmp
LDFLAGS += -lgomp
else
CFLAGS	+= -Wno-unknown-pragmas
CXXFLAGS  += -Wno-unknown-pragmas
endif

# partial compilation of C source code
%.o: %.c %.h
	$(CC) -c -o $@  $< $(CFLAGS)
# partial compilation of C++ source code
%.o: %.cpp %.h
	$(CXX) -c -o $@  $< $(CXXFLAGS)

# link all the object code
$(BIN): $(OBJ) $(LIBDEPS)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)