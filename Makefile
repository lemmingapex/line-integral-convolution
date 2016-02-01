INCFLAGS = -I /usr/include/GL

#CFLAGS = -O3 -funroll-loops -g -Wall -ansi
CFLAGS = -O1 -g -Wall -ansi
#CFLAGS = -g -Wall -ansi
CC = g++
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
PROG = LIC

#Linux
LINKFLAGS = -lGLEW -lglut -lm -lGL

# Windows (cygwin)
ifeq "$(OS)" "Windows_NT"
	LINKFLAGS = -lopengl32 -lglu32 -lglut32
	PROG = LIC.exe
endif

# OS X
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LINKFLAGS = -framework Carbon -framework OpenGL -framework GLUT
endif

all: $(SRCS) $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LINKFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCFLAGS)

depend:
	makedepend $(INCFLAGS) -Y $(SRCS)

clean:
	rm $(OBJS) $(PROG)
