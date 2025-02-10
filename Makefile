UNAME_S := $(shell uname -s)

OBJ =  bin/ShaderProgram.o \
	   bin/main.o

all: bin/main

ifeq ($(UNAME_S),Linux)
LIBS = -lGL -lGLEW -lglfw -ldl -lfreetype
INCLUDES=-I ./include -I/usr/include/freetype2
else ifeq ($(UNAME_S),Darwin)
OBJ += bin/glad.o

LIBS= -L/opt/homebrew/opt/glfw/lib \
	  -lglfw -lfreetype

INCLUDES=-I./include \
		-I/opt/homebrew/opt/glfw/include \
		-I/opt/homebrew/include \
		-I./include \
		-I/usr/local/include

else
# Windows (MinGW64)
LIBS = -lglew32 -lglfw3 -lopengl32 -lgdi32 -lfreetype \
	   -LC:\msys64\mingw64\lib
INCLUDES=-IC:\msys64\mingw64\include -IC:\msys64\mingw64\include\freetype2

clean:
	del bin\*.o
	del bin\main.exe

endif

WARNINGS=-w

FLAGS=-std=c++17

bin/%.o: src/%.cpp | bin
	g++ -c $< -o $@ $(INCLUDES) $(WARNINGS) $(FLAGS)

bin/main: $(OBJ)
	g++ $(OBJ) $(FRAMEWORKS) $(LIBS) $(INCLUDES) -o $@ $(WARNINGS) $(FLAGS)

bin:
	mkdir bin
