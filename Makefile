# ObjectFile=

ExecFile = gtest_main.cpp \
           test.cpp

SrcFiles = gtest.cpp \
           gtest_internal.cpp

OBJ = ${patsubst %.cpp, %.o, $(SrcFiles)}
ExecOBJ = ${patsubst %.cpp, %.o, $(ExecFile)}

CFLAGS = -g -Wall -std=c++11

Lib = libmygtest.so

all : a.out

.cpp.o :
	g++ -fPIC $(CFLAGS) -c $^ -o $@

$(Lib) : ${OBJ}
	g++ -shared $^ -o $@

a.out : $(ExecOBJ) $(Lib)
	g++ -fPIC -L./ -Wl,-rpath=./ $^ -o $@

.PHONY:all clean

clean:
	rm *.o *.so *.out