CFLAGS+= -g -Wall
LDFLAGS+=-lm     # link to math library



.PHONY: all clean train test op

train : MAKEFLAGS =  $(CFLAGS)
test  : MAKEFLAGS =  $(CFLAGS)

TARGET=test_hmm

all: $(TARGET)
# type make/make all to compile test_hmm

test: test.cpp
	g++ -o $@ $(MAKEFLAGS) test.cpp $(LDFLAGS)
train: train.cpp
	g++ -o $@ $(MAKEFLAGS) train.cpp $(LDFLAGS)

optrain : train.cpp
	g++ -o $@ -O3 train.cpp $(LDFLAGS)

optest  : test.cpp
	g++ -o $@ -O3 test.cpp $(LDFLAGS)

clean:
	rm -f test_hmm test train optrain optest  # type make clean to remove the compiled file
