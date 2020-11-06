#include "RaspberryPiControl.hpp"

#include <iostream>

RaspberryPiControl::RaspberryPiControl() :
    commands_ { Command::START, 
                Command::STOP,
                Command::FULL,
                Command::HIGH_SPEED,
                Command::MEDIUM,
                Command::LOW_SPEED,
                Command::UP,
                Command::DOWN
              },
    commandNames_ { {Command::START, "Start"},
                    {Command::STOP, "Stop"},
                    {Command::FULL, "Full speed"},
                    {Command::HIGH_SPEED, "High speed"},
                    {Command::MEDIUM, "Medium speed"},
                    {Command::LOW_SPEED, "Low speed"},
                    {Command::UP, "Lift Up"},
                    {Command::DOWN, "Lift Down"} 
                  },
    currentCommand_ {Command::NO_CMD},
    direction_ {Command::NO_CMD},
    currentState_ {State::NOT_SETUP}
{}

bool RaspberryPiControl::setup() {
    int setupVal = wiringPiSetup() ;
    errNo_ = errno;
    if(setupVal == 0) {
        if(currentState_ == State::DISCONNECTED) {
            currentState_ = State::SETUP;
        } else {
            std::cout << __FILE__ << ":" << __LINE__
                << " INFO: Setting up pin mappings for circuit.\n";
            setupVal = softPwmCreate(enablePin, PWM_OUTPUT, 100);
            if(setupVal == 0) {
                pinMode(in1, OUTPUT);
                pinMode(in2, OUTPUT);
                currentState_ = State::SETUP;
            } else {
                std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR: Failed to create a pwm pin. pin["
                    << enablePin << "]\n";
                currentState_ = State::FAILURE;
            }
        }
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to setup. \n";
        currentState_ = State::FAILURE;
    }

    return setupVal == 0;
}

void RaspberryPiControl::initialize() {
    if(currentState_ == State::SETUP) {
        std::cout << __FILE__ << ":" << __LINE__ 
            << " INFO: Setting initial values for pins.\n";
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        currentState_ = State::DISCONNECTED;
    }
}

void RaspberryPiControl::printCommands() {
    std::cout << __FILE__ << ":" << __LINE__
        << " INFO: Available commands for raspberry pi: ";
    for(const auto& iter : commandNames_) {
        std::cout << iter.second << "("
            << static_cast<int>(iter.first) << ") ";
    }
    std::cout << "\n";
}

bool RaspberryPiControl::executeCommand(Command receivedCommand) {
    bool commandExecuted {false};
    currentCommand_ = receivedCommand;
    switch(receivedCommand) {
        case Command::START:
            commandExecuted = start();
            break;
        case Command::STOP:
            commandExecuted = stop();
            break;
        case Command::FULL:
        case Command::HIGH_SPEED:
        case Command::MEDIUM:
        case Command::LOW_SPEED:
            commandExecuted = changeSpeed(receivedCommand);
            break;
        case Command::UP:
        case Command::DOWN:
            changeDirection(receivedCommand);
            commandExecuted = true;
            break;
        default:
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Unknown command passed. command["
                << static_cast<int>(receivedCommand) << "]\n";
            break;
    }

    return commandExecuted;
}

std::string RaspberryPiControl::getCurrentCommand() {
    std::string commandName {""};
    auto commandNameIter = commandNames_.find(currentCommand_);
    if(commandNameIter != commandNames_.end()) {
        commandName = commandNameIter->second;
    } 

    return commandName;
}

void RaspberryPiControl::changeDirection(Command direction) {
    if(direction == Command::UP) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW); 
    } else if(direction == Command::DOWN) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH); 
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Invalid direction selected. direction["
            << getCurrentCommand() << "]\n";
    }
}

bool RaspberryPiControl::changeSpeed(Command speed) {
    bool speedChanged {false};

    switch(speed) {
        case Command::FULL:
            speedChanged = pwmWrite(100);
            break;
        case Command::HIGH_SPEED:
            speedChanged = pwmWrite(75);
            break;
        case Command::MEDIUM:
            speedChanged = pwmWrite(50);
            break;
        case Command::LOW_SPEED:
            speedChanged = pwmWrite(25);
            break;
        default:
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Invalid speed passed to pwm pin. pin["
                << enablePin << "] command[" << getCurrentCommand()
                << "]\n";
            break;
    }

    if(!speedChanged) currentState_ = State::FAILURE;

    return speedChanged;
}

bool RaspberryPiControl::stop() {
    bool isStopped {false};
    if(currentState_ != State::STOPPED) {
        isStopped = pwmWrite(0);
        currentState_ = isStopped ? State::STOPPED : State::FAILURE;
    }

    return isStopped;
}

bool RaspberryPiControl::start() {
    bool isStarted {false};
    if(currentState_ != State::STARTED) {
        changeDirection(Command::UP);
        isStarted = changeSpeed(Command::HIGH_SPEED);
        currentState_ = isStarted ? State::STARTED : State::FAILURE;
    }

    return isStarted;
}

bool RaspberryPiControl::pwmWrite(int pwmRange) {
    bool pwmValWritten {false};
    if(pwmRange >= 0 && pwmRange <= 100) {
        softPwmWrite(enablePin, pwmRange);
        errNo_ = errno;
        if(errNo_ != 0) {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Failed to write pwm value. pin["
                << enablePin << "] pwmRange[" << pwmRange
                << "]\n";
        } else {
            std::cout << __FILE__ << ":" << __LINE__
                << " INFO: Successfuly wrote pwm value. pin["
                << enablePin << "] pwmRange[" << pwmRange
                << "]\n";
            pwmValWritten = true;
        }
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Invalid pwm range passed. pwmRange["
            << pwmRange << "]\n";
    }

    return pwmValWritten;
}
