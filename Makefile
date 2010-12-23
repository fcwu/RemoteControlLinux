TARGET=server
boost_LFALGS=/usr/lib/libboost_filesystem-mt.a /usr/lib/libboost_system-mt.a /usr/lib/libboost_thread-mt.a -lpthread
xwin_LFLAGS=-lSM -lICE -lX11 -lXmu -lglib-2.0
bluez_LFALGS=-lbluetooth
LFLAGS=$(boost_LFALGS) $(xwin_LFLAGS) $(bluez_LFALGS) `pkg-config dbus-1 --libs`

all: $(TARGET) 

$(TARGET): server.o xwindow.o session.o
	g++ $^ -o $@ $(LFLAGS)

server.o: server.cpp 
	g++ $< -c -o $@ `pkg-config dbus-1 --cflags`

xwindow.o: xwindow.c
	gcc -DVERSION=\"1.07\" -I. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -g -O2 -MT xwindow.o -MD -MP -c -o $@ $<

session.o: session.cpp 
	g++ $< -c -o $@ -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include `pkg-config dbus-1 --cflags`

clean:
	-rm *.o
	-rm server
