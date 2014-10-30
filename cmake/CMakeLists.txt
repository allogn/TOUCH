CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
PROJECT(SpatialIndex)
SET(MYLIB "FLATIndex")
SET(MACHINE bbplinsrv)


	MESSAGE (STATUS "Detected Sadegh Laptop")
	INCLUDE_DIRECTORIES (
	  ${CMAKE_SOURCE_DIR}/../include/
	  C:/Project/Brain/boost/include/ 
	  C:/Project/Brain/bbpdias/BBPSDK/include/
	  C:/Project/Brain/HDF5/include/
	  C:/Project/Brain/RTree/include/
	)
	
	LINK_DIRECTORIES ( 
	   C:/Project/Brain/boost/lib/ 
	   C:/Project/Brain/bbpdias/BBPSDK/
	   C:/Project/Brain/HDF5/lib/
	   C:/Project/Brain/RTree/
	)    
	
	SET(BBPSDK_LIB bbpsdk)
	SET(HDF5_LIB hdf5)
	SET(RTREE_LIB spatialindex)
	SET(BOOST_LIB libboost_program_options-mgw48-mt-1_56.dll libboost_filesystem-mgw48-mt-1_56.dll libboost_thread-mgw48-mt-1_56.dll libboost_system-mgw48-mt-1_56.dll)
	ADD_DEFINITIONS("-O3 -Wall -DPROFILING -DFATAL -DDEBUG -DINFORMATION -DPROGRESS ")
	OPTION(MAKE_EXECUTABLES "Make the executables." ON)


SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../bin/)
SET(LIBRARY_OUTPUT_PATH    ${CMAKE_SOURCE_DIR}/../lib/)
SET(SRC_DIR ${CMAKE_SOURCE_DIR}/../src/)
SET(APP_DIR ${CMAKE_SOURCE_DIR}/../apps/)
SET(INC_DIR ${CMAKE_SOURCE_DIR}/../include/)
SET(WRP_DIR ${CMAKE_SOURCE_DIR}/../wrapper/)


IF (MAKE_EXECUTABLES)

	FILE(GLOB SRC_FILES  ${SRC_DIR}/*.cpp)
	ADD_LIBRARY(${MYLIB} STATIC ${SRC_FILES})
	FILE(GLOB MAIN_FILES ${APP_DIR}*.cpp)
	FOREACH(APPNAME ${MAIN_FILES})
    		GET_FILENAME_COMPONENT(BASENAME ${APPNAME} NAME_WE)
    		ADD_EXECUTABLE(${BASENAME} ${APPNAME})
    		TARGET_LINK_LIBRARIES( ${BASENAME} ${MYLIB} ${BBPSDK_LIB} ${HDF5_LIB} ${BOOST_LIB} ${RTREE_LIB})    
	ENDFOREACH(APPNAME ${MAIN_FILES})
	MAKE_DIRECTORY(${LIBRARY_OUTPUT_PATH})
	MAKE_DIRECTORY(${EXECUTABLE_OUTPUT_PATH})

ELSE(MAKE_EXECUTABLES)

	FILE(GLOB SRC_FILES  ${SRC_DIR}/*.cpp)
	FILE(GLOB WRP_FILES  ${WRP_DIR}/*.cpp)
	ADD_LIBRARY(${MYLIB} SHARED ${SRC_FILES} ${WRP_FILES})
	TARGET_LINK_LIBRARIES( ${BASENAME} ${MYLIB} ${BBPSDK_LIB} ${BOOST_LIB} ${RTREE_LIB}) 
	MAKE_DIRECTORY(${LIBRARY_OUTPUT_PATH})

ENDIF (MAKE_EXECUTABLES)
