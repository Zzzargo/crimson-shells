#include "include/uiManager.h"

UIManager initUIManager() {
    UIManager uiManager = calloc(1, sizeof(struct UIManager));
    if (!uiManager) {
        printf("Failed to allocate memory for the UI Manager\n");
        exit(EXIT_FAILURE);
    }

    // Add the root container node
    SDL_Rect root = {0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT};
    UILayout *rootLayout = UIcreateLayout(
        UI_LAYOUT_VERTICAL, (UIPadding){.bottom = 0.0, .left = 0.0, .right = 0.0, .top = 0.0},
        (UIAlignment){.h = UI_ALIGNMENT_ABSOLUTE, .v = UI_ALIGNMENT_ABSOLUTE}, 0.0
    );  // Defaults
    UINode *rootContainer = UIcreateContainer(root, rootLayout);
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
        UIaddChild(parent, newNode);
    }
    #ifdef DEBUG
        printf("Added new UI node of type %d to parent node %d\n", newNode->type, parent ? parent->type : -1);
    #endif
}

/**
 * =====================================================================================================================
 */

void UIaddChild(UINode *parent, UINode *child) {
    if (!parent || !child) {
        printf("Parent or child is NULL in UIaddChild\n");
        return;
    }

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
    parent->children[parent->childrenCount] = child;
    child->parent = parent;
    child->siblingIndex = parent->childrenCount++;
}

/**
 * =====================================================================================================================
 */

