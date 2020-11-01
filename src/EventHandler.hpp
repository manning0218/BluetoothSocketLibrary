#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <utility>
#include <functional>

struct EventHandler {
    int fd_;
    unsigned int event_;
    
    virtual void execute() = 0;
};

template<typename T>
struct EventHandlerImpl : public EventHandler{
    std::function<T()> func_;

    void execute() {
        func_();
    }
};

#endif 
