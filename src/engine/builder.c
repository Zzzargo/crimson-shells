#include "builder.h"
#include "engine/core/engine.h"  // This is some dark magic here

WeaponPrefab* getWeaponPrefab(HashMap prefabMng, const char *key) {
    if (!prefabMng || !key) THROW_ERROR_AND_RETURN("Resource manager or key is NULL", NULL);
    if (prefabMng->type != MAP_PREFABS) THROW_ERROR_AND_RETURN("Prefabs manager is of wrong type", NULL);
    MapEntry *entry = MapGetEntry(prefabMng, key);
    if (entry && entry->type == ENTRY_WEAPON_PREFAB) {
        return (WeaponPrefab *)entry->data.ptr;
    }
    fprintf(stderr, "Weapon prefab with key '%s' not found or wrong type\n", key);
    return NULL;  // weapon prefab not found or wrong type
}

/**
 * =====================================================================================================================
 */

TankPrefab* getTankPrefab(HashMap prefabMng, const char *key) {
    if (!prefabMng || !key) THROW_ERROR_AND_RETURN("Resource manager or key is NULL", NULL);
    if (prefabMng->type != MAP_PREFABS) THROW_ERROR_AND_RETURN("Prefabs manager is of wrong type", NULL);
    MapEntry *entry = MapGetEntry(prefabMng, key);
    if (entry && entry->type == ENTRY_TANK_PREFAB) {
        return (TankPrefab *)entry->data.ptr;
    }
    THROW_ERROR_AND_DO("Tank prefab with key ", fprintf(stderr, "'%s' not found\n", key); return NULL;);
}

/**
 * =====================================================================================================================
 */

Tile getTilePrefab(HashMap prefabMng, const char *key) {
    if (!prefabMng || !key) THROW_ERROR_AND_RETURN("Resource manager or key is NULL", (Tile){0});
    if (prefabMng->type != MAP_PREFABS) THROW_ERROR_AND_RETURN("Prefabs manager is of wrong type", (Tile){0});
    MapEntry *entry = MapGetEntry(prefabMng, key);
    if (entry && entry->type == ENTRY_TILE_PREFAB) {
        return *(Tile *)entry->data.ptr;
    }
    THROW_ERROR_AND_DO("Tile prefab with key ", fprintf(stderr, "'%s' not found\n", key); return (Tile){0};);
}

/**
 * =====================================================================================================================
 */

