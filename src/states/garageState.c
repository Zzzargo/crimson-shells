#include "stateManager.h"

void onEnterGarage(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIgarageState.json");
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

void getMainGuns(ZENg zEngine, HashMap map) {
    if (!zEngine) {
        fprintf(stderr, "Cannot get main guns from a NULL ZENg\n");
        return;
    }

    Entity *guns = calloc(3, sizeof(Entity));
    if (!guns) {
        fprintf(stderr, "Failed to allocate memory for the main guns array\n");
        return;
    }

    Entity playa = instantiateTank(
        zEngine, getTankPrefab(zEngine->prefabs, "player"), (Vec2){.x = LOGICAL_WIDTH / 2, .y = LOGICAL_HEIGHT / 2}
    );

    // For now, hardcode the player and his loadout

    Entity mainGunID = createEntity(zEngine->ecs, STATE_PLAYING);
    WeaponPrefab *mainGunPrefab = getWeaponPrefab(zEngine->prefabs, "Bigfella");
    WeaponComponent *mainG = instantiateWeapon(zEngine, mainGunPrefab, PLAYER_ID);
    addComponent(zEngine->ecs, mainGunID, WEAPON_COMPONENT, (void *)mainG);

    Entity secGun1ID = createEntity(zEngine->ecs, STATE_PLAYING);
    WeaponPrefab *secGun1Prefab = getWeaponPrefab(zEngine->prefabs, "PKT");
    WeaponComponent *secGun1 = instantiateWeapon(zEngine, secGun1Prefab, PLAYER_ID);
    addComponent(zEngine->ecs, secGun1ID, WEAPON_COMPONENT, (void *)secGun1);
    CDLLNode *weapList = initList((void *)secGun1);

    Entity secGun2ID = createEntity(zEngine->ecs, STATE_PLAYING);
    WeaponPrefab *secGun2Prefab = getWeaponPrefab(zEngine->prefabs, "M240C");
    WeaponComponent *secGun2 = instantiateWeapon(zEngine, secGun2Prefab, PLAYER_ID);
    addComponent(zEngine->ecs, secGun2ID, WEAPON_COMPONENT, (void *)secGun2);
    CDLLInsertLast(weapList, (void *)secGun2);

    Entity hullID = createEntity(zEngine->ecs, STATE_PLAYING);
    Entity moduleID = createEntity(zEngine->ecs, STATE_PLAYING);
    LoadoutComponent *loadout = createLoadoutComponent(mainGunID, weapList, hullID, moduleID);
    addComponent(zEngine->ecs, PLAYER_ID, LOADOUT_COMPONENT, (void *)loadout);

    guns[0] = mainGunID;  // Basic Cannon
    guns[1] = secGun1ID;  // Spread Shot
    guns[2] = secGun2ID;  // Laser Beam

    Uint8 *mainGunsCount = calloc(1, sizeof(Uint8));
    *mainGunsCount = 3;

    // MapAddEntry(map, "mainGuns", (MapEntryVal){ .entityArray = guns }, MAP_ENTRY_ENTITYARRAY);
    // MapAddEntry(map, "mainGunsCount", (MapEntryVal){ .boolean = mainGunsCount }, MAP_ENTRY_BOOL);

    // addStateData(getCurrState(zEngine->stateMng), (void *)guns, STATE_DATA_PLAIN);

    // SDL_Rect *rects = calloc(3, sizeof(SDL_Rect));
    // for (int i = 0; i < 3; i++) {
    //     rects[i] = (SDL_Rect){.x = 0, .y = 0, .w = 120, .h = 68};
    // }
    // MapAddEntry(map, "mainGunsRects", (MapEntryVal){ .rectArray = rects }, MAP_ENTRY_RECTARRAY);
    // addStateData(getCurrState(zEngine->stateMng), (void *)rects, STATE_DATA_PLAIN);

    #ifdef DEBUG
        printf("Added 3 main guns to parser map\n");
    #endif
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