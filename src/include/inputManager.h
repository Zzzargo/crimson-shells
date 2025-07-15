#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <SDL2/SDL_ttf.h>

typedef enum {
    INPUT_MOVE_UP,
    INPUT_MOVE_DOWN,
    INPUT_MOVE_LEFT,
    INPUT_MOVE_RIGHT,
    INPUT_SELECT,
    INPUT_BACK,
    INPUT_INTERACT,
    INPUT_SHOOT,
    INPUT_SPECIAL,
    INPUT_UNKNOWN,
    INPUT_ACTION_COUNT  // automatically counts
} InputAction;

typedef struct inputmng {
    const Uint8 *keyboardState;  // current keyboard state
    SDL_Scancode bindings[INPUT_ACTION_COUNT];  // input configuration
} *InputManager;

// Loads key bindings from a file into the input manager
// If the file does not exist, it will set the default bindings
void loadKeyBindings(InputManager inputMng, const char *filePath);

// saves the current key bindings to a file
void saveKeyBindings(InputManager inputMng, const char *filePath);

void setDefaultBindings(InputManager inputMng);

Uint8 isActionPressed(InputManager inputMng, InputAction action);

// Translates a scancode to an action
InputAction scancodeToAction(InputManager inputMng, SDL_Scancode scancode);

#endif