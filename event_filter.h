///////////////////////////////////////////////////////////////////////////////
//
// Name: event_filter.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __RCTEVENTFILTER_H__
#define __RCTEVENTFILTER_H__

#include "event.h"
#include "common.h"

class EventFilter
{
public:
    virtual void setQueue(EventQueuePtr p) { mQueue = p; }
    virtual bool update(EventPacketPtr) = 0;
protected:
    EventQueuePtr mQueue;
};

class MouseEventFilter : public EventFilter
{
public:
    virtual bool update(EventPacketPtr);
private:
};

class KeyboardEventFilter : public EventFilter
{
public:
    virtual bool update(EventPacketPtr);
private:
};

class WindowEventFilter : public EventFilter
{
public:
    virtual bool update(EventPacketPtr);
private:
};

class FileEventFilter : public EventFilter
{
public:
    virtual bool update(EventPacketPtr);
private:
};

class WebcamEventFilter : public EventFilter
{
public:
    virtual bool update(EventPacketPtr);
private:
};

typedef boost::shared_ptr<EventFilter>          EventFilterPtr;
typedef boost::shared_ptr<MouseEventFilter>     MouseEventFilterPtr;
typedef boost::shared_ptr<KeyboardEventFilter>  KeyboardEventFilterPtr;
typedef boost::shared_ptr<WindowEventFilter>    WindowEventFilterPtr;
typedef boost::shared_ptr<FileEventFilter>      FileEventFilterPtr;
typedef boost::shared_ptr<WebcamEventFilter>    WebcamEventFilterPtr;

#endif
