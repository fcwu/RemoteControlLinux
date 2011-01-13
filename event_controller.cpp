///////////////////////////////////////////////////////////////////////////////
//
// Name: event_controller.cpp
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "event_controller.h"
#include "event.h"


using std::vector;

EventController::EventController() :
    mInputEventQueue(new EventQueue()),
    mOutputEventQueue(new EventQueue()),
    mRequestStop(false) 
{
}

EventController::~EventController() 
{
    vector<ThreadPtr>::iterator iterThread = mThreads.begin(), 
                                iterThreadEnd = mThreads.end();
    //std::vector<EventGeneratorPtr>::iterator iter = mGenerators.begin(),
    //                                         iterEnd = mGenerators.end();

    int i = 0;
    for (; iterThread != iterThreadEnd; ++iterThread, ++i) {
        if ((*iterThread)->joinable()) {
            LOG_I("interrupt: %d\n", i);
            (*iterThread)->interrupt();
            (*iterThread)->join();
        }
    }
}

bool EventController::addGenerator(EventGeneratorPtr generator) 
{
    generator->setQueue(mInputEventQueue);
    generator->setRunning(true);
    ThreadPtr pThread(new Thread(boost::bind(
                    &EventGenerator::generate, generator)));
    mThreads.push_back(pThread);
    mGenerators.push_back(generator);
    return true;
}

//bool EventController::removeGenerator(EventGeneratorPtr generator) {
//    vector<EventGeneratorPtr>::iterator iter = mGenerators.begin(), 
//                                        iterEnd = mGenerators.end();
//
//    for (; iter != iterEnd; ++iter) {
//        if (*iter == generator) {
//            mGenerators.erase(iter);
//            return true;
//        }
//    }
//    return false;
//}


bool EventController::addListener(EventListenerPtr listener) {
    listener->setQueue(mOutputEventQueue);
    ThreadPtr pThread(new Thread(boost::bind(
                    &EventListener::update, listener)));
    mThreads.push_back(pThread);
    mListeners.push_back(listener);
    return true;
}


//bool EventController::removeListener(EventListenerPtr listener) {
//    vector<EventListenerPtr>::iterator iter = mListeners.begin(), 
//                                        iterEnd = mListeners.end();
//
//    for (; iter != iterEnd; ++iter) {
//        if (*iter == listener) {
//            mListeners.erase(iter);
//            return true;
//        }
//    }
//    return false;
//}

bool EventController::addFilter(EventFilterPtr filter) {
    filter->setQueue(mOutputEventQueue);
    mFilters.push_back(filter);
    return true;
}


bool EventController::check() {
    //bool b = false;
    //b |= (mInputEventQueue->size() > 0) ? true : false;
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    std::vector<EventGeneratorPtr>::iterator iter = mGenerators.begin(),
                                             iterEnd = mGenerators.end();
    int i;
    while (1) {
        if (mInputEventQueue->size() > 0) {
            return true;
        }
        for (i = 0, iter = mGenerators.begin(); iter != iterEnd; ++iter, ++i) {
            if (!(*iter)->getRunning()) {
                LOG_I("Checking: Thread stop: %d\n", i);
                return false;
            }
        }
        xt.nsec += SHORT_WAITING_TIME;
        boost::thread::sleep(xt);
    }
    return false;
}


bool EventController::handle() {
    bool b = false;
    EventPacketPtr p; 
    vector<EventFilterPtr>::iterator iter = mFilters.begin(), 
                                     iterEnd = mFilters.end();
    try {
        while (mInputEventQueue->size() > 0) {
            p = mInputEventQueue->front();
            mInputEventQueue->pop();
            b = true;

            LOG_V("handle a event... 0x%02X\n", p->getType());
            for (; iter != iterEnd; ++iter) {
                if (!(*iter)->update(p))  // propagate ?
                    break;
            }
        }
    } catch (RCT::Exception& e) {
        throw e;
    }

    return b;
}

////////////////////////////////////////////////////////////////////////////////

int AcceptEventController::getSocket() {
    EventPacketPtr p; 
    int socket = -1;

    while (mInputEventQueue->size() > 0) {
        p = mInputEventQueue->front();
        mInputEventQueue->pop();
        if (p->getType() == INNER_GET_SOCKET) {
            socket = *(int *)(p->getPayload());
            break;
        }
    }
    return socket;
}


