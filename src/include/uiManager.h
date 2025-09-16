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

typedef enum {
    UI_LAYOUT_NONE,
    UI_LAYOUT_VERTICAL,
    UI_LAYOUT_HORIZONTAL,
    UI_LAYOUT_GRID
} UILayoutType;

typedef enum {
    UI_ALIGNMENT_ABSOLUTE,  // Default, rect stays where it's been defined
    UI_ALIGNMENT_START,
    UI_ALIGNMENT_CENTER,
    UI_ALIGNMENT_END
} UIAlignmentType;

typedef struct {
    UIAlignmentType v, h;
} UIAlignment;

// Padding values are in percentage of the parent's size
typedef struct {
    float top;
    float bottom;
    float left;
    float right;
} UIPadding;

typedef struct {
    UILayoutType type;
    UIAlignment alignment;

    float spacing;  // Space between child elements, in percentage of the parent's size
    UIPadding padding;
} UILayout;


// This boi right there is the base of the UI tree
typedef struct UINode {
    void *widget;  // Pointer to the actual UI element
    SDL_Rect *rect;  // Position and size of the UI element the node represents

    UIType type;  // Tells what kind of widget this node is
    UIState state;  // Current state of the UI element the node represents
    Uint8 isDirty;  // Indicates if the UI element the node represents needs to be updated
    Uint8 isVisible;  // Indicates if the UI element the node represents is visible
    
    UILayout *layout;
    struct UINode *parent;
    struct UINode **children;
    size_t siblingIndex;  // Index of this node in the parent's children array
    size_t childrenCount;
    size_t childrenCapacity;
} UINode;

typedef struct UIContainer {
    // Everything a container can describe is in the layout
} UIContainer;

typedef struct UILabel {
    char *text;
    TTF_Font *font;  // Font to use for rendering text
    SDL_Texture *texture;  // Texture for the text
    SDL_Color currColor;  // Color of the text
} UILabel;

typedef struct UIButton {
    void (*onClick)(ZENg, void *);  // What the button does
    char *text;
    TTF_Font *font;  // Font used by the button text
    SDL_Texture *texture;
    void *data;  // Pointer to any data the button might need
    SDL_Color colors[UI_STATE_COUNT];  // Color of the button in different states
    SDL_Color currColor;  // Current color of the button
} UIButton;

/**
 * An option cycle is basically a list of options that can be cycled through, controlled by a button node
 * Both elements are contained in this struct, so it can be viewed as a special container
 */
typedef struct UIOptionCycle {
    UINode *selector;  // Button that shows the name of the option and applies it. Used the node for the rect
    CDLLNode *currOption;  // Circular doubly linked list of whatever option style you want. Data is UINode*
} UIOptionCycle;

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
 * Adds a child node to a parent node
 * @param parent the parent node to which the child will be added
 * @param child the child node to be added
 */
void UIaddChild(UINode *parent, UINode *child);

/**
 * Removes a child node from a parent node
 * @param parent the parent node from which the child will be removed
 * @param child the child node to be removed
 */
void UIremoveChild(UINode *parent, UINode *child);

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
 * Applies the layout of a container node to its children
 * @param node the container node whose layout is to be applied
 */
void UIapplyLayout(UINode *node);

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
 * Creates an UI layout
 * @param type layout type
 * @param padding paddings for the child elements of the element that has this layout
 * @param alignment alignment of the child elements of the element that has this layout
 * @param spacing spacing for the child elements of the element that has this layout
 */
UILayout* UIcreateLayout(UILayoutType type, UIPadding padding, UIAlignment alignment, float spacing);

/**
 * Creates a container UI node
 * @param rect the rectangle defining the container's position and size
 * @param layout the layout to be used for the container
 * @return UINode* = pointer to the created container node
 * @note the returned container is created @ 0x0, so further positioning is needed
 */
UINode* UIcreateContainer(SDL_Rect rect, UILayout *layout);

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

/**
 * Creates an option cycle UI node
 * @param rect the option cycle container's rectangle
 * @param layout the layout to be used for the option cycle container
 * @param selectorBtn button UI node that shows the name of the option and applies it
 * @param currOption circular doubly linked list of whatever option style you want
 * @return UINode* = pointer to the created option cycle node
 * @note the returned option cycle is created @ 0x0, so further positioning is needed
 */
UINode* UIcreateOptionCycle(SDL_Rect rect, UILayout *layout, UINode *selectorBtn, CDLLNode *currOption);

#endif  // UI_MANAGER_H