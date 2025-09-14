#include "include/uiManager.h"

UIManager initUIManager() {
    UIManager uiManager = calloc(1, sizeof(struct UIManager));
    if (!uiManager) {
        printf("Failed to allocate memory for the UI Manager\n");
        exit(EXIT_FAILURE);
    }

    // Add the root container node
    SDL_Rect root = {0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT};
    UINode *rootContainer = UIcreateContainer(root);
    UIinsertNode(uiManager, NULL, rootContainer);
    return uiManager;
}

/**
 * =====================================================================================================================
 */

void UIinsertNode(UIManager uiManager, UINode *parent, UINode *newNode) {
    if (!uiManager || !newNode) {
        printf("UIManager or newNode is NULL in addUINode\n");
        return;
    }

    if (!parent) {
        // If no parent is provided, add as root
        if (!uiManager->root) {
            uiManager->root = newNode;
        } else {
            printf("UIManager already has a root, cannot add another root node\n");
            return;
        }
    } else {
        // Add as a child of the specified parent
        if (parent->childrenCount >= parent->childrenCapacity) {
            // Make space for one more child
            parent->childrenCapacity++;
            UINode **newChildren = realloc(
                parent->children,
                parent->childrenCapacity * sizeof(UINode*)
            );
            if (!newChildren) {
                fprintf(stderr, "Failed to resize UI Node's children array\n");
                return;
            }
            parent->children = newChildren;
        }
        parent->children[parent->childrenCount] = newNode;
        newNode->parent = parent;
        newNode->siblingIndex = parent->childrenCount++;
    }
    #ifdef DEBUG
        printf("Added new UI node of type %d to parent node %d\n", newNode->type, parent ? parent->type : -1);
    #endif
}

/**
 * =====================================================================================================================
 */

void UIdeleteNode(UIManager uiManager, UINode *node) {
    if (!uiManager || !node) {
        printf("UIManager or node is NULL in removeUINode\n");
        return;
    }

        if (node->children) {
        // Recursively delete all children
        for (size_t i = node->childrenCount; i > 0; i--) {
            UIdeleteNode(uiManager, node->children[i - 1]);
        }
        free(node->children);
        node->children = NULL;
        node->childrenCount = 0;
        node->childrenCapacity = 0;
    }
    if (node->parent) {
        // Shift the other children to maintain order
        for (size_t i = node->siblingIndex; i < node->parent->childrenCount - 1; i++) {
            node->parent->children[i] = node->parent->children[i + 1];
            node->parent->children[i]->siblingIndex = i;
        }
        node->parent->childrenCount--;
    }
    switch (node->type) {
        case UI_CONTAINER: {
            free(((UIContainer *)(node->widget))->rect);
            break;
        }
        case UI_LABEL: {
            free(((UILabel *)(node->widget))->text);
            free(((UILabel *)(node->widget))->destRect);
            SDL_DestroyTexture(((UILabel *)(node->widget))->texture);
            break;
        }
        case UI_BUTTON: {
            free(((UIButton *)(node->widget))->text);
            free(((UIButton *)(node->widget))->destRect);
            SDL_DestroyTexture(((UIButton *)(node->widget))->texture);
            break;
        }
        case UI_OPTION_CYCLE: {
            free(((UIOptionCycle *)(node->widget))->rect);
            break;
        }
    }
    free(node->widget);
    free(node);
}

/**
 * =====================================================================================================================
 */

void UIclear(UIManager uiManager) {
    if (!uiManager) {
        printf("UIManager is NULL in UIclear\n");
        return;
    }

    // Recursively delete all nodes except the root
    if (uiManager->root) {
        for (size_t i = uiManager->root->childrenCount; i > 0; i--) {
            UIdeleteNode(uiManager, uiManager->root->children[i - 1]);
        }
        free(uiManager->root->children);
        uiManager->root->children = NULL;
        uiManager->root->childrenCount = 0;
        uiManager->root->childrenCapacity = 0;
    }

    uiManager->focusedNode = NULL;
    uiManager->dirtyCount = 0;
}

/**
 * =====================================================================================================================
 */

void UIclose(UIManager uiManager) {
    if (!uiManager) {
        printf("UIManager is NULL in UIclose\n");
        return;
    }

    // Free the tree root and all its children
    UIdeleteNode(uiManager, uiManager->root);

    // Free the dirty nodes array
    free(uiManager->dirtyNodes);

    // Free the UIManager itself
    free(uiManager);
}

/**
 * =====================================================================================================================
 */

