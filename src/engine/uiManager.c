#include "uiManager.h"
#include "../states/stateManager.h"

UIManager initUIManager() {
    UIManager uiManager = calloc(1, sizeof(struct UIManager));
    if (!uiManager) {
        printf("Failed to allocate memory for the UI Manager\n");
        exit(EXIT_FAILURE);
    }
    return uiManager;
}

/**
 * =====================================================================================================================
 */

UINode* UIparseFromFile(ZENg zEngine, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open UI file: %s\n", filename);
        return NULL;
    }

    // Magic commences
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(size + 1);
    fread(data, 1, size, f);
    data[size] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    free(data);

    if (!root) return NULL;

    // Prepare the hashmap
    HashMap parserMap = MapInit(64, MAP_PARSER);
    loadParserEntries(parserMap, getCurrState(zEngine->stateMng)->type);

    // Parse recursively from the root
    UINode *uiRoot = UIparseNode(zEngine, parserMap, root);
    cJSON_Delete(root);
    MapFree(parserMap);

    return uiRoot;
}

/**
 * =====================================================================================================================
 */

UINode* UIparseNode(ZENg zEngine, HashMap parserMap, cJSON *json) {
    const char *type = cJSON_GetObjectItemCaseSensitive(json, "type")->valuestring;
    UINode *node = NULL;

    // Rectangle
    SDL_Rect rect = {0};
    cJSON *rectJson = cJSON_GetObjectItemCaseSensitive(json, "rect");
    if (rectJson) {
        rect.x = cJSON_GetObjectItem(rectJson, "x")->valueint;
        rect.y = cJSON_GetObjectItem(rectJson, "y")->valueint;
        rect.w = cJSON_GetObjectItem(rectJson, "w")->valueint;
        rect.h = cJSON_GetObjectItem(rectJson, "h")->valueint;
    }

    if (strcmp(type, "container") == 0) {
        // Layout
        cJSON *layoutJson = cJSON_GetObjectItem(json, "layout");
        UILayout *layout = NULL;
        if (layoutJson) {
            // defaults
            UIPadding pad = {0,0,0,0};
            UIAlignment align = { UI_ALIGNMENT_ABSOLUTE, UI_ALIGNMENT_ABSOLUTE };
            float spacing = 0.0;

            cJSON *padJson = cJSON_GetObjectItem(layoutJson, "padding");
            if (padJson) {
                pad.top = cJSON_GetObjectItem(padJson, "top")->valuedouble;
                pad.bottom = cJSON_GetObjectItem(padJson, "bottom")->valuedouble;
                pad.left = cJSON_GetObjectItem(padJson, "left")->valuedouble;
                pad.right = cJSON_GetObjectItem(padJson, "right")->valuedouble;
            }
            cJSON *alignJson = cJSON_GetObjectItem(layoutJson, "alignment");
            if (alignJson) {
                const char *h = cJSON_GetObjectItem(alignJson, "h")->valuestring;
                const char *v = cJSON_GetObjectItem(alignJson, "v")->valuestring;
                align.h = strcmp(h,"center") == 0 ? UI_ALIGNMENT_CENTER : strcmp(h,"end") == 0 ? UI_ALIGNMENT_END
                : UI_ALIGNMENT_START;
                align.v = strcmp(v,"center") == 0 ? UI_ALIGNMENT_CENTER : strcmp(v,"end") == 0 ? UI_ALIGNMENT_END
                : UI_ALIGNMENT_START;
            }
            cJSON *spacingJson = cJSON_GetObjectItem(layoutJson, "spacing");
            if (spacingJson) spacing = (float)spacingJson->valuedouble;

            const char *layoutType = cJSON_GetObjectItem(layoutJson, "type")->valuestring;
            int layoutTypeVal = strcmp(layoutType, "vertical") == 0 ? UI_LAYOUT_VERTICAL : UI_LAYOUT_HORIZONTAL;

            layout = UIcreateLayout(layoutTypeVal, pad, align, spacing);
        }
        node = UIcreateContainer(rect, layout);

        // Children
        cJSON *children = cJSON_GetObjectItemCaseSensitive(json, "children");
        cJSON *child;
        cJSON_ArrayForEach(child, children) {
            UINode *cNode = UIparseNode(zEngine, parserMap, child);
            if (cNode) UIinsertNode(zEngine->uiManager, node, cNode);
        }
    }
    else if (strcmp(type, "label") == 0) {
        const char *text = cJSON_GetObjectItem(json, "text")->valuestring;
        const char *font = cJSON_GetObjectItem(json, "font")->valuestring;
        const char *colorName = cJSON_GetObjectItem(json, "color")->valuestring;

        SDL_Color color = applyColorAlpha(parserMap, cJSON_GetObjectItem(json, "color"));

        node = UIcreateLabel(
            zEngine->display->renderer,
            getFont(zEngine->resources, font),
            strdup(text),
            color
        );
    }
    else if (strcmp(type, "button") == 0) {
        cJSON *textJson = cJSON_GetObjectItem(json, "text");
        const char *text = textJson ? textJson->valuestring : NULL;
        const char *font = cJSON_GetObjectItem(json, "font")->valuestring;
        cJSON *onClickJSON = cJSON_GetObjectItem(json, "onClick");
        const char *actionStr = NULL;
        if (onClickJSON && cJSON_IsString(onClickJSON)) {
            actionStr = onClickJSON->valuestring;
        }

        cJSON *colorJson = cJSON_GetObjectItem(json, "color");
        SDL_Color colors[UI_STATE_COUNT] = {0};
        if (colorJson) {
            for (Uint8 i = 0; i < UI_STATE_COUNT; i++) {
                const char *state = i == UI_STATE_NORMAL ? "normal" : i == UI_STATE_FOCUSED ? "focused" : "selected";

                cJSON *stateColorJson = cJSON_GetObjectItem(colorJson, state);
                colors[i] = applyColorAlpha(parserMap, stateColorJson);
            }
        }

        void (*action)(ZENg, void*);
        if (actionStr) action = resolveAction(parserMap, actionStr);

        node = UIcreateButton(
            zEngine->display->renderer,
            getFont(zEngine->resources, font),
            strdup(text),
            UI_STATE_NORMAL,
            colors,
            action,
            NULL
        );
    } else if (strcmp(type, "optionCycle") == 0) {
        // Layout
        cJSON *layoutJson = cJSON_GetObjectItem(json, "layout");
        UILayout *layout = NULL;
        // defaults
        UIPadding pad = {0,0,0,0};
        UIAlignment align = { UI_ALIGNMENT_ABSOLUTE, UI_ALIGNMENT_ABSOLUTE };
        float spacing = 0.0;

        cJSON *padJson = cJSON_GetObjectItem(layoutJson, "padding");
        if (padJson) {
            pad.top = cJSON_GetObjectItem(padJson, "top")->valuedouble;
            pad.bottom = cJSON_GetObjectItem(padJson, "bottom")->valuedouble;
            pad.left = cJSON_GetObjectItem(padJson, "left")->valuedouble;
            pad.right = cJSON_GetObjectItem(padJson, "right")->valuedouble;
        }
        cJSON *alignJson = cJSON_GetObjectItem(layoutJson, "alignment");
        if (alignJson) {
            const char *h = cJSON_GetObjectItem(alignJson, "h")->valuestring;
            const char *v = cJSON_GetObjectItem(alignJson, "v")->valuestring;
            align.h = strcmp(h,"center") == 0 ? UI_ALIGNMENT_CENTER : strcmp(h,"end") == 0 ? UI_ALIGNMENT_END
            : UI_ALIGNMENT_START;
            align.v = strcmp(v,"center") == 0 ? UI_ALIGNMENT_CENTER : strcmp(v,"end") == 0 ? UI_ALIGNMENT_END
            : UI_ALIGNMENT_START;
        }
        cJSON *spacingJson = cJSON_GetObjectItem(layoutJson, "spacing");
        if (spacingJson) spacing = (float)spacingJson->valuedouble;

        cJSON *layoutTypeJson = cJSON_GetObjectItem(layoutJson, "type");
        const char *layoutType = layoutTypeJson ? layoutTypeJson->valuestring : NULL;
        int layoutTypeVal = strcmp(layoutType, "vertical") == 0 ? UI_LAYOUT_VERTICAL : UI_LAYOUT_HORIZONTAL;

        layout = UIcreateLayout(layoutTypeVal, pad, align, spacing);


        // Selector
        cJSON *selectorJson = cJSON_GetObjectItem(json, "selector");
        UINode *selector = NULL;
        if (!selectorJson) {
            printf("Option cycle node missing selector definition\n");
            return NULL;
        }
        selector = UIparseNode(zEngine, parserMap, selectorJson);
        if (!selector) {
            printf("Failed to parse option cycle selector node\n");
            return NULL;
        }

        // Option list
        CDLLNode *options = NULL;
        cJSON *optionsJson = cJSON_GetObjectItem(json, "options");
        if (!optionsJson) {
            printf("Option cycle node missing options definition\n");
            return NULL;
        }
        char *listTypeStr = cJSON_GetObjectItem(optionsJson, "type")->valuestring;
        if (strcmp(listTypeStr, "buttonList") == 0) {
            cJSON *dataJson = cJSON_GetObjectItem(optionsJson, "data");
            if (!dataJson) {
                printf("Option cycle options missing data definition\n");
                return NULL;
            }

            cJSON *fontJson = cJSON_GetObjectItem(optionsJson, "font");
            if (!fontJson) {
                printf("Option cycle options missing font definition\n");
                return NULL;
            }
            TTF_Font *font = getFont(zEngine->resources, fontJson->valuestring);

            cJSON *colorJson = cJSON_GetObjectItem(optionsJson, "color");
            SDL_Color colors[UI_STATE_COUNT] = {0};
            if (colorJson) {
                for (Uint8 j = 0; j < UI_STATE_COUNT; j++) {
                    const char *state = j == UI_STATE_NORMAL ? "normal" : j == UI_STATE_FOCUSED ?
                    "focused" : "selected";
                    cJSON *colorStateJson = cJSON_GetObjectItem(colorJson, state);
                    colors[j] = applyColorAlpha(parserMap, colorStateJson);
                }
            }

            cJSON *providerJson = cJSON_GetObjectItem(dataJson, "provider");
            if (!providerJson) {
                printf("Option cycle options missing data provider definition\n");
                return NULL;
            }
            char *providerStr = providerJson->valuestring;
            void (*providerFunc)(ZENg, HashMap) = resolveProvider(parserMap, providerStr);
            if (providerFunc) providerFunc(zEngine, parserMap);
            // if (strcmp(providerStr, "getWindowModes") == 0) {
            //     // Uint8 *windowModesCount = resolveBool(parserMap, "windowModesCount");
            //     for (Uint8 i = 0; i < *windowModesCount; i++) {
            //         char key[32];
            //         snprintf(key, sizeof(key), "windowModes[%d]", i);
            //         // Uint8 *mode = resolveBool(parserMap, key);

            //         char *btnText = calloc(32, sizeof(char));
            //         snprintf(btnText, 32, "%s", *mode == 0 ? "Windowed" : "Fullscreen");

            //         UINode *btn = UIcreateButton(
            //             zEngine->display->renderer,
            //             font,
            //             btnText,
            //             UI_STATE_NORMAL,
            //             colors,
            //             NULL,
            //             (void *)mode
            //         );
            //         if (!options) {
            //             options = initList((void *)btn);
            //         } else {
            //             CDLLInsertLast(options, (void *)btn);
            //         }
            //     }
            // } else if (strcmp(providerStr, "getResolutions") == 0) {
            //     Uint8 *resCount = resolveBool(parserMap, "resolutionsCount");
            //     SDL_DisplayMode *modes = resolveDisplayMode(parserMap, "resolutions");
            //     for (Uint8 i = 0; i < *resCount; i++) {
            //         SDL_DisplayMode *mode = &modes[i];

            //         char *btnText = calloc(16, sizeof(char));
            //         snprintf(btnText, 16, "%dx%d", mode->w, mode->h);

            //         UINode *btn = UIcreateButton(
            //             zEngine->display->renderer,
            //             font,
            //             btnText,
            //             UI_STATE_NORMAL,
            //             colors,
            //             NULL,
            //             (void *)mode
            //         );
            //         if (!options) {
            //             options = initList((void *)btn);
            //         } else {
            //             CDLLInsertLast(options, (void *)btn);
            //         }
            //     }
            // }
        } else if (strcmp(listTypeStr, "imageList") == 0) {
            cJSON *dataJson = cJSON_GetObjectItem(optionsJson, "data");
            if (!dataJson) {
                printf("Option cycle options missing data definition\n");
                return NULL;
            }
            cJSON *providerJson = cJSON_GetObjectItem(dataJson, "provider");
            if (!providerJson) {
                printf("Option cycle options missing data provider definition\n");
                return NULL;
            }
            char *providerStr = providerJson->valuestring;
            void (*providerFunc)(ZENg, HashMap) = resolveProvider(parserMap, providerStr);
            if (providerFunc) providerFunc(zEngine, parserMap);
            // if (strcmp(providerStr, "getMainGuns") == 0) {
            //     // getMainGuns must provide an array of guns entities, one of SDL_Rects for weapon icons and their count
            //     // The texture is fetched from each gun entity's render component
            //     // Data is a pointer to the current entity
            //     // Uint8 *mainGunsCount = resolveBool(parserMap, "mainGunsCount");
            //     // Entity *mainGuns = resolveEntityArray(parserMap, "mainGuns");
            //     // SDL_Rect *mainGunsRects = resolveRectArray(parserMap, "mainGunsRects");
            //     for (Uint8 i = 0; i < *mainGunsCount; i++) {
            //         char key[32] = {0};
            //         snprintf(key, sizeof(key), "mainGuns[%d]", i);

            //         UINode *img = UIcreateImage(
            //             mainGunsRects[i],
            //             getTexture(zEngine->resources, key),
            //             &mainGuns[i]
            //         );
            //         if (!options) {
            //             options = initList((void *)img);
            //         } else {
            //             CDLLInsertLast(options, (void *)img);
            //         }
            //     }
            // } else if (strcmp(providerStr, "getSecondaryGuns") == 0) {

            // } else if (strcmp(providerStr, "getHulls") == 0) {

            // } else if (strcmp(providerStr, "getModules") == 0) {

            // }

        }

        // Nav arrows
        cJSON *navArrowsJson = cJSON_GetObjectItem(json, "Arrows");
        if (!navArrowsJson) {
            printf("Option cycle node missing arrows definition\n");
            return NULL;
        }
        char *arrowPath = navArrowsJson->valuestring;

        node = UIcreateOptionCycle(rect, layout, selector, options, getTexture(zEngine->resources, arrowPath));

        // Add those as children to apply layout
        UIinsertNode(zEngine->uiManager, node, selector);
        UIinsertNode(zEngine->uiManager, node, (UINode *)options->data);
    }

    if (node) {
        // Focused flag
        cJSON *focusedFlag = cJSON_GetObjectItem(json, "focused");
        if (cJSON_IsTrue(focusedFlag)) {
            zEngine->uiManager->focusedNode = node;
            node->state = UI_STATE_FOCUSED;
            UIrefocus(zEngine->uiManager, node);
        }
    }

    return node;
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
        // If the current node has a parent, remove it from the parent's children
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
            // Data should be owned by someone other than the button
            break;
        }
        case UI_OPTION_CYCLE: {
            /**
             * When deleting an option cycle keep in mind that
             * the children (last option and the selector) are already deleted
             */
            UIOptionCycle *optCycle = (UIOptionCycle *)(node->widget);

            CDLLNode *currOption = optCycle->currOption;
            if (!currOption) {
                break;
            }
            CDLLNode *head = currOption->next;
            currOption->prev->next = currOption->next;  // Remove currOption from the list
            free(currOption);  // The current option has the data already deleted (was a child)

            CDLLNode *curr = head->next;
            while (curr != head) {
                CDLLNode *next = curr->next;
                UIdeleteNode(uiManager, (UINode *)curr->data);  // Delete the option node
                free(curr);
                curr = next;
            }

            UIdeleteNode(uiManager, (UINode *)head->data);  // Delete the head option node
            free(head);
            optCycle->currOption = NULL;
            break;
        }
    }
    if (node->layout) free(node->layout);
    if (node->rect) free(node->rect);
    if (node->widget) free(node->widget);
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

    // Recursively delete all nodes
    if (uiManager->root) {
        UIdeleteNode(uiManager, uiManager->root);
        uiManager->root = NULL;
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

    #ifdef DEBUGPP
        // Guard against the empty UI tree
        if (node->rect) printf(
            "Rendering UI node of type %d (x=%d, y=%d, w=%d, h=%d)\n",
            node->type, node->rect->x, node->rect->y, node->rect->w, node->rect->h
        );
    #endif

    // Render this node based on its type
    if (node->isVisible == 0 || (!node->rect)) return;
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
        case UI_IMAGE: {
            UIImage *image = (UIImage *)(node->widget);
            if (image->texture) {
                SDL_RenderCopy(rdr, image->texture, NULL, node->rect);
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

                if (optionCycle->arrowTexture) {
                    // Render two arrows on the sides of the current option
                    int arrowSize = (currOptionNode->rect->h);  // The arrows are squares
                    SDL_Rect leftArrowRect = {
                        .x = currOptionNode->rect->x - arrowSize - 5,  // 5px gap
                        .y = currOptionNode->rect->y,
                        .w = arrowSize,
                        .h = arrowSize
                    };
                    SDL_Rect rightArrowRect = {
                        .x = currOptionNode->rect->x + currOptionNode->rect->w + 5,  // 5px gap
                        .y = currOptionNode->rect->y,
                        .w = arrowSize,
                        .h = arrowSize
                    };
                    SDL_RenderCopyEx(rdr, optionCycle->arrowTexture, NULL, &leftArrowRect, 0.0, NULL, SDL_FLIP_NONE);
                    // The arrow textures are left-pointing, so flip the right one
                    SDL_RenderCopyEx(
                        rdr, optionCycle->arrowTexture, NULL, &rightArrowRect, 0.0, NULL, SDL_FLIP_HORIZONTAL
                    );
                }
            }
            break;
        }
    }

    // Render children recursively
    for (size_t i = 0; i < node->childrenCount; i++) {
        UIrenderNode(rdr, node->children[i]);
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
    if (!button->text) {
        button->text = strdup("");  // Default to empty string if NULL
    }
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

UINode *UIcreateImage(SDL_Rect rect, SDL_Texture *texture, void *data) {
    UINode *node = calloc(1, sizeof(UINode));
    if (!node) {
        printf("Failed to allocate memory for image node\n");
        exit(EXIT_FAILURE);
    }
    node->isDirty = 1;  // Dirty by default
    node->type = UI_IMAGE;
    node->isVisible = 1;
    SDL_Rect *nodeRect = calloc(1, sizeof(SDL_Rect));
    if (!nodeRect) {
        printf("Failed to allocate memory for image rectangle\n");
        exit(EXIT_FAILURE);
    }
    *nodeRect = rect;
    node->rect = nodeRect;

    UIImage *image = calloc(1, sizeof(UIImage));
    if (!image) {
        printf("Failed to allocate memory for image UI element\n");
        exit(EXIT_FAILURE);
    }

    image->texture = texture;
    image->data = data;

    node->widget = (void *)image;
    return node;
}

/**
 * =====================================================================================================================
 */

UINode* UIcreateOptionCycle(
    SDL_Rect rect, UILayout *layout, UINode *selectorBtn, CDLLNode *currOption, SDL_Texture *arrowTexture
) {
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
    optionCycle->arrowTexture = arrowTexture;

    node->widget = (void *)optionCycle;
    node->layout = layout;
    return node;
}

/**
 * =====================================================================================================================
 */

void* resolveAction(HashMap parserMap, const char *key) {
    if (!parserMap || !key) THROW_ERROR_AND_RETURN("Parser map or key is NULL", NULL);
    if (parserMap->type != MAP_PARSER) THROW_ERROR_AND_RETURN("Parser map is of wrong type", NULL);
    MapEntry *entry = MapGetEntry(parserMap, key);
    if (entry && entry->type == ENTRY_BTN_FUNC) return entry->data.ptr;
    THROW_ERROR_AND_DO(
        "Action with key", fprintf(stderr, "'%s' not found in parser map\n", key); return NULL;
    );
}

/**
 * =====================================================================================================================
 */

void* resolveProvider(HashMap parserMap, const char *key) {
    if (!parserMap || !key) THROW_ERROR_AND_RETURN("Parser map or key is NULL", NULL);
    if (parserMap->type != MAP_PARSER) THROW_ERROR_AND_RETURN("Parser map is of wrong type", NULL);
    MapEntry *entry = MapGetEntry(parserMap, key);
    if (entry && entry->type == ENTRY_PROVIDER_FUNC) return entry->data.ptr;
    THROW_ERROR_AND_DO(
        "Provider with key", fprintf(stderr, "'%s' not found in parser map\n", key); return NULL;
    );
}

/**
 * =====================================================================================================================
 */

SDL_Color resolveColor(HashMap parserMap, const char *key) {
    if (!parserMap || !key) THROW_ERROR_AND_RETURN("Parser map or key is NULL", COLOR_WHITE);
    if (parserMap->type != MAP_PARSER) THROW_ERROR_AND_RETURN("Parser map is of wrong type", COLOR_WHITE);
    MapEntry *entry = MapGetEntry(parserMap, key);
    if (entry && entry->type == ENTRY_COLOR) return *(SDL_Color *)entry->data.ptr;
    THROW_ERROR_AND_DO(
        "Color with key", fprintf(stderr, "'%s' not found in parser map\n", key); return COLOR_WHITE;
    );
}

/**
 * =====================================================================================================================
 */

SDL_Color applyColorAlpha(HashMap parserMap, cJSON *colorJson) {
    if (colorJson) {
        // Colors with transparency are of the form: <color>_<alpha(see global.h)>
        char colorCopy[strlen(colorJson->valuestring) + 1];
        strncpy(colorCopy, colorJson->valuestring, sizeof(colorCopy));

        char *colorBase = strtok(colorCopy, "_");
        SDL_Color clr = resolveColor(parserMap, colorBase);

        char *alphaStr = strtok(NULL, "_");
        if (alphaStr) {
            int alpha = atoi(alphaStr);
            clr = COLOR_WITH_ALPHA(clr, alpha);
        }
        return clr;
    }
    return COLOR_GRAY; // Default color
}

/**
 * =====================================================================================================================
 */

void loadParserEntries(HashMap parserMap, GameStateType state) {
    // Some colors
    MapAddEntry(parserMap, "white", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_WHITE]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "black", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_BLACK]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "gray", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_GRAY]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "red", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_RED]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "green", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_GREEN]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "blue", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_BLUE]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "yellow", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_YELLOW]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "cyan", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_CYAN]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "magenta", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_MAGENTA]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "crimson", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_CRIMSON]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "purple", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_PURPLE]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "pink", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_PINK]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "crimsondark", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_CRIMSON_DARK]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "gold", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_GOLD]}, ENTRY_COLOR);
    MapAddEntry(parserMap, "brown", (MapEntryVal){.ptr = (void *)&COLOR_TABLE[IDX_BROWN]}, ENTRY_COLOR);

    switch (state) {
        case STATE_MAIN_MENU: {
            MapAddEntry(parserMap, "prepareExit", (MapEntryVal){.ptr = &prepareExit}, ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "mMenuToPlay", (MapEntryVal){.ptr = &mMenuToPlay}, ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "mMenuToGarage", (MapEntryVal){.ptr = &mMenuToGarage}, ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "mMenuToSettings", (MapEntryVal){.ptr = &mMenuToSettings}, ENTRY_BTN_FUNC);
            break;
        }
        case STATE_PLAYING: {
            // Nothing for now
            break;
        }
        case STATE_PAUSED: {
            MapAddEntry(parserMap, "pauseToMMenu", (MapEntryVal){.ptr = &pauseToMMenu}, ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "pauseToPlay", (MapEntryVal){.ptr = &pauseToPlay}, ENTRY_BTN_FUNC);
            break;
        }
        case STATE_GARAGE: {
            MapAddEntry(parserMap, "getMainGuns", (MapEntryVal){.ptr = &getMainGuns}, ENTRY_PROVIDER_FUNC);
            MapAddEntry(parserMap, "garageToMMenu", (MapEntryVal){.ptr = &garageToMMenu}, ENTRY_BTN_FUNC);
            break;
        }
        case STATE_SETTINGS: {
            MapAddEntry(parserMap, "settingsToGameSettings", (MapEntryVal){.ptr = &settingsToGameSettings},
            ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "settingsToAudioSettings", (MapEntryVal){.ptr = &settingsToAudioSettings},
            ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "settingsToVideoSettings", (MapEntryVal){.ptr = &settingsToVideoSettings},
            ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "settingsToControlsSettings", (MapEntryVal){.ptr = &settingsToControlsSettings},
            ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "settingsToMMenu", (MapEntryVal){.ptr = &settingsToMMenu}, ENTRY_BTN_FUNC);
            break;
        }
        case STATE_GAME_SETTINGS: {
            MapAddEntry(parserMap, "gameSettingsToSettings", (MapEntryVal){.ptr = &gameSettingsToSettings},
            ENTRY_BTN_FUNC);
            break;
        }
        case STATE_AUDIO_SETTINGS: {
            MapAddEntry(parserMap, "audioSettingsToSettings", (MapEntryVal){.ptr = &audioSettingsToSettings},
            ENTRY_BTN_FUNC);
            break;
        }
        case STATE_VIDEO_SETTINGS: {
            MapAddEntry(parserMap, "getResolutions", (MapEntryVal){.ptr = &getResolutions}, ENTRY_PROVIDER_FUNC);
            MapAddEntry(parserMap, "getWindowModes", (MapEntryVal){.ptr = &getWindowModes}, ENTRY_PROVIDER_FUNC);
            MapAddEntry(parserMap, "changeWindowMode", (MapEntryVal){.ptr = &changeWindowMode}, ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "changeRes", (MapEntryVal){.ptr = &changeRes}, ENTRY_BTN_FUNC);
            MapAddEntry(parserMap, "videoSettingsToSettings", (MapEntryVal){.ptr = &videoSettingsToSettings},
            ENTRY_BTN_FUNC);
            break;
        }
        case STATE_CONTROLS_SETTINGS: {
            MapAddEntry(parserMap, "controlsSettingsToSettings", (MapEntryVal){.ptr = &controlsSettingsToSettings},
            ENTRY_BTN_FUNC);
            break;
        }
    }
}
