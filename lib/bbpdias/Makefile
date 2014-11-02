# Set your paths in the file below
include Makefile.paths

CPPDEPS = -MMD -MP -MT '$(basename $@).d' -MT '$(basename $@).o' 

CXXFLAGS := -O3 -ggdb -gdwarf-2 -g3 -Wall -W -frounding-math # -DNDEBUG

#-Wstrict-overflow=5 -Wconversion \
#	-Wlogical-op -Wsign-conversion

CPPFLAGS = $(INCLUDE_PATHS) $(CPPDEPS) -I . -I BBPSDK/include

BBPSDK := BBPSDK/libbbpsdk.a
LIBDIAS := libbbpdias/libbbpdias.a
LIBBTREE := libbtree/libbtree.a

BBPLIBS :=  $(LIBBTREE) $(LIBDIAS) $(BBPSDK)

# TODO: split executables into ones using CGAL and not needing it
LIBS := $(LIB_PATHS) -lspatialindex -lhdf5 $(BOOST_LIBS) # -lCGAL

PROGRAM_SRCS := DumpMBR.cpp Test.cpp DumpSingleMorph.cpp FarhanLoader.cpp FarhanReader.cpp FarhanTest.cpp FarhanTestReader.cpp test-hilbert-resolution.cpp VoronoiTest.cpp delaunay2voronoi.cpp our_approach.cpp delaunay2metis.cpp our1_approach.cpp
# SmallSTRTest.cpp TestRStarTree.cpp

PROGRAM_BINS := $(patsubst %.cpp,%,$(PROGRAM_SRCS))
PROGRAM_OBJS := $(patsubst %.cpp,%.o,$(PROGRAM_SRCS))
PROGRAM_DEPS := $(patsubst %.cpp,%.d,$(PROGRAM_SRCS))

all : $(PROGRAM_BINS)

$(PROGRAM_BINS) : $(BBPLIBS)

$(PROGRAM_BINS): %: %.o
	$(CXX) $< $(BBPLIBS) $(LIBS) -o $@

clean:
	rm -f $(PROGRAM_BINS) $(PROGRAM_OBJS) $(PROGRAM_DEPS)

$(BBPSDK) :
	cd BBPSDK && $(MAKE)

$(LIBDIAS) :
	cd libbbpdias && $(MAKE)

$(LIBBTREE) :
	cd libbtree && $(MAKE)

.PHONY : all clean

-include $(ALLDEPS)
