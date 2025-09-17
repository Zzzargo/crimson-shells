#include "stateManager.h"

void onEnterPauseState(ZENg zEngine) {
    // How much each of the height containers take
    float titleSize = 0.3;
    
    float listSize = 0.7;
    float listPaddingTop = 0.2;  // 20% of the list height
    float listSpacing = 0.08;  // 8% of the list height

    UINode *titleDiv = UIcreateContainer(
        (SDL_Rect){.x = 0, .y = 0, .w = LOGICAL_WIDTH, .h = (int)(LOGICAL_HEIGHT * titleSize)},
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){0.0, 0.0, 0.0, 0.0},
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_END}, 0.0
        )
    );
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, titleDiv);

    UINode *titleLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
        strdup("PAUSED"), COLOR_CRIMSON
    );
    UIinsertNode(zEngine->uiManager, titleDiv, titleLabel);


    UINode *listDiv = UIcreateContainer(
        (SDL_Rect){
            .x = 0,
            .y = (int)(LOGICAL_HEIGHT * titleSize),
            .w = LOGICAL_WIDTH,
            .h = (int)(LOGICAL_HEIGHT * listSize)
        },
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){.top = listPaddingTop, .bottom = 0.0, .left = 0.0, .right = 0.0},
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_START}, listSpacing
        )
    );
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, listDiv);

    char* buttonLabels[] = {
        "Resume", "Main menu"
    };
    void (*buttonActions[])(ZENg, void *) = {
        &pauseToPlay, &pauseToMMenu
    };
    size_t buttonCount = sizeof(buttonLabels) / sizeof(buttonLabels[0]);

    for (Uint8 i = 0; i < buttonCount; i++) {
        UINode *button = UIcreateButton(
            zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
            strdup(buttonLabels[i]), i == 0 ? UI_STATE_FOCUSED : UI_STATE_NORMAL,
            (SDL_Color[]){ COLOR_WHITE, COLOR_YELLOW, COLOR_WITH_ALPHA(COLOR_WHITE, OPACITY_MEDIUM) },
            buttonActions[i], NULL
        );

        if (i == 0) {
            // First button gets the focus
            zEngine->uiManager->focusedNode = button;
        }
        UIinsertNode(zEngine->uiManager, listDiv, button);
    }

    UIapplyLayout(zEngine->uiManager->root);

    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    systems[SYS_LIFETIME]->isActive = 0;
    systems[SYS_WEAPONS]->isActive = 0;
    systems[SYS_VELOCITY]->isActive = 0;
    systems[SYS_WORLD_COLLISIONS]->isActive = 0;
    systems[SYS_ENTITY_COLLISIONS]->isActive = 0;
    systems[SYS_POSITION]->isActive = 0;
    systems[SYS_HEALTH]->isActive = 0;
    systems[SYS_TRANSFORM]->isActive = 0;
}

/**
 * =====================================================================================================================
 */

void onExitPauseState(ZENg zEngine) {
    // Delete pause state entities
    sweepState(zEngine->ecs, STATE_PAUSED);
    UIclear(zEngine->uiManager);

    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    // Enable the game's systems
    systems[SYS_LIFETIME]->isActive = 1;
    systems[SYS_WEAPONS]->isActive = 1;
    systems[SYS_VELOCITY]->isActive = 1;
    systems[SYS_WORLD_COLLISIONS]->isActive = 1;
    systems[SYS_ENTITY_COLLISIONS]->isActive = 1;
    systems[SYS_POSITION]->isActive = 1;
    systems[SYS_HEALTH]->isActive = 1;
    systems[SYS_TRANSFORM]->isActive = 1;
    // Force a frame
    systems[SYS_VELOCITY]->isDirty = 1;
}

/**
 * =====================================================================================================================
 */

Uint8 handlePauseStateEvents(SDL_Event *e, ZENg zEngine) {
    return handleMenuNavigation(e, zEngine);
}

/**
 * =====================================================================================================================
 */

void pauseToPlay(ZENg zEngine, void *data) {
    popState(zEngine);
}

/**
 * =====================================================================================================================
 */

void pauseToMMenu(ZENg zEngine, void *data) {
    popState(zEngine);  // -> play
    popState(zEngine);  // -> Menu
}