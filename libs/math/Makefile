include ../../Makefile.inc

SOURCES := $(wildcard [^_]*.cc)
OBJECTS := $(foreach file,$(SOURCES),$(subst .cc,.o,$(file)))
LIBRARY := libmath.a

EXT_INCL := -I..
EXT_LIBS := -lpthread

TEST_SRC := _test.cc
TEST_BIN := test

GTEST_SRCS := $(wildcard _gtest_*.cc)
GTEST_OBJS := $(foreach file,$(GTEST_SRCS),$(subst .cc,.o,$(file)))
GTEST_INCL := -I${GTEST_PATH}/include
GTEST_LIBS := -L${GTEST_PATH}/make -lgtest_main

libmath: ${OBJECTS}
	#ar rcs ${LIBRARY} ${OBJECTS}
	#chmod a+x ${LIBRARY}

test: libmath FORCE
	${CXX} -o ${TEST_BIN} ${TEST_SRC} ${OBJECTS} ${EXT_INCL} ${EXT_LIBS} ${CXXFLAGS}

gtest: ${GTEST_OBJS} ${OBJECTS}
	${CXX} ${GTEST_OBJS} ${OBJECTS} -o _gtest ${GTEST_LIBS} ${EXT_LIBS}

_gtest_%.o: _gtest_%.cc
	${CXX} -c -o $@ $< ${GTEST_INCL} ${EXT_INCL}

%.o: %.cc
	${CXX} -c -o $@ $< ${CXXFLAGS} ${EXT_INCL}

depend:
	# This is commented out because there are no .cc files
	#${CXX} -MM ${SOURCES} ${EXT_INCL} > Makefile.dep

clean: FORCE
	${RM} ${OBJECTS} ${LIBRARY} ${TESTBIN}

FORCE:

-include Makefile.dep
