#pragma once

namespace tr
{
    class Servo final
    {
    public:
        Servo();
        ~Servo();

        void init();
        void rotate(int _angleDeg);
    };
} // namespace tr