void UImarkNodeDirty(UIManager uiManager, UINode *node) {
    if (!uiManager || !node) {
        fprintf(stderr, "UIManager or node is NULL, cannot mark node dirty\n");
        return;
    }


    if (uiManager->dirtyCount >= uiManager->dirtyCapacity) {
        // Double the capacity if it is full, begin with 4
        uiManager->dirtyCapacity = uiManager->dirtyCapacity == 0 ? 4 : uiManager->dirtyCapacity * 2;
        UINode **newDirtyNodes = realloc(
            uiManager->dirtyNodes,
            uiManager->dirtyCapacity * sizeof(UINode*)
        );
        if (!newDirtyNodes) {
            fprintf(stderr, "Failed to resize dirty UI nodes array\n");
            return;
        }
        uiManager->dirtyNodes = newDirtyNodes;
    }

    uiManager->dirtyNodes[uiManager->dirtyCount++] = node;
    node->isDirty = 1;
}

/**
 * =====================================================================================================================
 */

void UIunmarkNodeDirty(UIManager uiManager) {
    if (!uiManager) {
        fprintf(stderr, "UIManager is NULL, cannot unmark node dirty\n");
        return;
    }

    UINode *node = uiManager->dirtyNodes[0];
    if (!node) {
        fprintf(stderr, "No dirty nodes to unmark\n");
        return;
    }
    node->isDirty = 0;

    // Swap with the last dirty node and decrease the count
    uiManager->dirtyNodes[0] = uiManager->dirtyNodes[--uiManager->dirtyCount];
    uiManager->dirtyNodes[uiManager->dirtyCount] = NULL;  // Shouldn't be necessary but just in case
}

/**
 * =====================================================================================================================
 */

void UIrenderNode(SDL_Renderer *rdr, UINode *node) {
    if (!node) return;

    // Render this node based on its type
    if (node->isVisible == 0) return;
    switch(node->type) {
        case UI_CONTAINER: {
            break;
        }
        case UI_LABEL: {
            UILabel *label = (UILabel *)(node->widget);
            if (label->texture) {
                SDL_RenderCopy(rdr, label->texture, NULL, label->destRect);
            }
            break;
        }
        case UI_BUTTON: {
            UIButton *button = (UIButton *)(node->widget);
            if (button->texture) {
                SDL_RenderCopy(rdr, button->texture, NULL, button->destRect);
            }
            break;
        }
        case UI_OPTION_CYCLE: {
            UIOptionCycle *optionCycle = (UIOptionCycle *)(node->widget);
            if (optionCycle->currOption && optionCycle->currOption->data) {
                UIButton *btn = (UIButton *)optionCycle->currOption->data;
                if (btn->texture) SDL_RenderCopy(rdr, btn->texture, NULL, btn->destRect);
            }
            break;
        }
    }

    // Depth-first
    UIrenderNode(rdr, node->children ? node->children[0] : NULL);
    // Then breadth-wise
    if (node->parent) {
        for (size_t i = node->siblingIndex + 1; i < node->parent->childrenCount; i++) {
            UIrenderNode(rdr, node->parent->children[i]);
        }
    }
}

/**
 * =====================================================================================================================
 */

void UIrender(UIManager uiManager, SDL_Renderer *rdr) {
    if (!uiManager || !rdr) {
        printf("UIManager or renderer is NULL in UIrender\n");
        return;
    }
    if (uiManager->root) {
        // Start rendering from the root node
        UIrenderNode(rdr, uiManager->root);
    }
}

/**
 * =====================================================================================================================
 */

void UIrefocus(UIManager uiManager, UINode *newFocus) {
    // Defocus the current if any
    if (uiManager->focusedNode) {
        uiManager->focusedNode->state = UI_STATE_NORMAL;
        UImarkNodeDirty(uiManager, uiManager->focusedNode);
        switch (uiManager->focusedNode->type) {
            case UI_BUTTON: {
                UIButton *button = (UIButton *)(uiManager->focusedNode->widget);
                button->currColor = button->colors[UI_STATE_NORMAL];
                break;
            }
        }
    }

    // Focus on the new node
    if (!newFocus) {
        uiManager->focusedNode = NULL;
        return;
    }
    uiManager->focusedNode = newFocus;
    uiManager->focusedNode->state = UI_STATE_FOCUSED;
    UImarkNodeDirty(uiManager, uiManager->focusedNode);
    switch (uiManager->focusedNode->type) {
        case UI_BUTTON: {
            UIButton *button = (UIButton *)(uiManager->focusedNode->widget);
            button->currColor = button->colors[UI_STATE_FOCUSED];
            break;
        }
    }
}

