include ../Makefile.paths

CPPFLAGS = $(INCLUDE_PATHS) -MMD -MP -MT '$(basename $@).d' -MT '$(basename $@).o' \
	-I. -I..
CXXFLAGS := -march=core2 -O3 -ggdb -gdwarf-2 -g3 -Wall -Wextra -Winit-self \
	-Wmissing-include-dirs -Wfloat-equal -Wundef -Wcast-qual \
	-Wcast-align -Wlogical-op -Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Woverloaded-virtual -DNDEBUG -flto

LIB := libbtree.a

LIB_SRCS := hilbert_btree_base.cpp unlimited_degree_hilbertbtree.cpp

TEST_SRCS := test.cpp test_unlimited_degree_hilbertbtree.cpp test_limited_degree_hilbertbtree.cpp
TEST_PROGS := test test_unlimited_degree_hilbertbtree test_limited_degree_hilbertbtree # FIXME make from TEST_SRCS

SRC := $(LIB_SRCS) $(TEST_SRCS)

LIB_OBJS := $(patsubst %.cpp,%.o,$(LIB_SRCS))

OBJS := $(patsubst %.cpp,%.o,$(SRC))
DEPS := $(patsubst %.cpp,%.d,$(SRC))

LIBS := -O2 -flto $(LIB_PATHS) -lspatialindex -lhdf5 $(BOOST_LIBS) \
	../libbbpdias/libbbpdias.a

all : $(TEST_PROGS)

test : LDLIBS := $(LIBS)
test : CC := $(CXX)
test : $(LIB)

test_unlimited_degree_hilbertbtree : LDLIBS = $(LIBS)
test_unlimited_degree_hilbertbtree : CC := $(CXX)
test_unlimited_degree_hilbertbtree : $(LIB)

test_limited_degree_hilbertbtree : LDLIBS = $(LIBS)
test_limited_degree_hilbertbtree : CC := $(CXX)
test_limited_degree_hilbertbtree : $(LIB)

$(LIB) : $(LIB_OBJS)
	ar rcu $@ $?

clean:
	rm -f $(OBJS) $(DEPS) $(LIB) $(TEST_PROGS)

.PHONY : all clean

-include $(DEPS)
