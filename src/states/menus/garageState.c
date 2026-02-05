#include "states/stateManager.h"

void onEnterGarage(ZENg zEngine) {
    // Here the engine should fetch the player data from a savegame but it's not implemented yet so hardcode

    Entity playa = instantiateTank(
        zEngine, getTankPrefab(zEngine->prefabs, "player"), (Vec2){.x = 0, .y =0}
    );

    ECS ecs = zEngine->ecs;

    Uint64 plPage = playa / PAGE_SIZE;
    Uint64 plOffset = playa % PAGE_SIZE;
    
    // Make the player bigger in the garage
    Uint64 plRendDenseIdx = ecs->components[RENDER_COMPONENT].sparse[plPage][plOffset];
    RenderComponent *plRendComp = (RenderComponent *)ecs->components[RENDER_COMPONENT].dense[plRendDenseIdx];
    plRendComp->destRect->w = TILE_SIZE * 5;
    plRendComp->destRect->h = TILE_SIZE * 5;
    plRendComp->destRect->x = (LOGICAL_WIDTH - plRendComp->destRect->w) / 2;
    plRendComp->destRect->y = (LOGICAL_HEIGHT - plRendComp->destRect->h) / 2;

    Entity mainGunID = createEntity(ecs, STATE_GARAGE);
    WeaponPrefab *mainGunPrefab = getWeaponPrefab(zEngine->prefabs, "Bigfella");
    WeaponComponent *mainG = instantiateWeapon(zEngine, mainGunPrefab, PLAYER_ID);
    addComponent(ecs, mainGunID, WEAPON_COMPONENT, (void *)mainG);
    RenderComponent *mainGRender = createRenderComponent(
        getTexture(zEngine->resources, mainGunPrefab->iconPath), 10, 10, 128, 64, 0
    );
    addComponent(ecs, mainGunID, RENDER_COMPONENT, (void *)mainGRender);

    Entity secGun1ID = createEntity(ecs, STATE_GARAGE);
    WeaponPrefab *secGun1Prefab = getWeaponPrefab(zEngine->prefabs, "PKT");
    WeaponComponent *secGun1 = instantiateWeapon(zEngine, secGun1Prefab, PLAYER_ID);
    addComponent(ecs, secGun1ID, WEAPON_COMPONENT, (void *)secGun1);
    RenderComponent *secGun1Render = createRenderComponent(
        getTexture(zEngine->resources, secGun1Prefab->iconPath), 10, 10, 128, 64, 0
    );
    addComponent(ecs, secGun1ID, RENDER_COMPONENT, (void *)secGun1Render);
    // The list contains pointers to the weapon entities
    CDLLNode *weapList = initList((GenericData){.u64 = secGun1ID}, DATA_U64);

    Entity secGun2ID = createEntity(ecs, STATE_GARAGE);
    WeaponPrefab *secGun2Prefab = getWeaponPrefab(zEngine->prefabs, "M240C");
    WeaponComponent *secGun2 = instantiateWeapon(zEngine, secGun2Prefab, PLAYER_ID);
    addComponent(ecs, secGun2ID, WEAPON_COMPONENT, (void *)secGun2);
    RenderComponent *secGun2Render = createRenderComponent(
        getTexture(zEngine->resources, secGun2Prefab->iconPath), 10, 10, 128, 64, 0
    );
    addComponent(ecs, secGun2ID, RENDER_COMPONENT, (void *)secGun2Render);
    CDLLInsertLast(weapList, (GenericData){.u64 = secGun2ID}, DATA_U64);

    Entity hullID = createEntity(ecs, STATE_GARAGE);
    Entity moduleID = createEntity(ecs, STATE_GARAGE);
    LoadoutComponent *loadout = createLoadoutComponent(mainGunID, weapList, hullID, moduleID);
    addComponent(ecs, PLAYER_ID, LOADOUT_COMPONENT, (void *)loadout);

    getCurrState(zEngine->stateMng)->stateData = MapInit(31, MAP_STATE_DATA);
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIgarageState.json");
    UIapplyLayout(zEngine->uiManager->root);
}

/**
 * =====================================================================================================================
 */

void onExitGarage(ZENg zEngine) {
    UIclear(zEngine->uiManager);
    sweepState(zEngine->ecs, STATE_GARAGE);
}

/**
 * =====================================================================================================================
 */

ProviderResult* getMainGuns(ZENg zEngine) {
    // This should fetch the player's available main guns but it's not implemented yet
    // So this function is going to get the player's loadout guns

    if (!zEngine) THROW_ERROR_AND_RETURN("Engine is NULL in getMainGuns", NULL);
    Entity *guns = calloc(3, sizeof(Entity));
    if (!guns) THROW_ERROR_AND_EXIT("Failed to allocate memory for main guns array in getMainGuns");

    Uint64 page = PLAYER_ID / PAGE_SIZE;
    Uint64 offset = PLAYER_ID % PAGE_SIZE;
    Uint64 loadoutDenseIdx = zEngine->ecs->components[LOADOUT_COMPONENT].sparse[page][offset];
    LoadoutComponent *loadout = (LoadoutComponent *)zEngine->ecs->components[LOADOUT_COMPONENT].dense[loadoutDenseIdx];

    Entity secGun1ID = (loadout->currSecondaryGun->data.u64);
    Entity secGun2ID = (loadout->currSecondaryGun->next->data.u64);

    guns[0] = loadout->primaryGun;
    guns[1] = secGun1ID;
    guns[2] = secGun2ID;

    ProviderResult *result = calloc(1, sizeof(ProviderResult));
    if (!result) THROW_ERROR_AND_EXIT("Failed to allocate memory for ProviderResult in getMainGuns");
    result->data = (void *)guns;
    result->size = 3;
    result->type = RESULT_WEAPONS_ARRAY;

    HashMap stateData = getCurrState(zEngine->stateMng)->stateData;
    MapAddEntry(stateData, "mainGuns", (MapEntryVal){.ptr = result}, ENTRY_PROVIDER_RESULT);

    #ifdef DEBUG
        printf("Added 3 main guns to parser map\n");
    #endif
    return result;
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
