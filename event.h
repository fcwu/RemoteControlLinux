///////////////////////////////////////////////////////////////////////////////
//
// Name: event.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __RCTEVENT_H__
#define __RCTEVENT_H__

#include "common.h"

#include <queue>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

typedef enum {
    MOUSE_DIRECTION_UP = 0, //deprecated
    MOUSE_DIRECTION_RIGHT,  //deprecated
    MOUSE_DIRECTION_DOWN,   //deprecated
    MOUSE_DIRECTION_LEFT,   //deprecated
    MOUSE_LCLICK,
    MOUSE_RCLICK,
    MOUSE_MOVE,
    MOUSE_MAX,              //deprecated
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_LEFT,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_CUSTOM,
    KEY_MAX,
    SP_WIN_LIST,
    SP_WIN_FOCUS,
    SP_WIN_LIST_ICON,
    SP_FILE_SEND,
    SP_WEBCAM,
    SP_MAX,
    /**
     * Local Event
     */
    DBUS_FILE_SEND,
    INNER_GET_SOCKET
} EventType;


class EventPacket
{
public:
    EventPacket(EventType type, char *payload, unsigned int payload_len);
    ~EventPacket();

    EventType getType() const { return mType; }
    unsigned int getPayloadLength() const { return mPayloadLen; }
    char * getPayload() const { return mPayload; }

private:
    EventPacket();
    EventType mType;
    unsigned int mPayloadLen;
    char * mPayload;
};

typedef boost::shared_ptr<EventPacket> EventPacketPtr;
typedef std::queue<EventPacketPtr>     EventQueue;
typedef boost::shared_ptr<EventQueue>  EventQueuePtr;

typedef boost::thread Thread;
typedef boost::shared_ptr<boost::thread> ThreadPtr;


#endif

