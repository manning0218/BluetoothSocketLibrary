#ifndef RASPBERRY_PI_CONTROL_HPP
#define RASPBERRY_PI_CONTROL_HPP

#include <wiringPi.h>
#include <softPwm.h>
#include <string>
#include <vector>
#include <unordered_map>

#define enablePin 1
#define in1 4
#define in2 5

enum class Command {
    START = 0,
    STOP,
    FULL,
    HIGH_SPEED,
    MEDIUM,
    LOW_SPEED,
    UP,
    DOWN,
    NO_CMD
};

enum class State {
    STOPPED = 0,
    STARTED,
    FAILURE,
    CONNECTED,
    DISCONNECTED,
    SETUP,
    NOT_SETUP
};

// TODO: Set as base class and create specialized classes for circuits
class RaspberryPiControl {
    public:
        RaspberryPiControl();
        ~RaspberryPiControl() = default;

        bool setup();
        void initialize();

        std::vector<Command> getCommands() { return commands_; }
        void printCommands();

        bool executeCommand(Command receivedCommand);

        std::string getCurrentCommand();
        State getCurrentState() { return currentState_; }

        Command getCommand(int i) { return commands_[i]; }

        void setConnectionState(bool connected) { currentState_ = connected ? State::CONNECTED : State::DISCONNECTED; }

        int getErrorNo() { return errNo_; }

    private:
        void changeDirection(Command direction);
        bool changeSpeed(Command speed);
        bool stop();
        bool start();
        
        bool pwmWrite(int pwmWrange);

    private:
        std::vector<Command> commands_;
        std::unordered_map<Command, std::string> commandNames_;
        Command currentCommand_;
        Command direction_;
        State currentState_;
        int errNo_;
};

#endif
