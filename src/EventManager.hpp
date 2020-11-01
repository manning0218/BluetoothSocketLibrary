#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <iostream>
#include <cerrno>
#include <memory>
#include <utility>
#include <vector>
#include <type_traits>

#include <sys/epoll.h>
#include <unistd.h>

#include "EventHandler.hpp"

typedef struct epoll_event epoll_event_t;

class EventManager {
    public: 
        EventManager() = delete;
        ~EventManager() = delete;

        static bool initialize();
        static bool registerEvent(std::unique_ptr<EventHandler> handler);
        static bool deregisterEvent(int fd);

        static void run();

    protected:
        static void triggerEvents(int availableEvents);

    private:
        static int epollFd_;
        static int numberOfEvents_;
        static constexpr int maxNumberOfEvents_ {10};
        static epoll_event_t eventManager_;
        static epoll_event_t *events_;
        static std::vector<std::unique_ptr<EventHandler>> handlers_;
        static int errNo_;
};

#endif
