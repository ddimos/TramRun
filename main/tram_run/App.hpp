#pragma once

#include "tram_run/State.hpp"

namespace tr
{
    class App final
    {
    public:

        App();
        ~App();

        void start();

    private:
        static void mainTask(void* _pvParameter);
        //static void buttonTask(void* _pvParameter);

        state::Id getStateSafe() const;

        void transit(state::Transit _transit);
        void transitInitState(state::Transit _transit);
        void transitRunState(state::Transit _transit);

        void update();
        state::Status updateInitState();
        state::Status updateRunState();

        state::Id m_state = state::Id::Init;
    };

} // namespace tr
