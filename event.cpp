///////////////////////////////////////////////////////////////////////////////
//
// Name: event.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "event.h"

EventPacket::EventPacket(EventType type, char *payload, unsigned int payload_len)
{
    mType = type;
    mPayloadLen = payload_len;
    mPayload = payload;
    LOG_V("mPayloadLen = %d, mPayload= %p\n", mPayloadLen, mPayload);
}

EventPacket::~EventPacket()
{
    if (mPayloadLen > 0)
        delete mPayload;
}

