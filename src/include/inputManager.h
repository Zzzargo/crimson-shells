#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "global.h"

// Available input actions enum
typedef enum {
    INPUT_MOVE_UP,
    INPUT_MOVE_DOWN,
    INPUT_MOVE_LEFT,
    INPUT_MOVE_RIGHT,
    INPUT_SELECT,
    INPUT_BACK,
    INPUT_INTERACT,
    INPUT_SHOOT,
    INPUT_SECONDARY,  // Secondary weapon
    INPUT_SWITCH_RIGHT,
    INPUT_SWITCH_LEFT,
    INPUT_SPECIAL,
    INPUT_UNKNOWN,
    INPUT_ACTION_COUNT  // automatically counts
} InputAction;

typedef struct inputmng {
    const Uint8 *keyboardState;  // current keyboard state
    SDL_Scancode bindings[INPUT_ACTION_COUNT];  // input configuration
} *InputManager;

/**
 * Saves the current key bindings to a file
 * @param inputMng pointer to the input manager
 * @param filePath path to the settings file
 * @note The function saves the bindings in a simple key=value format
 */
void saveKeyBindings(InputManager inputMng, const char *filePath);

/**
 * Sets the default key bindings
 * @note Defaults: WASD + IJKL
 * @param inputMng pointer to the input manager
 */
void setDefaultBindings(InputManager inputMng);

/**
 * Checks if an action is currently pressed
 * @param inputMng pointer to the input manager
 * @param action enum type of the action to check
 */
Uint8 isActionPressed(InputManager inputMng, InputAction action);

/**
 * Translates a SDL scancode to an engine action
 * @param inputMng pointer to the input manager
 * @param scancode SDL scancode to translate
 * @return InputAction enum value corresponding to the scancode
 * @note Returns INPUT_UNKNOWN if the scancode is not bound to any action
 */
InputAction scancodeToAction(InputManager inputMng, SDL_Scancode scancode);

/**
 * Translates an InputAction to a human-readable key name
 * @param inputMng pointer to the input manager
 * @param action InputAction enum value to translate
 * @return a string representing the key name, or NULL if the action is unknown
 */
const char* getHRKeyFromInputAction(InputManager inputMng, InputAction action);

#endif