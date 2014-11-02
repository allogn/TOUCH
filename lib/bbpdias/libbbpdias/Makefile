include ../Makefile.paths

CPPFLAGS = $(INCLUDE_PATHS) -MMD -MP -MT '$(basename $@).d' -MT '$(basename $@).o' \
	 -I.. -isystem ../BBPSDK/include -DNDEBUG
CXXFLAGS := -march=core2 -O3 -ggdb -Wall -Wextra -Winit-self \
	-Wmissing-include-dirs -Wfloat-equal -Wundef -Wcast-qual \
	-Wcast-align -Wlogical-op -Wmissing-declarations \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Woverloaded-virtual


LIB := libbbpdias.a

SRC := AABBCylinder.cpp bbp_tools.cpp circuit_loader.cpp spatialindex_tools.cpp \
	mbr.cpp hilbert.cpp Box.cpp Vertex.cpp cgal_tools.cpp

OBJS := $(patsubst %.cpp,%.o,$(SRC))
DEPS := $(patsubst %.cpp,%.d,$(SRC))

all : $(LIB)

$(LIB) : $(OBJS)
	ar rcu $@ $?

clean :
	-rm $(OBJS) $(DEPS) $(LIB)

.PHONY : all clean

-include $(DEPS)
