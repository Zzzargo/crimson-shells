#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "global.h"
#include "engine.h"

// A game state is described by a set of functions that handle its lifecycle
typedef struct state {
    void (*onEnter)(ZENg);  // load state's assets
    void (*onExit)(ZENg);  // unload state's assets
    Uint8 (*handleEvents)(SDL_Event*, ZENg);  // handle input via events
    void (*handleInput)(ZENg);  // handle continuous input like player movement

    GameStateType type;
    Uint8 isOverlay;  // for short lifetime states like pause state
} GameState;

#define MAX_GAME_STATES 10  // max size of the game state stack

/**
 * Generic function to handle menu navigation input
 * @param event SDL_Event pointer
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleMenuNavigation(SDL_Event *event, ZENg zEngine);

/**
 * Exits the game
 * @param zEngine pointer to the engine
 * @param data unused
 * @note This function just sets the current state to STATE_EXIT
 */
void prepareExit(ZENg zEngine, void *data);

// ============================================ MAIN MENU STATE ========================================================

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
 * Takes care of the events in the main menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleMainMenuEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from main menu to play state
 * @param zEngine pointer to the engine
 */
void mMenuToPlay(ZENg zEngine, void *data);

/**
 * Transition from main menu to options state
 * @param zEngine pointer to the engine
 */
void mMenuToOptions(ZENg zEngine, void *data);

// ============================================= OPTIONS STATE =========================================================

/**
 * Loads the options menu UI components into the ECS
 * @param zEngine pointer to the engine
 */
void onEnterOptionsMenu(ZENg zEngine);

/**
 * Clears the options menu UI components from the ECS
 * @param zEngine pointer to the engine
 */
void onExitOptionsMenu(ZENg zEngine);

/**
 * Takes care of the events in the options menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleOptionsMenuEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from options menu to game options
 * @param zEngine pointer to the engine
 */
void optionsToGameOpt(ZENg zEngine, void *data);

/**
 * Transition from options menu to audio options
 * @param zEngine pointer to the engine
 */
void optionsToAudioOpt(ZENg zEngine, void *data);

/**
 * Transition from options menu to video options
 * @param zEngine pointer to the engine
 */
void optionsToVideoOpt(ZENg zEngine, void *data);

/**
 * Transition from options menu to controls options
 * @param zEngine pointer to the engine
 */
void optionsToControlsOpt(ZENg zEngine, void *data);

/**
 * Transition from options menu to main menu
 * @param zEngine pointer to the engine
 */
void optionsToMMenu(ZENg zEngine, void *data);

// ============================================= VIDEO OPTIONS STATE ===================================================

/**
 * Loads the video options UI components into the ECS
 * @param zEngine pointer to the engine
 */
void onEnterVideoOptions(ZENg zEngine);

/**
 * Deletes the video options UI components from the ECS
 * @param zEngine pointer to the engine
 */
void onExitVideoOptions(ZENg zEngine);

/**
 * Button action to toggle fullscreen/windowed mode
 * @param zEngine pointer to the engine
 * @param data pointer to an Uint8 representing the new window mode (0 = windowed, 1 = fullscreen)
 */
void changeWindowMode(ZENg zEngine, void *data);

/**
 * Button action to change the display resolution
 * @param zEngine pointer to the engine
 * @param data pointer to an SDL_DisplayMode struct describing the new display mode
 */
void changeRes(ZENg zEngine, void *data);

/**
 * Handles the events in the video options menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleVideoOptionsEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from video options to options menu
 * @param zEngine pointer to the engine
 * @param data unused
 */
void videoOptToOpt(ZENg zEngine, void *data);

// ================================================ PLAY STATE =========================================================

/**
 * Loads the initial game entities into the ECS when entering the play state
 * @param zEngine pointer to the engine
 */
void onEnterPlayState(ZENg zEngine);

/**
 * Deletes the game entities
 * @param zEngine pointer to the engine
 */
void onExitPlayState(ZENg zEngine);

// /**
//  * Spawns a tank entity and sets it up with components
//  * 
//  */
// void spawnTank(
//     ZENg zEngine, Uint32 tileIdx, Int32 maxHealth,
// );

/**
 * Instantiates a weapon from a prefab
 * @param zEngine pointer to the engine
 * @param prefab pointer to a weapon prefab
 * @param owner entity ID of the weapon's owner
 * @return freshly allocated weapon
 */
WeaponComponent* instantiateWeapon(ZENg zEngine, WeaponPrefab *prefab, Entity owner);

/**
 * Instantiates a tank from a prefab
 * @param zEngine pointer to the engine
 * @param prefab pointer to a tank prefab
 * @param position initial position of the tank
 * @return entity ID of the newly created tank
 */
Entity instantiateTank(ZENg zEngine, TankPrefab *prefab, Vec2 position);

/**
 * Spawns a bullet type projectile
 * @param zEngine pointer to the engine
 * @param owner the owner entity of the bullet
 * @param bulletW width of the bullet
 * @param bulletH height of the bullet
 * @param speed speed of the bullet
 * @param projComp pointer to the ProjectileComponent describing the bullet's behavior
 * @param lifeTime how long the bullet lasts before disappearing, in seconds
 * @param texture pointer to the SDL_Texture of the bullet
 * @param sound pointer to the Mix_Chunk sound that plays when the bullet is fired
 */
void spawnBulletProjectile(
    ZENg zEngine, Entity owner, int bulletW, int bulletH,
    double_t speed, ProjectileComponent *projComp,
    double_t lifeTime, SDL_Texture *texture, Mix_Chunk *sound
);

/**
 * Handles the events in the play state
 * @param e pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handlePlayStateEvents(SDL_Event *e, ZENg zEngine);

/**
 * Game logic driven through continuous input
 * @param zEngine pointer to the engine
 */
void handlePlayStateInput(ZENg zEngine);

// ================================================ PAUSE STATE ========================================================

/**
 * Loads the pause menu UI components into the ECS
 * @param zEngine pointer to the engine
 */
void onEnterPauseState(ZENg zEngine);

/**
 * Unloads the pause menu UI components from the ECS
 * @param zEngine pointer to the engine
 */
void onExitPauseState(ZENg zEngine);

/**
 * Transitions from the pause state to the play state
 * @param zEngine pointer to the engine
 * @param data unused
 */
void pauseToPlay(ZENg zEngine, void *data);

/**
 * Transitions from the pause state to the main menu
 * @param zEngine pointer to the engine
 * @param data unused
 */
void pauseToMMenu(ZENg zEngine, void *data);

/**
 * Handles events in the pause state
 * @param e pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handlePauseStateEvents(SDL_Event *e, ZENg zEngine);

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