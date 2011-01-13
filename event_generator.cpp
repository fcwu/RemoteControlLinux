///////////////////////////////////////////////////////////////////////////////
//
// Name: event_generator.cpp
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "event_generator.h"
#include "common.h"

// tcp
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

// bluetooth
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

// dbus
#include <dbus/dbus.h>

#ifdef LOG_V(pcString, ...) 
#undef LOG_V
#define LOG_V(pcString, ...) 
#endif

TcpAccepter::TcpAccepter()
    :mServerSocket(-1)
{
}

TcpAccepter::~TcpAccepter()
{
    if (mServerSocket > 0)
        close(mServerSocket);
}

bool TcpAccepter::generate()
{
    mRunning = true;

    try {
        struct sockaddr_in loc_addr = { 0 };

        mServerSocket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
        if ( (mServerSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            RCT::Exception exception(RCT_DEVICE_ERROR, 
                    "failed to create listening socket");
            throw exception;
        }

        // bind socket to port 2222 of the first available 
        // local bluetooth adapter
        loc_addr.sin_family      = AF_INET;
        loc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        loc_addr.sin_port        = htons(2222);

        if (bind(mServerSocket, (struct sockaddr *)&loc_addr, sizeof(loc_addr)) < 0 ) {
            RCT::Exception exception(RCT_DEVICE_ERROR, 
                    "Error calling bind()");
            throw exception;
        }

        // put socket into listening mode
        if (listen(mServerSocket, 1) < 0 ) {
            RCT::Exception exception(RCT_DEVICE_ERROR, 
                    "Error calling listen()");
            throw exception;
        }

        // accept one connection
        LOG_I("waitting tcp connection...\n");

        struct sockaddr_in rem_addr = { 0 };
        socklen_t opt = sizeof(rem_addr);
        char buf[1024] = { 0 };
        int client;
        fd_set rfds;
        struct timeval tv;
        int retval;
        int *pInt = new int;

        for (;;)
        {

            // Watch socket to see when it has input
            FD_ZERO(&rfds);
            FD_SET(mServerSocket, &rfds);

            // Wait up to one seconds
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            retval = select(mServerSocket + 1, &rfds, NULL, NULL, &tv);

            if (retval == -1) {
                RCT::Exception exception(RCT_DEVICE_ERROR, 
                        "Error calling select()");
                throw exception;
            } else if (retval) {
                //LOG_I("Data is available now.\n");
                /* FD_ISSET(0, &rfds) will be true. */
            } else {
                boost::this_thread::interruption_point(); 
                //LOG_I("timeout\n");
                continue;
            }

            client = accept(mServerSocket, (struct sockaddr *)&rem_addr, &opt);

            //socklen_t len;
            //struct sockaddr_storage addr;
            //char ipstr[INET6_ADDRSTRLEN];
            //int port;

            //len = sizeof addr;
            //getpeername(s, (struct sockaddr*)&addr, &len);

            // deal with both IPv4 and IPv6:
            if (rem_addr.sin_family == AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *)&rem_addr;
                //port = ntohs(s->sin_port);
                inet_ntop(AF_INET, &s->sin_addr, buf, sizeof(buf));
            }
            LOG_I("Accepted tcp connection from %s\n", buf);
            

            *pInt = client;
            EventPacketPtr packet(new EventPacket(INNER_GET_SOCKET, (char *)pInt, sizeof(int)));
            mQueue->push(packet);

            break;
        }
    } catch (RCT::Exception& e) {
        LOG_E("Tcp Server Exception: %s\n", e.what());
        mRunning = false;
        return false;
    }
    mRunning = false;
    return true;
}

BluetoothAccepter::BluetoothAccepter() 
    :mServerSocket(-1)
{
}

BluetoothAccepter::~BluetoothAccepter() 
{
    if (mServerSocket > 0)
        close(mServerSocket);
}

