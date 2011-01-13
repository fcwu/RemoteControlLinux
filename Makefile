SOURCES_BASE=error.cpp
SOURCES=event_controller.cpp event_listener.cpp event_filter.cpp event_generator.cpp event.cpp server.cpp
OBJECTS=$(patsubst %.cpp, %.o, $(SOURCES)) $(patsubst %.cpp, %.o, $(SOURCES_BASE)) xwindow.o 
TARGET=server

dbus_CFLAGS=`pkg-config dbus-1 --cflags`
CFLAGS=-Wall $(dbus_CFLAGS) -g

dbus_LFLAGS=`pkg-config dbus-1 --libs`
boost_LFALGS=/usr/lib/libboost_filesystem-mt.a /usr/lib/libboost_system-mt.a /usr/lib/libboost_thread-mt.a -lpthread
xwin_LFLAGS=-lSM -lICE -lX11 -lXmu -lglib-2.0
bluez_LFALGS=-lbluetooth
LFLAGS=$(boost_LFALGS) $(xwin_LFLAGS) $(bluez_LFALGS) $(dbus_LFLAGS)

all: $(TARGET) 

$(TARGET): $(OBJECTS) xwindow.o
	g++ $^ -o $@ $(LFLAGS)

xwindow.o: xwindow.c
	gcc -DVERSION=\"1.07\" -I. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -MT xwindow.o -MD -MP -c -o $@ $<

event_filter.o: event_filter.cpp
	gcc -DVERSION=\"1.07\" -I. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -MT xwindow.o -MD -MP -c -o $@ $<

clean:
	-rm *.o
	-rm $(TARGET)

%.o: %.cpp
	g++ $(CFLAGS) $< -c -o $@

