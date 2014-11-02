INCDIRS       := -IC:\Users\farhan\Desktop\Workspace\BOOST\include \
			     -IC:\Users\farhan\Desktop\Workspace\HDF5\include \
			     -IC:\Users\farhan\Desktop\Workspace\BBPDIAS\BBPSDK\include \
			     -IC:\Users\farhan\Desktop\Workspace\RTREE-ORIG\include
			     
LIB_PATHS     := -LC:\Users\farhan\Desktop\Workspace\HDF5\lib \
                 -LC:\Users\farhan\Desktop\Workspace\BOOST\lib \
                 -LC:\Users\farhan\Desktop\Workspace\BBPDIAS\BBPSDK \
                 -LC:\Users\farhan\Desktop\Workspace\RTREE-ORIG
                 
BOOST_LIBS    := -llibboost_program_options-mgw34-mt \
			   	 -llibboost_filesystem-mgw34-mt \
			   	 -llibboost_thread-mgw34-mt \
			   	 -llibboost_system-mgw34-mt

CXXFLAGS=-O2 -g -Wall -frounding-math -fmessage-length=0 -DFATAL -DDEBUG -DINFORMATION -DPROGRESS $(INCDIRS)

OBJS := SpatialObjectFactory.o \
        SpatialObject.o \
        SpatialObjectStream.o \
        Vertex.o \
        Box.o \
		Cone.o \
		Sphere.o \
		Triangle.o \
		Timer.o \
		BufferedFile.o \
		DataFileReader.o \
		ExternalSort.o \
		SegmentObjectReader.o \
		MeshObjectReader.o \
		SomaObjectReader.o \
GenerateData.o
		#Test.o
		#BruteTest.o
		#QueryTest.o 
		#GenerateData.o
		#RTreeQueryTest.o
		
LIBS := ../BBPSDK/libbbpsdk.a $(LIB_PATHS) -lhdf5 -lspatialindex $(BOOST_LIBS) 

TARGET = GenerateData #Test #BruteTest #QueryTest  #GenerateData 

all:    $(TARGET)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET):      $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)