bool BluetoothAccepter::generate() 
{
    mRunning = true;

    try {
        struct sockaddr_rc loc_addr = { 0 };

        if ((mServerSocket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0) {
            RCT::Exception exception(RCT_DEVICE_ERROR, 
                    "failed to create listening socket");
            throw exception;
        }

        // bind socket to port 1 of the first available 
        // local bluetooth adapter
        loc_addr.rc_family = AF_BLUETOOTH;
        loc_addr.rc_bdaddr = *BDADDR_ANY;
        loc_addr.rc_channel = (uint8_t) 1;

        // put socket into listening mode
        if (bind(mServerSocket, (struct sockaddr *)&loc_addr, sizeof(loc_addr)) < 0 ) {
            RCT::Exception exception(RCT_DEVICE_ERROR, 
                    "Error calling bind()");
            throw exception;
        }

        // put socket into listening mode
        if (listen(mServerSocket, 1) < 0 ) {
            RCT::Exception exception(RCT_DEVICE_ERROR, 
                    "Error calling listen()");
            throw exception;
        }

        // accept one connection
        LOG_I("waitting bluetooth connection...\n");

        struct sockaddr_rc rem_addr = { 0 };
        socklen_t opt = sizeof(rem_addr);
        char buf[1024] = { 0 };
        int client;
        fd_set rfds;
        struct timeval tv;
        int retval;
        int *pInt = new int;

        for (;;)
        {

            // Watch socket to see when it has input
            FD_ZERO(&rfds);
            FD_SET(mServerSocket, &rfds);

            // Wait up to one seconds
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            retval = select(mServerSocket + 1, &rfds, NULL, NULL, &tv);

            if (retval == -1) {
                RCT::Exception exception(RCT_DEVICE_ERROR, 
                        "Error calling select()");
                throw exception;
            } else if (retval) {
                //LOG_I("Data is available now.\n");
                /* FD_ISSET(0, &rfds) will be true. */
            } else {
                //LOG_I("timeout\n");
                boost::this_thread::interruption_point(); 
                continue;
            }

            client = accept(mServerSocket, (struct sockaddr *)&rem_addr, &opt);

            ba2str(&rem_addr.rc_bdaddr, buf );
            LOG_I("Accepted bluetooth connection from %s\n", buf);

            *pInt = client;
            EventPacketPtr packet(new EventPacket(INNER_GET_SOCKET, (char *)pInt, sizeof(int)));
            mQueue->push(packet);

            break;
        }
    } catch (RCT::Exception& e) {
        LOG_E("Bluetooth Server Exception: %s\n", e.what());
        mRunning = false;
        return false;
    }
    LOG_I("BT exit\n");

    mRunning = false;
    return true;
}

SocketEventGenerator::SocketEventGenerator(int socket)
    :mSocket(socket)
{
}

int readWrap(int socket, char *pBuf, int requestSize)
{
    static int offsetBuf = 0;
    static int lenBuf = 0;
    static char buf[MAX_BUFFER_SIZE];

    int retVal;

    if (offsetBuf == lenBuf) {
        offsetBuf = 0;
        retVal = read(socket, buf, sizeof(buf));
        if (retVal < 0) {
            return -1;
        } else if (retVal == 0) {
            return 0;
        }
        lenBuf = retVal;
    }

    if ((lenBuf - offsetBuf) >= requestSize) {
        // buffer data is enough
        //offsetBuf += requestSize;
        //memcpy(pBuf, buf + offsetBuf, requestSize);
        //return requestSize;
    } else {
        // buffer data is not enough
        requestSize = (lenBuf - offsetBuf);
    }
    memcpy(pBuf, buf + offsetBuf, requestSize);
    LOG_V("readWrap: retVal = %d, offset = %d, requestSize = %d\n"
            , retVal, offsetBuf, requestSize);
    offsetBuf += requestSize;
    return requestSize;
}


int readSize(int socket, char *pBuf, int requestSize)
{
    int retVal;
    int offsetBuf = 0;

    while (requestSize > 0) {
        retVal = readWrap(socket, pBuf + offsetBuf, requestSize);
        LOG_V("readSize: ret = %d, offset = %d\n", retVal, offsetBuf);
        if (retVal < 0) {
            return -1;
        } else if (retVal == 0) {
            return 0;
        }
        offsetBuf += retVal;
        requestSize -= retVal;
    }
    return 1;
}

int readPacket(int socket, EventPacketPtr *p) 
{
    int retVal;
    char buf[5] ;

    retVal = readSize(socket, buf, 5);
    if (retVal < 0) {
        return retVal;
    } else if (retVal == 0) {
        return retVal;
    }

    int type = buf[0];
    int packetSize = (((int)buf[1] << 0) & 0x000000FF) 
                        | (((int)buf[2] <<  8) & 0x0000FF00) 
                        | (((int)buf[3] << 16) & 0x00FF0000) 
                        | (((int)buf[4] << 24) & 0xFF000000) ;
    LOG_I("Packet Header: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
            buf[0], buf[1], buf[2], buf[3], buf[4]);
    char * dataBuf = 0;
    if (packetSize > 0) {
        dataBuf = new char[packetSize];
        LOG_I("Get packet with type = %d  and size = %d\n", type, packetSize);

        retVal = readSize(socket, dataBuf, packetSize);
        if (retVal < 0) {
            return retVal;
        } else if (retVal == 0) {
            return retVal;
        }
    }

    /////////////////////////
    // Dump packet data
    /////////////////////////
    char pktDataStr[128];
    int offsetPktDataStr = 0;
    int i;
    for (i = 0; i < packetSize ; ++i) {
        if (i % 16 == 0) {
            offsetPktDataStr += sprintf(pktDataStr + offsetPktDataStr
                    , "0x%08X: ", i);
        }
        offsetPktDataStr += sprintf(pktDataStr + offsetPktDataStr, "0x%02X ", 
                dataBuf[i] & 0x000000FF);
        if (i % 16 == 15) {
            LOG_I("%s", pktDataStr);
            offsetPktDataStr = 0;
        }
    }
    if (i % 16 != 15) {
        LOG_I("%s\n", pktDataStr);
        offsetPktDataStr = 0;
    }
    /////////////////////////

    *p = EventPacketPtr(new EventPacket((EventType)type
                , dataBuf, packetSize));
    
    return retVal;
}

bool SocketEventGenerator::generate()
{
    mRunning = true;

    try {
        int retVal;

        for (;;)
        {

            EventPacketPtr p;
            retVal = readPacket(mSocket, &p);
            if (retVal < 0) {
                RCT::Exception exception(RCT_DEVICE_ERROR, "when reading");
                throw exception;
            } else if (retVal == 0) {
                RCT::Exception exception(RCT_DEVICE_ERROR, "EOF");
                throw exception;
            }

            mQueue->push(p);

        }
    } catch (RCT::Exception& e) {
        LOG_E("Socket Server Exception: %s\n", e.what());
        mRunning = false;
        return false;
    }
    mRunning = false;
    return true;
}

bool DbusEventGenerator::mIsInit = false;

DbusEventGenerator::DbusEventGenerator()
{
    DBusError err;
    int ret;

    // initialise the errors
    dbus_error_init(&err);

    // connect to the bus
    mConn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) { 
        LOG_E("Connection Error (%s)\n", err.message); 
        dbus_error_free(&err); 
    }
    if (NULL == mConn) { 
        return;
    }

    // add a rule for which messages we want to see
    dbus_bus_add_match(mConn, 
            "type='signal',interface='server.file.signal.Type'", 
            &err); // see signals from the given interface
    dbus_connection_flush(mConn);
    if (dbus_error_is_set(&err)) { 
        LOG_E("Match Error (%s)\n", err.message);
        return;
    }

    // request a name on the bus
    ret = dbus_bus_request_name(mConn, "com.doro.server", 
            DBUS_NAME_FLAG_REPLACE_EXISTING 
            , &err);
    if (dbus_error_is_set(&err)) { 
        LOG_E("Name Error (%s)\n", err.message); 
        dbus_error_free(&err); 
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
        return ;
    }

    mIsInit = true;
}

