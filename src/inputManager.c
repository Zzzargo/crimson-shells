#include "include/inputManager.h"

void loadKeyBindings(InputManager inputMng, const char *filePath) {
    // look for the file
    FILE *fin = fopen(filePath, "r");
    if (!fin) {
        printf("No config file found in %s. Using defaults\n", filePath);
        setDefaultBindings(inputMng);  // set default bindings if the file does not exist
        return;
    }

    // if the file exists, read the bindings
    char action[64], key[64];
    while (fscanf(fin, "%[^=]=%s\n", action, key) == 2) {
        SDL_Scancode scancode = SDL_GetScancodeFromName(key);
        if (scancode == SDL_SCANCODE_UNKNOWN) {
            printf("Unknown key '%s' for action '%s'\n", key, action);
            continue;
        }
        if (strcmp(action, "MOVE_UP") == 0) {
            inputMng->bindings[INPUT_MOVE_UP] = scancode;
        } else if (strcmp(action, "MOVE_DOWN") == 0) {
            inputMng->bindings[INPUT_MOVE_DOWN] = scancode;
        } else if (strcmp(action, "MOVE_LEFT") == 0) {
            inputMng->bindings[INPUT_MOVE_LEFT] = scancode;
        } else if (strcmp(action, "MOVE_RIGHT") == 0) {
            inputMng->bindings[INPUT_MOVE_RIGHT] = scancode;
        } else if (strcmp(action, "SELECT") == 0) {
            inputMng->bindings[INPUT_SELECT] = scancode;
        } else if (strcmp(action, "BACK") == 0) {
            inputMng->bindings[INPUT_BACK] = scancode;
        } else if (strcmp(action, "INTERACT") == 0) {
            inputMng->bindings[INPUT_INTERACT] = scancode;
        } else if (strcmp(action, "SHOOT") == 0) {
            inputMng->bindings[INPUT_SHOOT] = scancode;
        } else if (strcmp(action, "SPECIAL") == 0) {
            inputMng->bindings[INPUT_SPECIAL] = scancode;
        } else {
            printf("Unknown action '%s'\n", action);
        }
    }
    fclose(fin);
    printf("Key bindings loaded from %s\n", filePath);
}

void saveKeyBindings(InputManager inputMng, const char *filePath) {
    FILE *fout = fopen(filePath, "w");
    if (!fout) {
        printf("Failed to open config file %s for writing\n", filePath);
        return;
    }

    const char *actionNames[INPUT_ACTION_COUNT] = {
        "MOVE_UP",
        "MOVE_DOWN",
        "MOVE_LEFT",
        "MOVE_RIGHT",
        "SELECT",
        "BACK",
        "INTERACT",
        "SHOOT",
        "SPECIAL"
    };

    for (InputAction action = 0; action < INPUT_ACTION_COUNT; action++) {
        fprintf(fout, "%s=%s\n", actionNames[action], SDL_GetScancodeName(inputMng->bindings[action]));
    }
    fclose(fout);
    printf("Key bindings saved to %s\n", filePath);
}

void setDefaultBindings(InputManager inputMng) {
    inputMng->bindings[INPUT_MOVE_UP] = SDL_SCANCODE_W;
    inputMng->bindings[INPUT_MOVE_DOWN] = SDL_SCANCODE_S;
    inputMng->bindings[INPUT_MOVE_LEFT] = SDL_SCANCODE_A;
    inputMng->bindings[INPUT_MOVE_RIGHT] = SDL_SCANCODE_D;
    inputMng->bindings[INPUT_SELECT] = SDL_SCANCODE_RETURN;
    inputMng->bindings[INPUT_BACK] = SDL_SCANCODE_ESCAPE;
    inputMng->bindings[INPUT_INTERACT] = SDL_SCANCODE_SPACE;
    inputMng->bindings[INPUT_SHOOT] = SDL_SCANCODE_J;
    inputMng->bindings[INPUT_SPECIAL] = SDL_SCANCODE_L;
}

Uint8 isActionPressed(InputManager inputMng, InputAction action) {
    return inputMng->keyboardState[inputMng->bindings[action]];
}

InputAction scancodeToAction(InputManager inputMng, SDL_Scancode scancode) {
    for (InputAction i = 0; i < INPUT_ACTION_COUNT; i++) {
        if (inputMng->bindings[i] == scancode) {
            return i;
        }
    }
    return INPUT_UNKNOWN;  // no matching action
}