void loadPrefabs(ZENg zEngine, const char *filePath) {
    FILE *f = fopen(filePath, "rb");
    if (!f) {
        printf("Failed to open prefabs file: %s\n", filePath);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(size + 1);
    fread(data, 1, size, f);
    data[size] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    free(data);

    if (!root) {
        printf("Failed to parse prefabs JSON\n");
        return;
    }

    // The prefabs JSON is an array of prefab objects
    cJSON *prefabJson = NULL;
    cJSON_ArrayForEach(prefabJson, root) {
        cJSON *typeJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "prefabType");
        char *typeStr = typeJson ? typeJson->valuestring : NULL;

        cJSON *nameJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "prefabName");
        char *nameStr = nameJson ? nameJson->valuestring : NULL;

        if (!typeStr || !nameStr) {
            printf("Prefab missing type or name\n");
            continue;
        }
        if (strcmp(typeStr, "WEAPON") == 0) {
            cJSON *fireRateJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "fireRate");
            cJSON *projWJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "projW");
            cJSON *projHJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "projH");
            cJSON *projSpeedJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "projSpeed");
            cJSON *dmgJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "damage");
            cJSON *isPiercingJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "isPiercing");
            cJSON *isExplosiveJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "isExplosive");
            cJSON *projLifeTimeJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "projLifetime");
            cJSON *projTexturePathJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "projTexturePath");
            cJSON *projHitSoundPathJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "projHitSoundPath");
            cJSON *iconPathJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "iconPath");

            if (!fireRateJson || !projWJson || !projHJson || !projSpeedJson || !dmgJson ||
                !isPiercingJson || !isExplosiveJson || !projLifeTimeJson ||
                !projTexturePathJson || !projHitSoundPathJson || !iconPathJson) {
                printf("Incomplete weapon prefab data for '%s'\n", nameStr);
                continue;
            }

            double_t fireRate = fireRateJson->valuedouble;
            int projW = projWJson->valueint;
            int projH = projHJson->valueint;
            double_t projSpeed = projSpeedJson->valuedouble;
            int dmg = dmgJson->valueint;
            Uint8 isPiercing = (Uint8)isPiercingJson->valueint;
            Uint8 isExplosive = (Uint8)isExplosiveJson->valueint;
            double_t projLifeTime = projLifeTimeJson->valuedouble;
            char *projTexturePath = strdup(projTexturePathJson->valuestring);
            char *projHitSoundPath = strdup(projHitSoundPathJson->valuestring);
            char *iconPath = strdup(iconPathJson->valuestring);

            #ifdef DEBUG
                printf("===========================================================================================\n");
                printf(
                    "Parsed weapon prefab - name: %s, fireRate: %.2f, projW: %d, projH: %d, projSpeed: %.2f\n",
                    nameStr, fireRate, projW, projH, projSpeed
                );
                printf(
                    "ProjLifetime: %.2f, isPiercing: %hhu, isExplosive: %hhu, dmg: %d\n",
                    projLifeTime, isPiercing, isExplosive, dmg
                );
                printf(
                    "ProjTexturePath: %s, projHitSoundPath: %s, iconPath: %s\n================================\n",
                    projTexturePath, projHitSoundPath, iconPath
                );
            #endif

            WeaponPrefab *prefab = createWeaponPrefab(
                strdup(nameStr), fireRate, projW, projH, projSpeed, dmg, isPiercing, isExplosive, projLifeTime,
                projTexturePath, projHitSoundPath, iconPath
            );
            MapAddEntry(zEngine->prefabs, nameStr, (MapEntryVal){.ptr = prefab}, ENTRY_WEAPON_PREFAB);
        } else if (strcmp(typeStr, "TANK") == 0) {
            cJSON *entityTypeJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "entityType");
            cJSON *maxHealthJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "maxHealth");
            cJSON *speedJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "speed");
            cJSON *wJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "width");
            cJSON *hJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "height");
            cJSON *isSolidJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "isSolid");
            cJSON *texturePathJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "texturePath");
            cJSON *iconPathJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "iconPath");

            if (!entityTypeJson || !maxHealthJson || !speedJson || !wJson || !hJson ||
                !isSolidJson || !texturePathJson || !iconPathJson) {
                printf("Incomplete tank prefab data for '%s'\n", nameStr);
                continue;
            }

            EntityType entityType = (EntityType)entityTypeJson->valueint;
            Int32 maxHealth = maxHealthJson->valueint;
            double_t speed = speedJson->valuedouble;
            int w = wJson->valueint;
            int h = hJson->valueint;
            Uint8 isSolid = (Uint8)isSolidJson->valueint;
            char *texturePath = strdup(texturePathJson->valuestring);
            char *iconPath = strdup(iconPathJson->valuestring);

            #ifdef DEBUG
                printf("===========================================================================================\n");
                printf("Parsed tank prefab - name: %s, entityType: %d, maxHealth: %d, speed: %.2f\n",
                    nameStr, entityType, maxHealth, speed
                );
                printf(
                    "Width: %d, height: %d, isSolid: %hhu, texture: %s, icon: %s\n==================================\n",
                    w, h, isSolid, texturePath, iconPath
                );
            #endif
            TankPrefab *prefab = createTankPrefab(
                strdup(nameStr), entityType, maxHealth, speed, w, h, isSolid, texturePath, iconPath
            );
            MapAddEntry(zEngine->prefabs, nameStr, (MapEntryVal){.ptr = prefab}, ENTRY_TANK_PREFAB);
        } else if (strcmp(typeStr, "TILE") == 0) {
            // Defaults
            SDL_Texture *texture = NULL;
            double_t speedMod = 1.0;
            Int32 damage = 0;
            TileType type = TILE_EMPTY;
            Uint8 isWalkable = 1;
            Uint8 isSolid = 0;

            cJSON *typeJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "tileType");
            if (typeJson) type = (TileType)typeJson->valueint;
            cJSON *isWalkableJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "isWalkable");
            if (isWalkableJson) isWalkable = (Uint8)cJSON_IsTrue(isWalkableJson);
            cJSON *isSolidJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "isSolid");
            if (isSolidJson) isSolid = (Uint8)cJSON_IsTrue(isSolidJson);
            cJSON *texturePathJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "texturePath");
            char *texturePath = NULL;
            if (texturePathJson) {
                texturePath = texturePathJson->valuestring;
                texture = getTexture(zEngine->resources, texturePath);
            }
            cJSON *speedModJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "speedMod");
            if (speedModJson) speedMod = speedModJson->valuedouble;
            cJSON *damageJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "damage");
            if (damageJson) damage = damageJson->valueint;

            Tile *tile = calloc(1, sizeof(Tile));
            if (!tile) {
                fprintf(stderr, "Failed to allocate memory for TilePrefab\n");
                continue;
            }
            tile->type = type;
            tile->isWalkable = isWalkable;
            tile->isSolid = isSolid;
            tile->texture = texture;
            tile->speedMod = speedMod;
            tile->damage = damage;
            #ifdef DEBUG
            printf("===========================================================================================\n");
            printf("Parsed tile prefab - name: %s, type: %d, isWalkable: %hhu, isSolid: %hhu\n",
                nameStr, type, isWalkable, isSolid
            );
            printf("Texture path: %s, speedMod: %.2f, damage: %d\n============\n", texturePath, speedMod, damage);
            #endif
            MapAddEntry(zEngine->prefabs, nameStr, (MapEntryVal){.ptr = tile}, ENTRY_TILE_PREFAB);
        } else {
            printf("Unknown prefab type: %s\n", typeStr);
            continue;
        }
    }

    cJSON_Delete(root);
}

