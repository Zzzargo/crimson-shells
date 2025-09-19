#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "../global/global.h"
#include "../engine/engine.h"

typedef enum {
    STATE_DATA_PLAIN  // Value or array of values, need one free
} StateDataType;

typedef struct {
    void *data;
    StateDataType type;
} StateData;

// A game state is described by a set of functions that handle its lifecycle
typedef struct state {
    void (*onEnter)(ZENg);  // load state's assets
    void (*onExit)(ZENg);  // unload state's assets
    Uint8 (*handleEvents)(SDL_Event*, ZENg);  // handle input via events
    void (*handleInput)(ZENg);  // handle continuous input like player movement

    /**
     * Optional data needed by the state
     * Note: the state data is added by piece, while the deletion is done all at once at state exit
     */
    StateData *stateDataArray;
    size_t stateDataCount;  // number of elements in the stateDataArray
    size_t stateDataCapacity;  // capacity of the stateDataArray

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
 * Transition from main menu to garage state
 * @param zEngine pointer to the engine
 */
void mMenuToGarage(ZENg zEngine, void *data);

/**
 * Transition from main menu to settings state
 * @param zEngine pointer to the engine
 */
void mMenuToSettings(ZENg zEngine, void *data);

// ============================================= GARAGE STATE ===================================================

/**
 * Loads the garage UI and entities
 * @param zEngine pointer to the engine
 */
void onEnterGarage(ZENg zEngine);

/**
 * Unloads the garage UI and entities
 * @param zEngine pointer to the engine
 */
void onExitGarage(ZENg zEngine);

/**
 * Handles the events in the garage state
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleGarageEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from garage to the main menu
 * @param zEngine pointer to the engine
 * @param data unused
 */
void garageToMMenu(ZENg zEngine, void *data);

// ============================================= SETTINGS STATE ========================================================

/**
 * Loads the settings menu UI
 * @param zEngine pointer to the engine
 */
void onEnterSettingsMenu(ZENg zEngine);

/**
 * Clears the settings menu UI components from the ECS
 * @param zEngine pointer to the engine
 */
void onExitSettingsMenu(ZENg zEngine);

/**
 * Takes care of the events in the Settings menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleSettingsMenuEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from settings menu to game Settings
 * @param zEngine pointer to the engine
 */
void settingsToGameSettings(ZENg zEngine, void *data);

/**
 * Transition from settings menu to audio settings
 * @param zEngine pointer to the engine
 */
void settingsToAudioSettings(ZENg zEngine, void *data);

/**
 * Transition from settings menu to video Settings
 * @param zEngine pointer to the engine
 */
void settingsToVideoSettings(ZENg zEngine, void *data);

/**
 * Transition from settings menu to controls settings
 * @param zEngine pointer to the engine
 */
void settingsToControlsSettings(ZENg zEngine, void *data);

/**
 * Transition from settings menu to main menu
 * @param zEngine pointer to the engine
 */
void settingsToMMenu(ZENg zEngine, void *data);

// ============================================= GAME SETTINGS STATE ===================================================

/**
 * Loads the game Settings UI
 * @param zEngine pointer to the engine
 */
void onEnterGameSettings(ZENg zEngine);

/**
 * Unloads the game Settings UI
 * @param zEngine pointer to the engine
 */
void onExitGameSettings(ZENg zEngine);

/**
 * Handles the events in the game Settings menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleGameSettingsEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from game Settings to Settings menu
 * @param zEngine pointer to the engine
 * @param data unused
 */
void gameSettingsToSettings(ZENg zEngine, void *data);

// ============================================= AUDIO SETTINGS STATE ==================================================

/**
 * Loads the audio Settings UI
 * @param zEngine pointer to the engine
 */
void onEnterAudioSettings(ZENg zEngine);

/**
 * Unloads the audio Settings UI
 * @param zEngine pointer to the engine
 */
void onExitAudioSettings(ZENg zEngine);

/**
 * Handles the events in the audio Settings menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the audio, 1 otherwise
 */
Uint8 handleAudioSettingsEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from audio Settings to Settings menu
 * @param zEngine pointer to the engine
 * @param data unused
 */
void audioSettingsToSettings(ZENg zEngine, void *data);

// ============================================= VIDEO SETTINGS STATE ==================================================

/**
 * Loads the video Settings UI
 * @param zEngine pointer to the engine
 */
void onEnterVideoSettings(ZENg zEngine);

/**
 * Deletes the video Settings UI components from the ECS
 * @param zEngine pointer to the engine
 */
void onExitVideoSettings(ZENg zEngine);

/**
 * Provider function to get the available display resolutions
 * It adds them to the parser map and to the current state's data for later freeing
 * @param zEngine pointer to the engine
 * @param map pointer to the parser map where the resolutions will be stored
 */
void getResolutions(ZENg zEngine, ParserMap map);

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
 * Provider function to get the available window modes (windowed/fullscreen)
 * It adds them to the parser map and to the current state's data for later freeing
 * @param zEngine pointer to the engine
 * @param map pointer to the parser map where the window modes will be stored
 */
void getWindowModes(ZENg zEngine, ParserMap map);

/**
 * Handles the events in the video Settings menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the game, 1 otherwise
 */
Uint8 handleVideoSettingsEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from video Settings to Settings menu
 * @param zEngine pointer to the engine
 * @param data unused
 */
void videoSettingsToSettings(ZENg zEngine, void *data);

// ============================================= CONTROLS SETTINGS STATE ===============================================

/**
 * Loads the controls Settings UI
 * @param zEngine pointer to the engine
 */
void onEnterControlsSettings(ZENg zEngine);

/**
 * Unloads the controls Settings UI
 * @param zEngine pointer to the engine
 */
void onExitControlsSettings(ZENg zEngine);

/**
 * Handles the events in the controls Settings menu
 * @param event pointer to the SDL_Event
 * @param zEngine pointer to the engine
 * @return 0 if the event is trying to exit the controls, 1 otherwise
 */
Uint8 handleControlsSettingsEvents(SDL_Event *event, ZENg zEngine);

/**
 * Transition from controls Settings to Settings menu
 * @param zEngine pointer to the engine
 * @param data unused
 */
void controlsSettingsToSettings(ZENg zEngine, void *data);

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
 * Loads the pause menu UI
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
 * Adds data to a GameState's stateDataArray
 * @param state pointer to the GameState
 * @param data pointer to the data to add
 * @param type StateDataType enum value describing the type of data being added
 */
void addStateData(GameState *state, void *data, StateDataType type);

/**
 * Clears all state data from a GameState
 * @param state pointer to the GameState
 */
void clearStateData(GameState *state);

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