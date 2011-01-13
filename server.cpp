///////////////////////////////////////////////////////////////////////////////
//
// Name: server.cpp
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "event.h"
#include "event_controller.h"
#include "event_generator.h"
#include "event_filter.h"
#include "common.h"

static void server()
{
    int socket = -1;

    //
    // Waiting for connection acception
    //
    try {
        AcceptEventController acceptEventController;

        TcpAccepterPtr tcpAccepter(new TcpAccepter());
        BluetoothAccepterPtr bluetoothAccepter(new BluetoothAccepter());

        acceptEventController.addGenerator(tcpAccepter);
        acceptEventController.addGenerator(bluetoothAccepter);
        while (acceptEventController.check()) {
            socket = acceptEventController.getSocket();
            if (socket != -1) {
                break;
            }
        }
    } catch (RCT::Exception& e) {
        LOG_E("Accepter server: %s\n", e.what());
        return;
    }

    //
    // Handling
    //
    LOG_I("Handling events....\n");
    
    try {
        EventController packetEventController;

        // Generator
        SocketEventGeneratorPtr socketEventGenerator(new SocketEventGenerator(socket));
        DbusEventGeneratorPtr dbusEventGenerator(new DbusEventGenerator());
        packetEventController.addGenerator(socketEventGenerator);
        packetEventController.addGenerator(dbusEventGenerator);

        // Listener
        SocketEventListenerPtr socketEventListener(new SocketEventListener(socket));
        packetEventController.addListener(socketEventListener);

        // Filter
        MouseEventFilterPtr mouseEventFilter(new MouseEventFilter());
        KeyboardEventFilterPtr keyboardEventFilter(new KeyboardEventFilter());
        WindowEventFilterPtr windowEventFilter(new WindowEventFilter());
        FileEventFilterPtr fileEventFilter(new FileEventFilter());
        WebcamEventFilterPtr webcamEventFilter(new WebcamEventFilter());
        packetEventController.addFilter(mouseEventFilter);
        packetEventController.addFilter(keyboardEventFilter);
        packetEventController.addFilter(windowEventFilter);
        packetEventController.addFilter(webcamEventFilter);
        packetEventController.addFilter(fileEventFilter);

        while (packetEventController.check()) {
            packetEventController.handle();
        }

    } catch (RCT::Exception& e) {
        LOG_E("Handling server: %s\n", e.what());
    }
}

int main(int argc, char* argv[])
{

    try {
        while (1) {
            LOG_I("start restart\n");
            server();
        }
    } catch (std::exception& e) {
        LOG_E("MAIN Exception: %s\n", e.what());
        return -1;
    }

    return 0;
}

