#include "stateManager.h"

void onEnterGarage(ZENg zEngine) {
    float leftPanelWidth = 0.35;  // 35% of the screen width
    float listPaddings = 0.1;  // 10% on every side
    float listSpacing = 0.1;

    float centerPanelWidth = 0.3;  // 30% of the screen width
    float centerPanelSpacing = 0.4;  // Make enough space between title and tank
    float centerPanelPaddings = 0.15;  // 15% on every side
    float titleHeight = 0.3;  // 30% of the center panel height

    float rightPanelWidth = 0.35;  // 35% of the screen width
    float rightPanelPaddingV = 0.1;  // 20% of the screen height
    float rightPanelSpacing = 0.15;  // 15% of the screen height

    // Create a new background with horizontal layout
    UINode *bg = UIcreateContainer(
        (SDL_Rect){.x = 0, .y = 0, .w = LOGICAL_WIDTH, .h = LOGICAL_HEIGHT},
        UIcreateLayout(
            UI_LAYOUT_HORIZONTAL, (UIPadding){0.0, 0.0, 0.0, 0.0},
            (UIAlignment){.h = UI_ALIGNMENT_ABSOLUTE, .v = UI_ALIGNMENT_ABSOLUTE}, 0.0
        )
    );
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, bg);

    UINode *leftPanelDiv = UIcreateContainer(
        (SDL_Rect) {
            .x = 0,
            .y = 0,
            .w = LOGICAL_WIDTH * leftPanelWidth,
            .h = LOGICAL_HEIGHT
        },
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){listPaddings, listPaddings, listPaddings, listPaddings},
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_CENTER}, listSpacing
        )
    );
    UIinsertNode(zEngine->uiManager, bg, leftPanelDiv);

    char* buttonLabels[] = {
        "Test button 1", "Very long test button 1, indeed", "Back"
    };
    void (*buttonActions[])(ZENg, void *) = {
        &garageToMMenu, &garageToMMenu, &garageToMMenu
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
        UIinsertNode(zEngine->uiManager, leftPanelDiv, button);
    }

    UINode *centerPanelDiv = UIcreateContainer(
        (SDL_Rect){
            .x = LOGICAL_WIDTH * leftPanelWidth, .y = 0, .w = LOGICAL_WIDTH * centerPanelWidth, .h = LOGICAL_HEIGHT
        },
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){
                centerPanelPaddings, centerPanelPaddings, centerPanelPaddings, centerPanelPaddings
            },
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_START}, centerPanelSpacing
        )
    );
    UIinsertNode(zEngine->uiManager, bg, centerPanelDiv);

    UINode *titleLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
        strdup("Garage"), COLOR_CRIMSON
    );
    UIinsertNode(zEngine->uiManager, centerPanelDiv, titleLabel);

    UINode *testLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
        strdup("Here would be the tank"), COLOR_PURPLE
    );
    UIinsertNode(zEngine->uiManager, centerPanelDiv, testLabel);

    UINode *rightPanelDiv = UIcreateContainer(
        (SDL_Rect){
            .x = LOGICAL_WIDTH * (leftPanelWidth + centerPanelWidth), .y = 0,
            .w = LOGICAL_WIDTH * rightPanelWidth, .h = LOGICAL_HEIGHT
        },
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){
                .bottom = rightPanelPaddingV, .left = 0.0, .right = 0.0, .top = rightPanelPaddingV
            },
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_CENTER}, rightPanelSpacing
        )
    );
    UIinsertNode(zEngine->uiManager, bg, rightPanelDiv);

    UINode *descTitleLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
        strdup("Description"), COLOR_CRIMSON
    );
    UIinsertNode(zEngine->uiManager, rightPanelDiv, descTitleLabel);

    UINode *descLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
        strdup("I have to make the text wrap"), COLOR_PURPLE
    );
    UIinsertNode(zEngine->uiManager, rightPanelDiv, descLabel);

    UIapplyLayout(zEngine->uiManager->root);
}

/**
 * =====================================================================================================================
 */

void onExitGarage(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

Uint8 handleGarageEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

/**
 * =====================================================================================================================
 */

void garageToMMenu(ZENg zEngine, void *data) {
    popState(zEngine);
}