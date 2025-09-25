#pragma once

#include "tram_run/Display.hpp"

namespace tr
{

class App final
{
public:
    enum class State
    {
        Starting,
        //ConnectingToWifi,
        Run

    };

    App();
    ~App();

    void start();

private:
    static void mainTask(void* _pvParameter);
    static void displayTask(void* _pvParameter);
    static void servoTask(void* _pvParameter);

    State getStateSafe() const;

    State m_state = State::Starting;

    Display m_display;
};

} // namespace tr