/**
 * =====================================================================================================================
 */

void freePrefabsManager(HashMap *prefabmng) {
    if (!prefabmng || !*prefabmng) return;
    if ((*prefabmng)->type != MAP_PREFABS) THROW_ERROR_AND_RETURN_VOID("Prefabs manager is of wrong type");

    size_t size = (*prefabmng)->size;
    for (Uint32 i = 0; i < size; i++) {
        MapEntry *entry = (*prefabmng)->entries[i];
        while (entry) {
            MapEntry *next = entry->next;
            switch(entry->type) {
                case ENTRY_WEAPON_PREFAB: {
                    WeaponPrefab *wp = (WeaponPrefab *)entry->data.ptr;
                    if (wp->projTexturePath) {
                        free(wp->projTexturePath);
                    }
                    if (wp->projHitSoundPath) {
                        free(wp->projHitSoundPath);
                    }
                    if (wp->name) {
                        free(wp->name);
                    }
                    if (wp->iconPath) {
                        free(wp->iconPath);
                    }
                    free(wp);
                    break;
                }
                case ENTRY_TANK_PREFAB: {
                    TankPrefab *tp = (TankPrefab *)entry->data.ptr;
                    if (tp->texturePath) {
                        free(tp->texturePath);
                    }
                    if (tp->name) {
                        free(tp->name);
                    }
                    if (tp->iconPath) {
                        free(tp->iconPath);
                    }
                    free(tp);
                    break;
                }
                case ENTRY_TILE_PREFAB: {
                    Tile *tile = (Tile *)entry->data.ptr;
                    free(tile);
                    break;
                }
                default: {
                    THROW_ERROR_AND_DO(
                        "Unknown prefab type ",
                        fprintf(stderr, "%d for key '%s'\n", entry->type, entry->key);
                    );
                    break;
                }
            }
            free(entry->key);  // Free the key string
            free(entry);  // Free the entry itself
            entry = next;  // Move to the next entry in the chain
        }
    }

    free((*prefabmng)->entries);
    free(*prefabmng);
    *prefabmng = NULL;
}

/**
 * =====================================================================================================================
 */

WeaponPrefab* createWeaponPrefab(
    const char *name, double_t fireRate, int projW, int projH, double_t projSpeed, int dmg, Uint8 isPiercing,
    Uint8 isExplosive, double_t projLifeTime, const char *projTexturePath, const char *projHitSoundPath,
    const char *iconPath
) {
    WeaponPrefab *prefab = calloc(1, sizeof(WeaponPrefab));
    if (!prefab) {
        fprintf(stderr, "Failed to allocate memory for WeaponPrefab\n");
        exit(EXIT_FAILURE);
    }

    prefab->name = (char *)name;
    prefab->fireRate = fireRate;
    prefab->projW = projW;
    prefab->projH = projH;
    prefab->projSpeed = projSpeed;
    prefab->dmg = dmg;
    prefab->isPiercing = isPiercing;
    prefab->isExplosive = isExplosive;
    prefab->projLifeTime = projLifeTime;
    prefab->projTexturePath = (char *)projTexturePath;
    prefab->projHitSoundPath = (char *)projHitSoundPath;
    prefab->iconPath = (char *)iconPath;

    return prefab;
}

/**
 * =====================================================================================================================
 */

TankPrefab* createTankPrefab(
    const char *name, EntityType entityType, Int32 maxHealth, double_t maxSpeed, int w, int h,
    Uint8 isSolid, const char *texturePath, const char *iconPath
) {
    TankPrefab *prefab = calloc(1, sizeof(TankPrefab));
    if (!prefab) {
        fprintf(stderr, "Failed to allocate memory for TankPrefab\n");
        exit(EXIT_FAILURE);
    }

    prefab->name = (char *)name;
    prefab->entityType = entityType;
    prefab->maxHealth = maxHealth;
    prefab->maxSpeed = maxSpeed;
    prefab->w = w;
    prefab->h = h;
    prefab->isSolid = isSolid;
    prefab->texturePath = (char *)texturePath;
    prefab->iconPath = (char *)iconPath;

    return prefab;
}
