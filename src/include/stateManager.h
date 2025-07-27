#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "global.h"
#include "engine.h"

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_EXIT
} GameStateType;

typedef struct {
    void (*onEnter)(ZENg);  // load state's assets
    void (*onExit)(ZENg);  // unload state's assets
    Uint8 (*handleEvents)(SDL_Event*, ZENg);  // handle input via events
    void (*handleInput)(ZENg);  // handle continuous input like player movement
    void (*update)(ZENg, double_t);  // update game logic like physics
    void (*render)(ZENg);  // render current frame
    GameStateType type;
    Uint8 isOverlay;  // for short lifetime states like pause
} GameState;

#define MAX_GAME_STATES 5

// ============================================ MAIN MENU STATE ========================================================

// loads the main menu UI components into the ECS
void onEnterMainMenu(ZENg zEngine);

// clears the main menu UI components from the ECS
void onExitMainMenu(ZENg zEngine);

// updates the components in the UI ECS
void updateMenuUI(ZENg zEngine);

// takes care of the events in the main menu
Uint8 handleMainMenuEvents(SDL_Event *event, ZENg zEngine);

// takes care of the rendering part in the main menu
void renderMainMenu(ZENg zEngine);

// ================================================ PLAY STATE =========================================================

// loads the initial game entities into the ECS when entering the play state
void onEnterPlayState(ZENg zEngine);

// deletes the game entities
void onExitPlayState(ZENg zEngine);

// spawns a bullet with a given owner entity, in the same direction the owner is looking
void spawnBulletProjectile(ZENg zEngine, Entity owner);

// handles in-game events
Uint8 handlePlayStateEvents(SDL_Event *e, ZENg zEngine);

// game logic driven through continuous input
void handlePlayStateInput(ZENg zEngine);

//
void updatePlayStateLogic(ZENg zEngine, double_t deltaTime);

// renders the in-game entities
void renderPlayState(ZENg zEngine);

// ================================================ PAUSE STATE ========================================================

void updatePauseUI(ZENg zEngine);

Uint8 handlePauseStateEvents(SDL_Event *e, ZENg zEngine);

void renderPauseState(ZENg zEngine);

// =====================================================================================================================

typedef struct statemng {
    GameState *states[MAX_GAME_STATES];  // stack of game states
    Uint32 top;  // index of the top of the stack
} *StateManager;

/**
 * Initializes the state manager
 * @param stateMng pointer to StateManager = struct statemng**
 * @return void
 */
void initStateManager(StateManager *stateMng);

/**
 * Pushes a new state onto the state manager's stack
 * @param stateMng StateManager = struct statemng*
 * @param state pointer to the new GameState to push
 */
void pushState(ZENg zEngine, GameState *state);

/**
 * Removes a state from the top of the stack
 * @param zEngine ZENg = struct engine*
 */
void popState(ZENg zEngine);

/**
 * Gets the current state the game is in
 * @param stateMng StateManager = struct statemng*
 * @return Pointer to GameState struct
 */
GameState* getCurrState(StateManager stateMng);

#endif // STATE_MANAGER_H