#pragma once

#include "types.h"

class input_state {
public:
    input_state();

    void update();

    bool get_key(int scancode) const;
    bool get_key_down(int scancode) const;
    bool get_key_up(int scancode) const;

private:
    u8 keyStates[2][512];
};