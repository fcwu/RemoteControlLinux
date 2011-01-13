///////////////////////////////////////////////////////////////////////////////
//
// Name: event_controller.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __RCTEVENTCOTROLLER_H__
#define __RCTEVENTCOTROLLER_H__

#include "event.h"
#include "event_generator.h"
#include "event_listener.h"
#include "event_filter.h"

#include "common.h"

#include <vector>

class EventController
{
public:

    EventController();
    ~EventController();

    /**
     * Generator (Thread)
     */
    bool addGenerator(EventGeneratorPtr);

    /**
     * Listener (Thread)
     */
    bool addListener(EventListenerPtr);

    /**
     * Filter
     */
    bool addFilter(EventFilterPtr);

    /**
     * Handling
     */
    bool check();
    bool handle();

protected:
    EventQueuePtr mInputEventQueue;
    EventQueuePtr mOutputEventQueue;
    std::vector<EventFilterPtr> mFilters;
    std::vector<EventGeneratorPtr> mGenerators;
    std::vector<EventListenerPtr> mListeners;
    std::vector<ThreadPtr> mThreads;
private:
    bool mRequestStop;
};

class AcceptEventController : public EventController
{
public:
    int getSocket();
protected:
private:
};


typedef boost::shared_ptr<EventController> EventControllerPtr;

#endif
