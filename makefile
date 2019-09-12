# set this variable to the director in which you saved the common files
commondir = ../common/

dir = files/

OBJECTS = main.o Engine.o Person.o DrawCalls.o VectorUtils3.o GL_utilities.o loadobj.o LoadTGA.o MicroGlut.o

gunvor: $(OBJECTS) makefile
	gcc -o gunvor $(OBJECTS) -lXt -lX11 -lGL -lm -g -Wall -Wextra -Wpedantic

common: $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	gcc -I$(commondir) -c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

#Sub compiles
main.o: main.c
	gcc -c main.c

Engine.o: $(dir)Engine.c
	gcc -I$(dir) $(CCFLAGS) -c $(dir)Engine.c -lXt -lX11 -lGL -lm

Controls.o: $(dir)Controls.c
	gcc -I$(dir) $(CCFLAGS) -c $(dir)Controls.c -lXt -lX11 -lGL -lm

Person.o: $(dir)Person.c
	gcc -I$(dir) $(CCFLAGS) -c $(dir)Person.c -lXt -lX11 -lGL -lm

DrawCalls.o: $(dir)DrawCalls.c
	gcc -I$(dir) $(CCFLAGS) -c $(dir)DrawCalls.c -lXt -lX11 -lGL -lm

clean :
	@ \rm -rf *.o gunvor
