#include "inputManager.h"
#include "global/debug.h"

void saveKeyBindings(InputManager inputMng, const char *filePath) {
    FILE *fout = fopen(filePath, "w");
	ASSERT(fout != NULL, "Failed to open config file %s for writing\n", filePath);

    const char *actionNames[INPUT_ACTION_COUNT] = {
        "MOVE_UP",
        "MOVE_DOWN",
        "MOVE_LEFT",
        "MOVE_RIGHT",
        "SELECT",
        "BACK",
        "INTERACT",
        "SHOOT",
        "SECONDARY_SHOOT",
        "SWITCH_RIGHT",
        "SWITCH_LEFT",
        "SPECIAL"
    };

    fprintf(fout, "[INPUT]\n");

    for (InputAction action = 0; action < INPUT_ACTION_COUNT - 1; action++) {
        fprintf(fout, "%s=%s\n", actionNames[action], SDL_GetScancodeName(inputMng->bindings[action]));
    }
    fclose(fout);
    LOG(DEBUG, "Key bindings saved to %s\n", filePath);
}

/**
 * =====================================================================================================================
 */

void setDefaultBindings(InputManager inputMng) {
    if (!inputMng) {
        printf("Input manager is NULL, cannot set default settings\n");
        return;
    }
    inputMng->bindings[INPUT_MOVE_UP] = SDL_SCANCODE_W;
    inputMng->bindings[INPUT_MOVE_DOWN] = SDL_SCANCODE_S;
    inputMng->bindings[INPUT_MOVE_LEFT] = SDL_SCANCODE_A;
    inputMng->bindings[INPUT_MOVE_RIGHT] = SDL_SCANCODE_D;
    inputMng->bindings[INPUT_SELECT] = SDL_SCANCODE_RETURN;
    inputMng->bindings[INPUT_BACK] = SDL_SCANCODE_ESCAPE;
    inputMng->bindings[INPUT_INTERACT] = SDL_SCANCODE_SPACE;
    inputMng->bindings[INPUT_SHOOT] = SDL_SCANCODE_J;
    inputMng->bindings[INPUT_SECONDARY] = SDL_SCANCODE_K;
    inputMng->bindings[INPUT_SWITCH_RIGHT] = SDL_SCANCODE_E;
    inputMng->bindings[INPUT_SWITCH_LEFT] = SDL_SCANCODE_Q;
    inputMng->bindings[INPUT_SPECIAL] = SDL_SCANCODE_L;
}

/**
 * =====================================================================================================================
 */

Uint8 isActionPressed(InputManager inputMng, InputAction action) {
    return inputMng->keyboardState[inputMng->bindings[action]];
}

/**
 * =====================================================================================================================
 */

InputAction scancodeToAction(InputManager inputMng, SDL_Scancode scancode) {
    for (InputAction i = 0; i < INPUT_ACTION_COUNT; i++) {
        if (inputMng->bindings[i] == scancode) {
            return i;
        }
    }
    return INPUT_UNKNOWN;  // no matching action
}

const char* getHRKeyFromInputAction(InputManager inputMng, InputAction action) {
    if (!inputMng || action < 0 || action >= INPUT_ACTION_COUNT) {
        return "Unknown";
    }
    
    SDL_Scancode scancode = inputMng->bindings[action];
    const char* keyHRName = SDL_GetKeyName(SDL_GetKeyFromScancode(scancode));
    
    // If empty string returned, provide a fallback
    if (!keyHRName || keyHRName[0] == '\0') {
        return "Unknown";
    }

    return keyHRName;
}
