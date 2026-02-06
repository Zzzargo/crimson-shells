#include "stateManager.h"

#include "global/debug.h"

void initStateManager(StateManager *stateMng) {
    *stateMng = calloc(1, sizeof(struct statemng));
    ASSERT(stateMng != NULL, "Failed to allocate memory for StateManager\n");
}

/**
 * =====================================================================================================================
*/

void clearStateData(GameState *state) {
    ASSERT(state != NULL, "");
	if (!state->stateData)
		return;

    HashMap map = state->stateData;

    for (size_t i = 0; i < map->size; i++) {
        MapEntry *entry = map->entries[i];
        while (entry) {
            MapEntry *next = entry->next;
            // switch (entry->type) {
            //
            // }
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
            LOG(DEBUG, "Called onExit for state %d\n", curr->type);
        }

        zEngine->stateMng->states[zEngine->stateMng->top++] = state;
        if (state->onEnter) {
            state->onEnter(zEngine);
            LOG(DEBUG, "Called onEnter for state %d\n", state->type);
        }

        LOG(DEBUG, "Exited state %d, entered state %d\n", curr ? curr->type : -1, state->type);
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
            LOG(DEBUG, "Called onExit for state %d\n", currState->type);
        }

        // and call onEnter for the new top if the popped state was not an overlay
        GameState *newState = getCurrState(zEngine->stateMng);
        if (newState && !currState->isOverlay && newState->onEnter) {
            newState->onEnter(zEngine);
            LOG(DEBUG, "Called onEnter for state %d\n", newState->type);
        }


        LOG(DEBUG, "Exited state %d, entered state %d\n", currState->type, newState->type);
        
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

