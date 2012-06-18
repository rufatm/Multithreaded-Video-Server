CLI = client
SERV = server

all:
	gcc -c -I./include -I./include/netpbm -L/usr/X11R6/lib -L./lib/GL -L./lib/netpbm DrawWindow.c -lGLU -lGL -lnetpbm -lXext -lX11 -lm -o DrawWindow.o
	g++ DrawWindow.o -I./include -I./include/netpbm -L/usr/X11R6/lib -L./lib/GL -L./lib/netpbm Client.cpp -lGLU -lGL -lnetpbm -lpthread -lXext -lX11 -lm -o $(CLI)
	g++ -I./include -I./include/netpbm -L/usr/X11R6/lib -L./lib/GL -L./lib/netpbm CircularBuffer.cpp Server.cpp -lGLU -lGL -lnetpbm -lpthread -lXext -lX11 -lm -o $(SERV)

clean:
	rm -f *~
	rm -f *.o
	rm -f $(CLI)
	rm -f $(SERV)

