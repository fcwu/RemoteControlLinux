///////////////////////////////////////////////////////////////////////////////
//
// Name: event_generator.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __RCTEVENTGENERATOR_H__
#define __RCTEVENTGENERATOR_H__

#include "event.h"
#include "common.h"

#include <dbus/dbus.h>

class EventGenerator
{
public:
    virtual void setQueue(EventQueuePtr p) { mQueue = p; }
    virtual bool generate() = 0;
    virtual bool getRunning() { return mRunning; }
    virtual void setRunning(bool b) { mRunning = b; }
protected:
    EventQueuePtr mQueue;
    bool          mRunning;
private:
};

class TcpAccepter : public EventGenerator
{
public:
    TcpAccepter();
    virtual ~TcpAccepter();
    virtual bool generate();
private:
    int mServerSocket;
};

class BluetoothAccepter : public EventGenerator
{
public:
    BluetoothAccepter();
    virtual ~BluetoothAccepter();
    virtual bool generate();
private:
    int mServerSocket;
};

class SocketEventGenerator: public EventGenerator
{
public:
    SocketEventGenerator(int socket);
    virtual bool generate();
protected:
    int mSocket;
};

class DbusEventGenerator: public EventGenerator
{
public:
    DbusEventGenerator();
    virtual bool generate();
protected:
    DBusConnection *mConn;
    static bool mIsInit;
};

typedef boost::shared_ptr<EventGenerator>       EventGeneratorPtr;
typedef boost::shared_ptr<TcpAccepter>          TcpAccepterPtr;
typedef boost::shared_ptr<BluetoothAccepter>    BluetoothAccepterPtr;
typedef boost::shared_ptr<SocketEventGenerator> SocketEventGeneratorPtr;
typedef boost::shared_ptr<DbusEventGenerator>   DbusEventGeneratorPtr;

#endif
