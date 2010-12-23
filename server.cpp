///////////////////////////////////////////////////////////////////////////////
//
// Name: server.cpp
// Author: doro_wu@asus.com
// 
// Copyright 2010 by Doro Wu. All right reserved
//
///////////////////////////////////////////////////////////////////////////////


#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <dbus/dbus.h>


#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>


using std::cout;
using std::cerr;
using std::endl;

void session(int sock, DBusConnection *conn);
static boost::mutex s_doingSession;
static DBusConnection* s_dbusConn;

static bool dbus_init() 
{
    DBusError err;
    int ret;

    // initialise the errors
    dbus_error_init(&err);

    // connect to the bus
    s_dbusConn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) { 
        fprintf(stderr, "Connection Error (%s)\n", err.message); 
        dbus_error_free(&err); 
    }
    if (NULL == s_dbusConn) { 
        exit(1); 
    }

    // add a rule for which messages we want to see
    dbus_bus_add_match(s_dbusConn, 
            "type='signal',interface='server.file.signal.Type'", 
            &err); // see signals from the given interface
    dbus_connection_flush(s_dbusConn);
    if (dbus_error_is_set(&err)) { 
        fprintf(stderr, "Match Error (%s)\n", err.message);
        return false;
    }

    // request a name on the bus
    ret = dbus_bus_request_name(s_dbusConn, "com.doro.server", 
            DBUS_NAME_FLAG_REPLACE_EXISTING 
            , &err);
    if (dbus_error_is_set(&err)) { 
        fprintf(stderr, "Name Error (%s)\n", err.message); 
        dbus_error_free(&err); 
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
        return false;
    }

}

static void server_bluetooth_accepter(int server)
{
    struct sockaddr_rc rem_addr = { 0 };
    socklen_t opt = sizeof(rem_addr);
    char buf[1024] = { 0 };
    int client;

    for (;;)
    {
        // TODO: set timeout for accept function 
        client = accept(server, (struct sockaddr *)&rem_addr, &opt);

        s_doingSession.lock();
        ba2str( &rem_addr.rc_bdaddr, buf );
        cerr << "accepted bluetooth connection from " << buf << endl;

        session(client, s_dbusConn);
    }


    close(server);
}

static boost::thread * server_bluetooth()
{
    struct sockaddr_rc loc_addr = { 0 };
    int server;

    if ((server = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0) {
        cerr << "bluetooth server: failed to create listening socket.\n";
        return NULL;
    }

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 1;

    // put socket into listening mode
    if (bind(server, (struct sockaddr *)&loc_addr, sizeof(loc_addr)) < 0 ) {
        cerr << "bluetooth server: Error calling bind()\n";
        return NULL;
    }

    // put socket into listening mode
    if (listen(server, 1) < 0 ) {
        cerr << "bluetooth server: Error calling listen()\n";
        return NULL;
    }

    // accept one connection
    cerr << "waitting bluetooth connection...\n";

    boost::thread *pThread = new boost::thread(
            boost::bind(server_bluetooth_accepter, server));


    return pThread;
}

static void server_tcp_accepter(int server)
{
    struct sockaddr_rc rem_addr = { 0 };
    socklen_t opt = sizeof(rem_addr);
    char buf[1024] = { 0 };
    int client;

    for (;;)
    {
        // accept one connection
        cerr << "waitting tcp connection...\n";

        // TODO: set timeout for accept function 
        client = accept(server, (struct sockaddr *)&rem_addr, &opt);
        s_doingSession.lock();

        //ba2str( &rem_addr.rc_bdaddr, buf );
        cerr << "accepted tcp connection from " << endl;

        session(client, s_dbusConn);
    }


    close(server);
}

static boost::thread * server_tcp()
{
    struct sockaddr_in loc_addr = { 0 };
    int server;

    server = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if ( (server = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        cerr << "tcp server: failed to create listening socket.\n";
        return NULL;
    }

    // bind socket to port 2222 of the first available 
    // local bluetooth adapter
    loc_addr.sin_family      = AF_INET;
    loc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    loc_addr.sin_port        = htons(2222);

    if (bind(server, (struct sockaddr *)&loc_addr, sizeof(loc_addr)) < 0 ) {
        cerr << "tcp server: Error calling bind()\n";
        return NULL;
    }

    // put socket into listening mode
    if (listen(server, 1) < 0 ) {
        cerr << "tcp server: Error calling listen()\n";
        return NULL;
    }

    boost::thread *pThread = new boost::thread(
            boost::bind(server_tcp_accepter, server));


    return pThread;
}

void server()
{
    boost::thread *threadTcp;
    boost::thread *threadBluetooth;

    threadTcp = server_tcp();
    threadBluetooth = server_bluetooth();

    if (threadTcp == NULL) {
        cerr << "Failed to initialize tcp\n";
    }
    if (threadBluetooth == NULL) {
        cerr << "Failed to initialize bluetooth\n";
    }

    // waitting here...
    if (threadTcp != NULL) {
        threadTcp->join();
    }
    if (threadBluetooth != NULL) {
        threadBluetooth->join();
    }
}

int main(int argc, char* argv[])
{
    dbus_init();
    try {
        server();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

