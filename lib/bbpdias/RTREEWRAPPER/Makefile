INCDIRS = -I/home/tauheed/dias/RTREE-ORIG/include -I/opt/boost/include/boost-1_39 -I/opt/BBP-SDK/latest/include \
          -I/opt/python/include/python2.6

CXXFLAGS = -fPIC -O2 -DNDEBUG -g -Wall -frounding-math -fmessage-length=0 ${INCDIRS}

OBJS := SIWrapper.o \
	SI.o \
        bbp_tools.o \
        mbr.o \
	Box.o \
	Vertex.o \
        AABBCylinder.o

LIB_DIRS = -L/home/tauheed/dias/RTREE-ORIG/ -L/opt/BBP-SDK/latest/lib/ -L/opt/lib/

LIBS = -lm -lrt -lboost_python-gcc41-mt-1_39 -lpython2.6 -lBBP-SDK -lspatial

SI.so: $(OBJS)
	g++ -shared  -o SI.so $(OBJS) $(LIB_DIRS) $(LIBS) 

all:    SI.so

clean:
	rm -f $(OBJS) SI.so