/**
 * =====================================================================================================================
 */

Uint8 UIisNodeFocusable(UINode *node) {
    if (!node) return 0;
    switch (node->type) {
        case UI_BUTTON:
            return 1;
        default:
            return 0;
    }
}

/**
 * =====================================================================================================================
 */

UINode* UIcreateContainer(SDL_Rect rect) {
    UINode *container = calloc(1, sizeof(UINode));
    if (!container) {
        printf("Failed to allocate memory for the root UI node\n");
        exit(EXIT_FAILURE);
    }
    container->type = UI_CONTAINER;
    container->isDirty = 1;  // Dirty by default
    container->isVisible = 1;
    container->widget = calloc(1, sizeof(UIContainer));
    if (!container->widget) {
        printf("Failed to allocate memory for the container widget\n");
        free(container);
        exit(EXIT_FAILURE);
    }
    SDL_Rect *contRect = calloc(1, sizeof(SDL_Rect));
    if (!contRect) {
        printf("Failed to allocate memory for the container rect\n");
        free(container->widget);
        free(container);
        exit(EXIT_FAILURE);
    }
    *contRect = rect;
    ((UIContainer *)(container->widget))->rect = contRect;
    return container;
}

/**
 * =====================================================================================================================
 */

UINode* UIcreateLabel(SDL_Renderer *rdr, TTF_Font *font, char *text, SDL_Color color) {
    UILabel *label = calloc(1, sizeof(UILabel));
    if (!label) {
        printf("Failed to allocate memory for label\n");
        exit(EXIT_FAILURE);
    }

    label->font = font;
    label->text = text;
    label->currColor = color;

    SDL_Surface *surface = TTF_RenderText_Solid(label->font, label->text, label->currColor);
    if (!surface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    label->texture = SDL_CreateTextureFromSurface(rdr, surface);
    if (!label->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    label->destRect = calloc(1, sizeof(SDL_Rect));
    if (!label->destRect) {
        printf("Failed to allocate memory for text rectangle\n");
        exit(EXIT_FAILURE);
    }

    *label->destRect = (SDL_Rect) {
        .x = 0,
        .y = 0,
        .w = surface->w,
        .h = surface->h
    };

    SDL_FreeSurface(surface);
    UINode *node = calloc(1, sizeof(UINode));
    if (!node) {
        printf("Failed to allocate memory for label node\n");
        exit(EXIT_FAILURE);
    }
    node->isDirty = 1;  // Dirty by default
    node->type = UI_LABEL;
    node->isVisible = 1;
    node->widget = (void *)label;
    return node;
}

/**
 * =====================================================================================================================
 */

UINode* UIcreateButton(
    SDL_Renderer *rdr, TTF_Font *font, char *text, UIState state, SDL_Color colors[UI_STATE_COUNT],
    void (*onClick)(ZENg, void*), void *data
) {
    UIButton *button = calloc(1, sizeof(UIButton));
    if (!button) {
        printf("Failed to allocate memory for button UI element\n");
        exit(EXIT_FAILURE);
    }

    button->onClick = onClick;
    button->data = data;
    button->font = font;
    button->text = text;
    for (UIState s = 0; s < UI_STATE_COUNT; s++) {
        button->colors[s] = colors[s];
    }
    button->currColor = colors[state];

    SDL_Surface *titleSurface = TTF_RenderText_Solid(button->font, button->text, button->currColor);
    if (!titleSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    button->texture = SDL_CreateTextureFromSurface(rdr, titleSurface);
    if (!button->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    button->destRect = calloc(1, sizeof(SDL_Rect));
    if (!button->destRect) {
        printf("Failed to allocate memory for button rectangle\n");
        exit(EXIT_FAILURE);
    }

    *button->destRect = (SDL_Rect) {
        .x = 0,
        .y = 0,
        .w = titleSurface->w,
        .h = titleSurface->h
    };

    SDL_FreeSurface(titleSurface);  // we don't need the surface anymore

    UINode *node = calloc(1, sizeof(UINode));
    if (!node) {
        printf("Failed to allocate memory for button node\n");
        exit(EXIT_FAILURE);
    }
    node->isDirty = 1;  // Dirty by default
    node->type = UI_BUTTON;
    node->state = state;
    node->isVisible = 1;
    node->widget = (void *)button;
    return node;
}
