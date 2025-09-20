#include "builder.h"
#include "engine.h"  // This is some dark magic here

void initPrefabsManager(PrefabsManager *prefabmng) {
    *prefabmng = calloc(1, sizeof(struct prefabsmng));
    if (!*prefabmng) {
        fprintf(stderr, "Failed to allocate memory for PrefabsManager\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * =====================================================================================================================
 */

static inline Uint32 hashFunc(const char *key) {
    Uint32 hash = 5381;
    int c;

    while (c = *key++)
        hash = ((hash << 5) + hash) + (Uint32)c; // hash * 33 + c

    return hash % PREFAB_HASHMAP_SIZE;
}

/**
 * =====================================================================================================================
 */

BuilderEntry* getPrefab(PrefabsManager prefabMng, const char *key) {
    Uint32 idx = hashFunc(key);

    BuilderEntry *entry = prefabMng->hashmap[idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;  // found the data
        }
        entry = entry->next;  // go to the next entry in the chain if there was a collision
    }
    fprintf(stderr, "Prefab with key '%s' not found\n", key);
    return NULL;  // data not found
}

/**
 * =====================================================================================================================
 */

WeaponPrefab* getWeaponPrefab(PrefabsManager prefabMng, const char *key) {
    BuilderEntry *entry = getPrefab(prefabMng, key);
    if (entry && entry->type == BUILDER_WEAPONS) {
        return (WeaponPrefab *)entry->data;  // cast the data to WeaponPrefab
    }
    fprintf(stderr, "Weapon prefab with key '%s' not found or wrong type\n", key);
    return NULL;  // weapon prefab not found or wrong type
}

/**
 * =====================================================================================================================
 */

TankPrefab* getTankPrefab(PrefabsManager prefabMng, const char *key) {
    BuilderEntry *entry = getPrefab(prefabMng, key);
    if (entry && entry->type == BUILDER_TANKS) {
        return (TankPrefab *)entry->data;  // cast the data to TankPrefab
    }
    fprintf(stderr, "Tank prefab with key '%s' not found or wrong type\n", key);
    return NULL;  // tank prefab not found or wrong type
}

/**
 * =====================================================================================================================
 */

Tile getTilePrefab(PrefabsManager prefabMng, const char *key) {
    BuilderEntry *entry = getPrefab(prefabMng, key);
    if (entry && entry->type == BUILDER_TILES) {
        return *(Tile *)entry->data;  // cast the data to TilePrefab
    }
    fprintf(stderr, "Tile prefab with key '%s' not found or wrong type\n", key);
    return (Tile){0};  // tile prefab not found or wrong type
}

/**
 * =====================================================================================================================
 */

void addPrefab(PrefabsManager prefabMng, const char *key, void *data, BuilderType type) {
    Uint32 idx = hashFunc(key);  // index at which to add the prefab
    BuilderEntry *new = calloc(1, sizeof(BuilderEntry));
    if (!new) {
        fprintf(stderr, "Failed to allocate memory for new prefab entry\n");
        exit(EXIT_FAILURE);
    }

    new->key = strdup(key);  // Safely copy the key
    if (!new->key) {
        fprintf(stderr, "Failed to allocate memory for prefab key\n");
        free(new);
        exit(EXIT_FAILURE);
    }

    new->data = data;
    new->type = type;  // set the data type
    new->next = prefabMng->hashmap[idx];  // the new entry is added at the beginning of the chain
    prefabMng->hashmap[idx] = new;  // update the hashmap to point to the new entry
}

/**
 * =====================================================================================================================
 */

void removePrefab(PrefabsManager prefabMng, const char *key) {
    Uint32 idx = hashFunc(key);

    BuilderEntry *entry = prefabMng->hashmap[idx];
    BuilderEntry *prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // found the data to remove
            if (prev) {
                prev->next = entry->next;  // link the previous entry to the one after the current
            } else {
                // if this is the first entry in the chain
                prefabMng->hashmap[idx] = entry->next;  // update the head of the chain
            }
            free(entry->key);  // free the key string
            free(entry);  // free the entry itself
            return;
        }
        prev = entry;
        entry = entry->next;  // go to the next entry in the chain
    }
    // if we reach here, the data was not found
    fprintf(stderr, "Resource with key '%s' not found for removal\n", key);
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

            if (!fireRateJson || !projWJson || !projHJson || !projSpeedJson || !dmgJson ||
                !isPiercingJson || !isExplosiveJson || !projLifeTimeJson ||
                !projTexturePathJson || !projHitSoundPathJson) {
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

            #ifdef DEBUG
                printf("===============================================================================================\n");
                printf(
                    "Parsed weapon prefab - name: %s, fireRate: %.2f, projW: %d, projH: %d, projSpeed: %.2f\n",
                    nameStr, fireRate, projW, projH, projSpeed
                );
                printf(
                    "ProjLifetime: %.2f, isPiercing: %hhu, isExplosive: %hhu, dmg: %d\n",
                    projLifeTime, isPiercing, isExplosive, dmg
                );
                printf("ProjTexturePath: %s, projHitSoundPath: %s\n=============\n", projTexturePath, projHitSoundPath);
            #endif

            WeaponPrefab *prefab = createWeaponPrefab(
                strdup(nameStr), fireRate, projW, projH, projSpeed, dmg, isPiercing, isExplosive, projLifeTime,
                projTexturePath, projHitSoundPath
            );
            addPrefab(zEngine->prefabs, nameStr, (void *)prefab, BUILDER_WEAPONS);
        } else if (strcmp(typeStr, "TANK") == 0) {
            cJSON *entityTypeJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "entityType");
            cJSON *maxHealthJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "maxHealth");
            cJSON *speedJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "speed");
            cJSON *wJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "width");
            cJSON *hJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "height");
            cJSON *isSolidJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "isSolid");
            cJSON *texturePathJson = cJSON_GetObjectItemCaseSensitive(prefabJson, "texturePath");

            if (!entityTypeJson || !maxHealthJson || !speedJson || !wJson || !hJson ||
                !isSolidJson || !texturePathJson) {
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

            #ifdef DEBUG
                printf("===========================================================================================\n");
                printf("Parsed tank prefab - name: %s, entityType: %d, maxHealth: %d, speed: %.2f\n",
                    nameStr, entityType, maxHealth, speed
                );
                printf("Width: %d, height: %d, isSolid: %hhu, texture: %s\n============\n", w, h, isSolid, texturePath);
            #endif
            TankPrefab *prefab = createTankPrefab(
                strdup(nameStr), entityType, maxHealth, speed, w, h, isSolid, texturePath
            );
            addPrefab(zEngine->prefabs, nameStr, (void *)prefab, BUILDER_TANKS);
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
            addPrefab(zEngine->prefabs, nameStr, (void *)tile, BUILDER_TILES);
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

void freePrefabsManager(PrefabsManager *prefabmng) {
    if (!prefabmng || !*prefabmng) {
        return;
    }

    for (Uint32 i = 0; i < PREFAB_HASHMAP_SIZE; i++) {
        BuilderEntry *entry = (*prefabmng)->hashmap[i];
        while (entry) {
            BuilderEntry *next = entry->next;
            switch(entry->type) {
                case BUILDER_WEAPONS: {
                    WeaponPrefab *wp = (WeaponPrefab *)entry->data;
                    if (wp->projTexturePath) {
                        free(wp->projTexturePath);
                    }
                    if (wp->projHitSoundPath) {
                        free(wp->projHitSoundPath);
                    }
                    if (wp->name) {
                        free(wp->name);
                    }
                    free(wp);
                    break;
                }
                case BUILDER_TANKS: {
                    TankPrefab *tp = (TankPrefab *)entry->data;
                    if (tp->texturePath) {
                        free(tp->texturePath);
                    }
                    if (tp->name) {
                        free(tp->name);
                    }
                    free(tp);
                    break;
                }
            }
            free(entry->key);  // free the key string
            free(entry);  // free the entry itself
            entry = next;  // move to the next entry in the chain
        }
    }

    free(*prefabmng);
    *prefabmng = NULL;
}

/**
 * =====================================================================================================================
 */

WeaponPrefab* createWeaponPrefab(
    const char *name, double_t fireRate, int projW, int projH, double_t projSpeed, int dmg, Uint8 isPiercing,
    Uint8 isExplosive, double_t projLifeTime, const char *projTexturePath, const char *projHitSoundPath
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

    return prefab;
}

/**
 * =====================================================================================================================
 */

TankPrefab* createTankPrefab(
    const char *name, EntityType entityType, Int32 maxHealth, double_t maxSpeed, int w, int h,
    Uint8 isSolid, const char *texturePath
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

    return prefab;
}