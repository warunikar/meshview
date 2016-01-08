
all: smf_view

smf_view: main.o process.o
	g++ main.o process.o -o smf_view libglui.a -lglut -lGLU -lGL

main.o: main.cpp
	g++ -c main.cpp

process.o: process.cpp
	g++ -c process.cpp

clean:
	rm -rf smf_view