bool DbusEventGenerator::generate()
{
    DBusMessage* msg;
    DBusMessageIter args;
    char* sigvalue;
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    mRunning = true;

    if (!mIsInit)
        return false;
    while (1) {
        // non blocking read of the next available message
        dbus_connection_read_write(mConn, 0);
        msg = dbus_connection_pop_message(mConn);

        // loop again if we haven't read a message
        if (NULL == msg) { 
            xt.nsec += 100000000;
            boost::thread::sleep(xt);
            continue;
        }

        // check if the message is a signal from the correct interface
        // and with the correct name
        if (dbus_message_is_signal(msg, "server.file.signal.Type", "send")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args)) {
                LOG_E("Message has no arguments!\n"); 
            } else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
                LOG_E("Argument is not string!\n"); 
            } else {
                dbus_message_iter_get_basic(&args, &sigvalue);
                LOG_I("Got Signal with value %s\n", sigvalue);
                int len = strlen(sigvalue) + 1;
                char *filename = new char[len];
                memcpy(filename, sigvalue, len);
                filename[len] = 0;
                EventPacketPtr p = EventPacketPtr(new EventPacket((EventType)SP_FILE_SEND
                            , filename, len));
                mQueue->push(p);
                //session_sp_cmd_send_file(SP_FILE_SEND, sigvalue);
            }
        } else {
        }

        // free the message
        dbus_message_unref(msg);
    }
}

