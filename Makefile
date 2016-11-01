all: project

project: mib_1.0.c
	gcc mib_1.0.c -lncurses -lform -lmenu -o project
clean:
	rm *.o
