#include "EventManager.hpp"
#include "BTSocket.hpp"

#include <string.h>

int EventManager::epollFd_{-1};
int EventManager::numberOfEvents_ {0};
epoll_event_t EventManager::eventManager_;
epoll_event_t *EventManager::events_ = (epoll_event_t*)malloc(sizeof(epoll_event_t)*maxNumberOfEvents_);
std::vector<std::unique_ptr<EventHandler>> EventManager::handlers_;
int EventManager::errNo_ {0};

bool EventManager::initialize() {
    bool eventManagerInitialized {false};

    if(epollFd_ == -1) {
        epollFd_ = ::epoll_create(maxNumberOfEvents_);
        errNo_ = errno;

        if(epollFd_ == -1) {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Failed to start instance of epoll.\n";
        } else {
            eventManagerInitialized = true;
        }
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Instance of epoll event is already started.\n";
    }

    return eventManagerInitialized;
}

bool EventManager::registerEvent(std::unique_ptr<EventHandler> handler) {
    if(handler->fd_ >= handlers_.size()) {
        handlers_.resize(handler->fd_);
        std::cout << __FILE__ << ":" << __LINE__
            << " DEBUG: Resized number of handlers. size["
            << handlers_.size() << "]\n";
    }

    eventManager_.events = handler->event_;
    eventManager_.data.fd = handler->fd_;
    int registeredEvent = epoll_ctl(epollFd_, EPOLL_CTL_ADD, handler->fd_, &eventManager_);
    errNo_ = errno;

    if(registeredEvent == -1) {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to add register event on epoll["
            << epollFd_ << "]\n";
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " INFO: Registered event. fd["
            << handler->fd_ << "]\n";

        handlers_[handler->fd_ - 1] = std::move(handler);
        ++numberOfEvents_;
    }

    return registeredEvent != -1;
}

bool EventManager::deregisterEvent(int fd) {
    bool deregisteredEvent {false};

    if(handlers_[fd - 1]) {
        int result = epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
        errNo_ = errno;

        if(result == -1) {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Failed to deregister event. fd["
                << fd << "]\n";
        } else {
            deregisteredEvent = true;
            handlers_[fd - 1] = nullptr;
            --numberOfEvents_;
        }

    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: File descriptor does not contain a handler. fd["
            << fd << "]\n";
    }

    return deregisteredEvent;
}

void EventManager::run() {
    for(;;) {
        int availableEvents = epoll_wait(epollFd_, events_, maxNumberOfEvents_, -1);
        errNo_ = errno;

        if(availableEvents == -1) {
            std::cout << __FILE__ << ":" << __LINE__ 
                << " ERROR: Failed to wait for event in epoll. epollFd["
                << epollFd_ << "] message["
                << strerror(errNo_) << "]\n";
            break;
        } else {
            triggerEvents(availableEvents);
        }
    }
}

void EventManager::triggerEvents(int availableEvents) {
    for(int i = 0; i < availableEvents; ++i) {
        if(events_[i].data.fd > 3) {
            if(events_[i].data.fd == 6) {
                //char buf[1024];
                //auto bytesRead = recv(events_[i].data.fd, buf, sizeof(buf), 0);
                //std::cout << __FILE__ << ":" << __LINE__
                //    << " TRACE: Received data: " << buf << "\n";
                EventHandlerImpl<void>* handler = (EventHandlerImpl<void>*)(handlers_[events_[i].data.fd-1].get());
                handler->func_();
            } else {
                const auto& handler = handlers_[events_[i].data.fd-1];
                handler->execute();
            }
        } else {
            std::cout << __FILE__ << ":" << __LINE__
                << " WARN: Event is missing handler.\n";
        }
    }
}
