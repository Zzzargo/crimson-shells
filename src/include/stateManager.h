#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "global.h"
#include "engine.h"

// Available game states enum
typedef enum {
    STATE_MAIN_MENU,
    STATE_OPTIONS,
    STATE_OPTIONS_AUDIO,
    STATE_OPTIONS_VIDEO,
    STATE_OPTIONS_CONTROLS,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_EXIT,
} GameStateType;

// A game state is described by a set of functions that handle its lifecycle
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

#define MAX_GAME_STATES 10  // max size of the game state stack

/**
 * Generic function to render a menu UI based on the UI ECS entities' components
 * @param zEngine pointer to the engine
 */
void renderMenu(ZENg zEngine);

/**
 * Generic function to handle menu navigation input
 */
Uint8 handleMenuNavigation(SDL_Event *event, ZENg zEngine, char *firstItem, char *lastItem, void (*renderFunc)(ZENg));

/**
 * 
 */
void prepareExit(ZENg zEngine);

// ============================================ MAIN MENU STATE ========================================================

/**
 * 
 */
void mMenuToPlay(ZENg zEngine);

/**
 * 
 */
void mMenuToOptions(ZENg zEngine);

/**
 * Loads the main menu UI components into the UI ECS
 * @param zEngine pointer to the engine
 */
void onEnterMainMenu(ZENg zEngine);

/**
 * Clears the main menu UI components from the ECS
 * @param zEngine pointer to the engine
 */
void onExitMainMenu(ZENg zEngine);

/**
 * Updates the components in the UI ECS after an event
 * @param zEngine pointer to the engine
 */
void updateMenuUI(ZENg zEngine);

/**
 * Takes care of the events in the main menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleMainMenuEvents(SDL_Event *event, ZENg zEngine);

// ============================================= OPTIONS STATE =========================================================

/**
 * 
 */
void optionsToGameOpt(ZENg zEngine);

/**
 * 
 */
void optionsToAudioOpt(ZENg zEngine);

/**
 * 
 */
void optionsToVideoOpt(ZENg zEngine);

/**
 * 
 */
void optionsToControlsOpt(ZENg zEngine);

/**
 * 
 */
void optionsToMMenu(ZENg zEngine);

// Loads the options menu UI components into the ECS
void onEnterOptionsMenu(ZENg zEngine);

// Clears the options menu UI components from the ECS
void onExitOptionsMenu(ZENg zEngine);

// Takes care of the events in the options menu
Uint8 handleOptionsMenuEvents(SDL_Event *event, ZENg zEngine);

// ============================================= VIDEO OPTIONS STATE ===================================================

/**
 * 
 */
void onEnterVideoOptions(ZENg zEngine);

/**
 * 
 */
void onExitVideoOptions(ZENg zEngine);

/**
 * 
 */
Uint8 handleVideoOptionsEvents(SDL_Event *event, ZENg zEngine);

/**
 * 
 */
void videoOptToOpt(ZENg zEngine);

// ================================================ PLAY STATE =========================================================

// Loads the initial game entities into the ECS when entering the play state
void onEnterPlayState(ZENg zEngine);

// Deletes the game entities
void onExitPlayState(ZENg zEngine);

// Spawns a bullet with a given owner entity, in the same direction the owner is looking
void spawnBulletProjectile(ZENg zEngine, Entity owner);

/**
 * Handles the events in the play state
 * @param e pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handlePlayStateEvents(SDL_Event *e, ZENg zEngine);

// Game logic driven through continuous input
void handlePlayStateInput(ZENg zEngine);

// Updates the game logic
void updatePlayStateLogic(ZENg zEngine, double_t deltaTime);

// Renders the in-game entities
void renderPlayState(ZENg zEngine);

// ================================================ PAUSE STATE ========================================================

/**
 * 
 */
void pauseToPlay(ZENg zEngine);

/**
 * 
 */
void pauseToMMenu(ZENg zEngine);

// Updates the pause menu UI components
void updatePauseUI(ZENg zEngine);

/**
 * Handles events in the pause state
 * @param e pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handlePauseStateEvents(SDL_Event *e, ZENg zEngine);

// Renders the pause menu UI
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