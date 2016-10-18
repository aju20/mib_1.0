all: mib_1.0

mib_1.0: mib_1.0.c
	gcc mib_1.0.c -lncurses -lform -lmenu -o mib_1.0
clean:
	rm *.o
