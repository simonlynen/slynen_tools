
.PHONY: all
SHELL=/bin/bash

 CFLAGS=     -D__STDC_FORMAT_MACROS -I. -DNDEBUG         -g0 -O6 -fomit-frame-pointer -fno-exceptions -lpthread
#CFLAGS=-m32 -D__STDC_FORMAT_MACROS -I. -DNDEBUG         -g0 -O6 -fomit-frame-pointer -fno-exceptions -lpthread
#CFLAGS=     -D__STDC_FORMAT_MACROS -I. -D_DEBUG -DDEBUG -O0 -g                       -fno-exceptions -lpthread
#CFLAGS=-m32 -D__STDC_FORMAT_MACROS -I. -D_DEBUG -DDEBUG -O0 -g                       -fno-exceptions -lpthread

all: loadTest producerConsumer
	@echo done

loadTest:loadTest.cpp *.h
	c++ -o loadTest ${CFLAGS} loadTest.cpp

producerConsumer:producerConsumer.cpp *.h
	c++ -o producerConsumer ${CFLAGS} producerConsumer.cpp

clean:
	-@rm -rf producerConsumer loadTest *.o *.dSYM a.out *.s

