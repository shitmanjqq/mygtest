CXX = g++
INCLUDEFLAGS=

ExecFile = gtest_main.cpp 
           # test.cpp

SrcFiles = gtest.cpp \
           gtest_internal.cpp

IncludeFile = gtest.h \
              gtest_def.h \
              gtest.h \
              gtest_internal.h \
              gtest_message.h \
              gtest_port.h \
              gtest_pred_impl.h \
              gtest_printers.h \
              gtest_string.h \
              gtest_test_part.h

OBJ = ${patsubst %.cpp, %.o, $(SrcFiles)}
ExecOBJ = ${patsubst %.cpp, %.o, $(ExecFile)}

CFLAGS = -g -Wall -std=c++11

Lib = libmygtest.so

all : a.out

.cpp.o :
	$(CXX) -shared -fPIC $(CFLAGS) -c $< -o $@

$(Lib) : ${OBJ}
	$(CXX) -shared $^ -o $@ 

a.out : $(Lib) $(ExecFile)
	$(CXX) -fPIC $(CFLAGS) -L./ -Wl,-rpath=./ -o $@ $(ExecFile) $<


%.d:%.cpp
	@set -e; rm -f $@; $(CXX) -MM $< $(INCLUDEFLAGS) > $@.$$; \
   sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$ > $@; \
   rm -f $@.$$

-include $(OBJ:.o=.d)
-include $(ExecOBJ:.o=.d)

.PHONY : all clean

clean:
	rm *.o *.so *.out *.d
