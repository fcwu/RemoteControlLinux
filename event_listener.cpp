///////////////////////////////////////////////////////////////////////////////
//
// Name: event_listener.cpp
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "event_listener.h"
#include "common.h"

SocketEventListener::SocketEventListener(int socket)
    : mSocket(socket)
{
}

bool SocketEventListener::update()
{
    try {
        int retVal;
        EventPacketPtr p;
        unsigned int unWrittenSize;
        char buf[5];
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);

        while (1) {

            while (mQueue->size() > 0) {
                p = mQueue->front();
                unWrittenSize = p->getPayloadLength();
                buf[0] = (char)p->getType();
                buf[1] = (char)((unsigned long)unWrittenSize >>  0) & 0x0FF;
                buf[2] = (char)((unsigned long)unWrittenSize >>  8) & 0x0FF;
                buf[3] = (char)((unsigned long)unWrittenSize >> 16) & 0x0FF;
                buf[4] = (char)((unsigned long)unWrittenSize >> 24) & 0x0FF;
                
                LOG_V("Socket write: header.type = %d, size = 0x%02X 0x%02X 0x%02X 0x%02X\n", 
                        buf[0], buf[1], buf[2], buf[3], buf[4]);
                retVal = write(mSocket, buf, 5);
                LOG_V("Socket write: data size = %d\n", retVal);
                if (retVal <= 0) {
                   RCT::Exception e(RCT_DEVICE_ERROR, "failed to write header");
                   throw e;
                }

                while (unWrittenSize > 0) {
                    retVal = write(mSocket
                            , p->getPayload() + (p->getPayloadLength() - unWrittenSize)
                            , unWrittenSize);
                    LOG_V("Socket write: data size = %d\n", retVal);
                    if (retVal > 0) {
                        unWrittenSize -= retVal;
                    } else {
                       RCT::Exception e(RCT_DEVICE_ERROR, "failed to write header");
                       throw e;
                    }
                }

                mQueue->pop();
            }
            xt.nsec += SHORT_WAITING_TIME;
            boost::thread::sleep(xt);
        }
    } catch (RCT::Exception& e) {
        LOG_E("SocketEventListener Execption: %s\n", e.what());
        throw e;
    }
    return true;
}