void UIremoveChild(UINode *parent, UINode *child) {
    if (!parent || !child) {
        printf("Parent or child is NULL in UIremoveChild\n");
        return;
    }

    if (child->parent != parent) {
        printf("The specified child does not belong to the specified parent in UIremoveChild\n");
        return;
    }

    // Shift the other children to maintain order
    for (size_t i = child->siblingIndex; i < parent->childrenCount - 1; i++) {
        parent->children[i] = parent->children[i + 1];
        parent->children[i]->siblingIndex = i;
    }
    parent->childrenCount--;
    child->parent = NULL;
    child->siblingIndex = 0;
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
        UIremoveChild(node->parent, node);
    }
    switch (node->type) {
        case UI_LABEL: {
            UILabel *label = (UILabel *)(node->widget);
            if (label->text) free(label->text);
            if (label->texture) SDL_DestroyTexture(label->texture);
            break;
        }
        case UI_BUTTON: {
            UIButton *btn = (UIButton *)(node->widget);
            if (btn->text) free(btn->text);
            if (btn->texture) SDL_DestroyTexture(btn->texture);
            break;
        }
        case UI_OPTION_CYCLE: {
            UIOptionCycle *optCycle = (UIOptionCycle *)(node->widget);
            CDLLNode *currOption = optCycle->currOption;
            if (currOption) freeList(&currOption);
            break;
        }
    }
    if (node->layout) free(node->layout);
    free(node->rect);
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

 void UIapplyLayout(UINode *node) {
    if (!node || !node->layout) return;

    UILayout *layout = node->layout;

    // Calculate padding in pixels
    int padTop = (int)(layout->padding.top * node->rect->h);
    int padBottom = (int)(layout->padding.bottom * node->rect->h);
    int padLeft = (int)(layout->padding.left * node->rect->w);
    int padRight = (int)(layout->padding.right * node->rect->w);

    // Cut the padding space
    int innerX = node->rect->x + padLeft;
    int innerY = node->rect->y + padTop;
    int innerW = node->rect->w - (padLeft + padRight);
    int innerH = node->rect->h - (padTop + padBottom);

    // Begin layouting on Y axis
    if (layout->type == UI_LAYOUT_VERTICAL) {
        int totalChildrenH = 0;
        for (size_t i = 0; i < node->childrenCount; i++) {
            totalChildrenH += node->children[i]->rect->h;
        }

        int spacingPx = (int)(layout->spacing * innerH);
        totalChildrenH += (int)((node->childrenCount - 1) * spacingPx);

        int startY = innerY;
        if (layout->alignment.v == UI_ALIGNMENT_CENTER) {
            startY = innerY + (innerH - totalChildrenH) / 2;
        } else if (layout->alignment.v == UI_ALIGNMENT_END) {
            startY = innerY + innerH - totalChildrenH;
        }

        int y = startY;
        for (size_t i = 0; i < node->childrenCount; i++) {
            UINode *child = node->children[i];
            child->rect->y = y;
            child->rect->x = innerX;

            // Horizontal alignment
            if (layout->alignment.h == UI_ALIGNMENT_START) {
                child->rect->x = innerX;
            } else if (layout->alignment.h == UI_ALIGNMENT_CENTER) {
                child->rect->x = innerX + (innerW - child->rect->w) / 2;
            } else if (layout->alignment.h == UI_ALIGNMENT_END) {
                child->rect->x = innerX + innerW - child->rect->w;
            }

            y += child->rect->h + spacingPx;

            // recurse into children
            UIapplyLayout(child);
        }
    }
    else if (layout->type == UI_LAYOUT_HORIZONTAL) {
        // Mirror logic along X axis
        int totalChildrenW = 0;
        for (size_t i = 0; i < node->childrenCount; i++) {
            totalChildrenW += node->children[i]->rect->w;
        }

        int spacingPx = (int)(layout->spacing * innerW);
        totalChildrenW += (int)((node->childrenCount - 1) * spacingPx);

        int startX = innerX;
        if (layout->alignment.h == UI_ALIGNMENT_CENTER) {
            startX = innerX + (innerW - totalChildrenW) / 2;
        } else if (layout->alignment.h == UI_ALIGNMENT_END) {
            startX = innerX + innerW - totalChildrenW;
        }

        int x = startX;
        for (size_t i = 0; i < node->childrenCount; i++) {
            UINode *child = node->children[i];
            child->rect->x = x;

            if (layout->alignment.v == UI_ALIGNMENT_START) {
                child->rect->y = innerY;
            } else if (layout->alignment.v == UI_ALIGNMENT_CENTER) {
                child->rect->y = innerY + (innerH - child->rect->h) / 2;
            } else if (layout->alignment.v == UI_ALIGNMENT_END) {
                child->rect->y = innerY + innerH - child->rect->h;
            }

            x += child->rect->w + spacingPx;

            UIapplyLayout(child);
        }
    }
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
                SDL_RenderCopy(rdr, label->texture, NULL, node->rect);
            }
            break;
        }
        case UI_BUTTON: {
            UIButton *button = (UIButton *)(node->widget);
            if (button->texture) {
                SDL_RenderCopy(rdr, button->texture, NULL, node->rect);
            }
            break;
        }
        case UI_OPTION_CYCLE: {
            UIOptionCycle *optionCycle = (UIOptionCycle *)(node->widget);
            if (optionCycle->currOption && optionCycle->currOption->data && optionCycle->selector) {
                UINode *selectorNode = (UINode *)optionCycle->selector;
                UIButton *selector = (UIButton *)(selectorNode->widget);
                if (selector->texture) SDL_RenderCopy(rdr, selector->texture, NULL, selectorNode->rect);

                UINode *currOptionNode = (UINode *)optionCycle->currOption->data;
                UIButton *currOptionBtn = (UIButton *)(currOptionNode->widget);
                if (currOptionBtn->texture) SDL_RenderCopy(rdr, currOptionBtn->texture, NULL, currOptionNode->rect);
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
        switch (uiManager->focusedNode->type) {
            case UI_BUTTON: {
                UIButton *button = (UIButton *)(uiManager->focusedNode->widget);
                button->currColor = button->colors[UI_STATE_NORMAL];
                UImarkNodeDirty(uiManager, uiManager->focusedNode);
                break;
            }
            case UI_OPTION_CYCLE: {
                UIOptionCycle *optCycle = (UIOptionCycle *)(uiManager->focusedNode->widget);
                if (optCycle->selector) {
                    UIButton *btn = (UIButton *)(optCycle->selector->widget);
                    btn->currColor = btn->colors[UI_STATE_NORMAL];
                    UImarkNodeDirty(uiManager, optCycle->selector);
                }
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
    switch (uiManager->focusedNode->type) {
        case UI_BUTTON: {
            UIButton *button = (UIButton *)(uiManager->focusedNode->widget);
            button->currColor = button->colors[UI_STATE_FOCUSED];
            UImarkNodeDirty(uiManager, uiManager->focusedNode);
            break;
        }
        case UI_OPTION_CYCLE: {
            UIOptionCycle *optCycle = (UIOptionCycle *)(uiManager->focusedNode->widget);
            if (optCycle->selector) {
                UIButton *btn = (UIButton *)(optCycle->selector->widget);
                btn->currColor = btn->colors[UI_STATE_FOCUSED];
                UImarkNodeDirty(uiManager, optCycle->selector);
            }
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
        case UI_OPTION_CYCLE:
            return 1;
        default:
            return 0;
    }
}

/**
 * =====================================================================================================================
 */

UILayout* UIcreateLayout(UILayoutType type, UIPadding padding, UIAlignment alignment, float spacing) {
    UILayout *layout = calloc(1, sizeof(UILayout));
    if (!layout) {
        printf("Failed to allocate memory for the UI layout\n");
        exit(EXIT_FAILURE);
    }
    layout->type = type;
    layout->alignment = alignment;
    layout->padding = padding;
    layout->spacing = spacing;
    return layout;
}

/**
 * =====================================================================================================================
 */

UINode* UIcreateContainer(SDL_Rect rect, UILayout *layout) {
    UINode *container = calloc(1, sizeof(UINode));
    if (!container) {
        printf("Failed to allocate memory for the root UI node\n");
        exit(EXIT_FAILURE);
    }
    container->type = UI_CONTAINER;
    container->isDirty = 1;  // Dirty by default
    container->isVisible = 1;
    container->layout = layout;
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
    container->rect = contRect;
    return container;
}

/**
 * =====================================================================================================================
 */

UINode* UIcreateLabel(SDL_Renderer *rdr, TTF_Font *font, char *text, SDL_Color color) {
    UINode *node = calloc(1, sizeof(UINode));
    if (!node) {
        printf("Failed to allocate memory for label node\n");
        exit(EXIT_FAILURE);
    }
    node->isDirty = 1;  // Dirty by default
    node->type = UI_LABEL;
    node->isVisible = 1;
    node->rect = calloc(1, sizeof(SDL_Rect));
    if (!node->rect) {
        printf("Failed to allocate memory for label rectangle\n");
        exit(EXIT_FAILURE);
    }

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
    *node->rect = (SDL_Rect) {
        .x = 0,
        .y = 0,
        .w = surface->w,
        .h = surface->h
    };

    SDL_FreeSurface(surface);
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
    UINode *node = calloc(1, sizeof(UINode));
    if (!node) {
        printf("Failed to allocate memory for button node\n");
        exit(EXIT_FAILURE);
    }
    node->isDirty = 1;  // Dirty by default
    node->type = UI_BUTTON;
    node->state = state;
    node->isVisible = 1;
    node->rect = calloc(1, sizeof(SDL_Rect));
    if (!node->rect) {
        printf("Failed to allocate memory for button rectangle\n");
        exit(EXIT_FAILURE);
    }

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

    node->widget = (void *)button;
    *node->rect = (SDL_Rect) {
        .x = 0,
        .y = 0,
        .w = titleSurface->w,
        .h = titleSurface->h
    };
    SDL_FreeSurface(titleSurface);  // we don't need the surface anymore
    return node;
}

/**
 * =====================================================================================================================
 */

UINode* UIcreateOptionCycle(SDL_Rect rect, UILayout *layout, UINode *selectorBtn, CDLLNode *currOption) {
    UINode *node = calloc(1, sizeof(UINode));
    if (!node) {
        printf("Failed to allocate memory for option cycle node\n");
        exit(EXIT_FAILURE);
    }
    node->isDirty = 1;  // Dirty by default
    node->type = UI_OPTION_CYCLE;
    node->isVisible = 1;
    SDL_Rect *nodeRect = calloc(1, sizeof(SDL_Rect));
    if (!nodeRect) {
        printf("Failed to allocate memory for option cycle rectangle\n");
        exit(EXIT_FAILURE);
    }
    *nodeRect = rect;
    node->rect = nodeRect;

    UIOptionCycle *optionCycle = calloc(1, sizeof(UIOptionCycle));
    if (!optionCycle) {
        printf("Failed to allocate memory for option cycle UI element\n");
        exit(EXIT_FAILURE);
    }

    optionCycle->selector = selectorBtn;
    optionCycle->currOption = currOption;

    node->widget = (void *)optionCycle;
    node->layout = layout;
    return node;
}