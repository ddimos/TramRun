#pragma once

namespace tr
{

class Display final
{
public:
    Display();
    ~Display();

    void init();
    void drawText(const char* _text, int _length, int _pos);
    void clear();
};

} // namespace tr
