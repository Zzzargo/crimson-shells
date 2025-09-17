#include "stateManager.h"

void onEnterControlsSettings(ZENg zEngine) {
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
        strdup("Controls Settings"), COLOR_CRIMSON
    );
    UIinsertNode(zEngine->uiManager, titleDiv, titleLabel);


    UINode *listDiv = UIcreateContainer(
        (SDL_Rect) {
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
        "Back"
    };
    void (*buttonActions[])(ZENg, void *) = {
        &controlsSettingsToSettings
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
}

/**
 * =====================================================================================================================
 */

void onExitControlsSettings(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

Uint8 handleControlsSettingsEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

/**
 * =====================================================================================================================
 */

void controlsSettingsToSettings(ZENg zEngine, void *data) {
    popState(zEngine);
}