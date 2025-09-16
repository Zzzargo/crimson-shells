#include "include/stateManager.h"

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

void addStateData(GameState *state, void *data, StateDataType type) {
    if (!state) {
        fprintf(stderr, "Cannot add state data to a NULL GameState\n");
        return;
    }

    if (state->stateDataCount >= state->stateDataCapacity) {
        // Resize the array
        size_t newCapacity = state->stateDataCapacity + 1;
        StateData *newArray = realloc(state->stateDataArray, newCapacity * sizeof(StateData));
        if (!newArray) {
            fprintf(stderr, "Failed to resize state data array\n");
            return;
        }
        state->stateDataArray = newArray;
        state->stateDataCapacity = newCapacity;
    }
    state->stateDataArray[state->stateDataCount].data = data;
    state->stateDataArray[state->stateDataCount].type = type;
    state->stateDataCount++;
}

void clearStateData(GameState *state) {
    if (!state || !state->stateDataArray) return;

    for (size_t i = 0; i < state->stateDataCount; i++) {
        if (state->stateDataArray[i].data) {
            switch (state->stateDataArray[i].type) {
                case STATE_DATA_PLAIN:
                    free(state->stateDataArray[i].data);
                    break;
                default:
                    fprintf(stderr, "Unknown StateDataType %d, cannot free data\n", state->stateDataArray[i].type);
                    break;
            }
        }
    }
    free(state->stateDataArray);
    state->stateDataArray = NULL;
    state->stateDataCount = 0;
    state->stateDataCapacity = 0;
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

