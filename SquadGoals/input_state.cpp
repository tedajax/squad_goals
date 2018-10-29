#include "input_state.h"
#include <SDL2/SDL.h>
#include <algorithm>

input_state::input_state() {
    std::memset(keyStates, 0, sizeof(keyStates));
}

void input_state::update() {
    int count;
    const u8* states = SDL_GetKeyboardState(&count);

    // copy previous frames states into previous buffer
    std::copy(&keyStates[0][0], &keyStates[0][count], &keyStates[1][0]);

    // copy current states into current buffer
    std::copy(&states[0], &states[count], &keyStates[0][0]);
}

bool input_state::get_key(int scancode) const {
    return keyStates[0][scancode];
}

bool input_state::get_key_down(int scancode) const {
    return keyStates[0][scancode] && !keyStates[1][scancode];
}

bool input_state::get_key_up(int scancode) const {
    return keyStates[1][scancode] && !keyStates[0][scancode];
}