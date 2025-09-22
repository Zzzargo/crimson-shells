#include "stateManager.h"

void initStateManager(StateManager *stateMng) {
    *stateMng = calloc(1, sizeof(struct statemng));
    if (!*stateMng) {
        fprintf(stderr, "Failed to allocate memory for StateManager\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * =====================================================================================================================
*/

void clearStateData(GameState *state) {
    if (!state) THROW_ERROR_AND_RETURN_VOID("State is NULL. Can't free data");
    HashMap map = state->stateData;
    if (!map) THROW_ERROR_AND_RETURN_VOID("Map is NULL. Can't free");

    for (size_t i = 0; i < map->size; i++) {
        MapEntry *entry = map->entries[i];
        while (entry) {
            MapEntry *next = entry->next;
            switch (entry->type) {
                
            }
            free(entry->key);  // Free the key string
            free(entry);  // Free the entry itself
            entry = next;  // Move to the next entry in the chain
        }
    }

    free(map->entries);
    free(map);
}

/**
 * =====================================================================================================================
 */

void pushState(ZENg zEngine, GameState *state) {
    if (zEngine->stateMng->top < MAX_GAME_STATES) {
        GameState *curr = getCurrState(zEngine->stateMng);
        // call onExit if the pushed state is not an overlay
        if (curr && !state->isOverlay && curr->onExit) {
            curr->onExit(zEngine);
            #ifdef DEBUG
                printf("Called onExit for state %d\n", curr->type);
            #endif
        }

        zEngine->stateMng->states[zEngine->stateMng->top++] = state;
        if (state->onEnter) {
            state->onEnter(zEngine);
            #ifdef DEBUG
                printf("Called onEnter for state %d\n", state->type);
            #endif
        }

        #ifdef DEBUG
            printf("Exited state %d, entered state %d\n", curr ? curr->type : -1, state->type);
        #endif
    }
}

/**
 * =====================================================================================================================
 */

void popState(ZENg zEngine) {
    if (zEngine->stateMng->top > 0) {
        // pop the top and call onExit
        GameState *currState = zEngine->stateMng->states[--zEngine->stateMng->top];
        clearStateData(currState);
        if (currState->onExit) {
            currState->onExit(zEngine);
            #ifdef DEBUG
                printf("Called onExit for state %d\n", currState->type);
            #endif
        }

        // and call onEnter for the new top if the popped state was not an overlay
        GameState *newState = getCurrState(zEngine->stateMng);
        if (newState && !currState->isOverlay && newState->onEnter) {
            newState->onEnter(zEngine);
            #ifdef DEBUG
                printf("Called onEnter for state %d\n", newState->type);
            #endif
        }

        
        #ifdef DEBUG
            printf("Exited state %d, entered state %d\n", currState->type, newState->type);
        #endif
        
        free(currState);
        currState = NULL;  // clear the pointer to avoid dangling pointer issues
    }
}

/**
 * =====================================================================================================================
 */

GameState* getCurrState(StateManager stateMng) {
    if (stateMng->top == 0) return NULL;
    return stateMng->states[stateMng->top - 1];
}

