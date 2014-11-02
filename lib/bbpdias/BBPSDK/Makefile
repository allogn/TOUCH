include ../Makefile.paths

CPPFLAGS = $(INCLUDE_PATHS) -MMD -MP -MT '$(basename $@).d' -MT '$(basename $@).o' -Iinclude
CXXFLAGS := -O3 -ggdb -Wall

LIB := libbbpsdk.a

SRC := Morphology_Reader.cpp Distiller.cpp Target.cpp Targets.cpp \
	Morphology_HDF5_File_Reader.cpp MVD_File_Parser.cpp Microcircuit.cpp \
	Morphology_HDF5_File_Parser.cpp Exceptions.cpp \
	Morphology_HDF5_v2_File_Parser.cpp Microcircuit_Composition_Reader.cpp \
	Microcircuit_MVD_File_Reader.cpp Neuron.cpp Microcircuit_Reader.cpp \
	Synapses.cpp Neurons.cpp Target_Reader.cpp Synapse_Reader.cpp \
	Mesh_Reader.cpp Mesh_File_Reader.cpp Target_File_Reader.cpp \
	Synapse_HDF5_File_Reader.cpp Mesh_ASCII_File_Parser.cpp \
	Mesh_Binary_File_Parser.cpp Mesh_Binary_File_Writer.cpp \
	Parsers/Lex_Yacc/ParsedTarget.cpp Parsers/Lex_Yacc/TargetParser.cpp \
	Parsers/Lex_Yacc/TargetScanner.cpp Experiment.cpp \
	Blue_Config_File_Reader.cpp Blue_Config_File_Parser.cpp \
	Experiment_Reader.cpp environ.cpp Segment_Voxel_Mapper.cpp \
	Compartment_Voltage_Voxel_Average.cpp \
	Compartment_Report_Reader_Impl.cpp \
	Compartment_Report_Binary_File_Reader.cpp \
	Compartment_Report_HDF5_File_Reader.cpp Compartment_Report_Mapping.cpp \
	Segment.cpp

OBJS := $(patsubst %.cpp,%.o,$(SRC))
DEPS := $(patsubst %.cpp,%.d,$(SRC))

all : $(LIB)

$(LIB) : $(OBJS)
	ar rcu $@ $?

clean :
	-rm $(OBJS) $(DEPS) $(LIB)

.PHONY : all clean

-include $(DEPS)
