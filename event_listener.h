///////////////////////////////////////////////////////////////////////////////
//
// Name: event_listener.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __RCTEVENTLISTENER_H__
#define __RCTEVENTLISTENER_H__

#include "event.h"
#include "common.h"

class EventListener
{
public:
    virtual void setQueue(EventQueuePtr p) { mQueue = p; }
    virtual bool update() = 0;
protected:
    EventQueuePtr mQueue;
private:
};

class SocketEventListener: public EventListener
{
public:
    SocketEventListener(int socket);
    virtual bool update();
protected:
    int mSocket;
};

typedef boost::shared_ptr<EventListener>       EventListenerPtr;
typedef boost::shared_ptr<SocketEventListener> SocketEventListenerPtr;

#endif
