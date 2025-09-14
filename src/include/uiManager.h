#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "global.h"

// Here begins my journey into hell

typedef enum {
    UI_CONTAINER,
    UI_LABEL,
    UI_BUTTON,
    UI_OPTION_CYCLE,
    UI_TYPE_COUNT  // Automatically counts
} UIType;

typedef enum {
    UI_STATE_NORMAL,
    UI_STATE_FOCUSED,  // One only
    UI_STATE_SELECTED,  // Could be multiple, usually branches
    UI_STATE_COUNT  // Automatically counts
} UIState;

typedef struct UIContainer {
    SDL_Rect *rect;
} UIContainer;

typedef struct UILabel {
    char *text;
    TTF_Font *font;  // Font to use for rendering text
    SDL_Rect *destRect;  // Where to render the text
    SDL_Texture *texture;  // Texture for the text
    SDL_Color currColor;  // Color of the text
} UILabel;

typedef struct UIButton {
    void (*onClick)(ZENg, void *);  // What the button does
    char *text;
    TTF_Font *font;  // Font used by the button text
    SDL_Rect *destRect;  // Where to render the button
    SDL_Texture *texture;
    void *data;  // Pointer to any data the button might need
    SDL_Color colors[UI_STATE_COUNT];  // Color of the button in different states
    SDL_Color currColor;  // Current color of the button
} UIButton;

typedef struct UIOptionCycle {
    SDL_Rect *rect;
    CDLLNode *currOption;  // Circular doubly linked list of whatever option style you want
    UIType optionType;  // Defines the option style
} UIOptionCycle;

// This boi right there is the base of the UI tree
typedef struct UINode {
    void *widget;  // Pointer to the actual UI element

    UIType type;  // Tells what kind of widget this node is
    UIState state;  // Current state of the UI element the node represents
    Uint8 isDirty;  // Indicates if the UI element the node represents needs to be updated
    Uint8 isVisible;  // Indicates if the UI element the node represents is visible
    
    struct UINode *parent;
    struct UINode **children;
    size_t siblingIndex;  // Index of this node in the parent's children array
    size_t childrenCount;
    size_t childrenCapacity;
} UINode;

// The UIManager is the root of the UI tree
typedef struct UIManager {
    UINode *root;
    UINode *focusedNode;  // The node that is currently under focus (for navigation)

    UINode **dirtyNodes;  // Array of pointers to dirty nodes
    Uint64 dirtyCount;  // Number of dirty nodes
    Uint64 dirtyCapacity;  // Capacity of the dirty nodes array
} *UIManager;

/**
 * Initialises the UI manager
 * @return UIManager = struct UIManager*
 */
UIManager initUIManager();

/**
 * Adds a new UI node to the UI tree
 * @param uiManager the UI manager = struct UIManager*
 * @param parent the parent node to which the new node will be added as a child
 * @param newNode the new node to be added
 * @note if the root is NULL, the new node becomes the root
 */
void UIinsertNode(UIManager uiManager, UINode *parent, UINode *newNode);

/**
 * Removes an UI node from the UI tree
 * @param uiManager the UI manager = struct UIManager*
 * @param node the node to be removed
 */
void UIdeleteNode(UIManager uiManager, UINode *node);

/**
 * Clears the UI tree, leaving only the root container
 * @param uiManager the UI manager = struct UIManager*
 */
void UIclear(UIManager uiManager);

/**
 * Closes the UI manager and frees all associated memory
 * @param uiManager the UI manager = struct UIManager*
 */
void UIclose(UIManager uiManager);

/**
 * Marks a UI node as dirty, so it will be updated in the next frame
 * @param uiManager the UI manager = struct UIManager*
 * @param node the node to be marked as dirty
 */
void UImarkNodeDirty(UIManager uiManager, UINode *node);

/**
 * Unmarks a UI node as dirty
 * @param uiManager the UI manager = struct UIManager*
 * @note the first node in the array is marked clean
 */
void UIunmarkNodeDirty(UIManager uiManager);

/**
 * Renders an UI tree from a root node
 * @param rdr the SDL_Renderer
 * @param node the root node of the UI tree to be rendered
 */
void UIrenderNode(SDL_Renderer *rdr, UINode *node);

/**
 * Renders the UI managed by the UI manager
 * @param uiManager the UI manager = struct UIManager*
 * @param rdr the SDL_Renderer
 */
void UIrender(UIManager uiManager, SDL_Renderer *rdr);

/**
 * Refocuses the UI to a new node
 * @param uiManager the UI manager = struct UIManager*
 * @param newFocus the node to be focused
 * @note the previously focused node is defocused
 */
void UIrefocus(UIManager uiManager, UINode *newFocus);

/**
 * Checks if a UI node is focusable
 * @param node the node to be checked
 * @return 1 if the node is focusable, 0 otherwise
 */
Uint8 UIisNodeFocusable(UINode *node);

/**
 * Creates a container UI node
 * @param rect the rectangle defining the container's position and size
 * @return UINode* = pointer to the created container node
 * @note the returned container is created @ 0x0, so further positioning is needed
 */
UINode* UIcreateContainer(SDL_Rect rect);

/**
 * Creates a label UI node
 * @param rdr the SDL_Renderer used to create the text texture
 * @param font font to be used for the label text
 * @param text label text
 * @param color label color
 * @return UINode* = pointer to the created label node
 * @note the returned label is created @ 0x0, so further positioning is needed
 */
UINode* UIcreateLabel(SDL_Renderer *rdr, TTF_Font *font, char *text, SDL_Color color);

/**
 * Creates a button UI node
 * @param rdr the SDL_Renderer used to create the text texture
 * @param font font to be used for the button text
 * @param text button text
 * @param state button state
 * @param colors button colors
 * @param onClick function to be called when the button is clicked
 * @param data pointer to any data the button might need
 * @return UINode* = pointer to the created button node
 * @note the returned button is created @ 0x0, so further positioning is needed
 */
UINode* UIcreateButton(
    SDL_Renderer *rdr, TTF_Font *font, char *text, UIState state, SDL_Color colors[UI_STATE_COUNT],
    void (*onClick)(ZENg, void*), void *data
);

#endif