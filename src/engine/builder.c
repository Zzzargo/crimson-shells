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

void loadPrefabs(PrefabsManager prefabMng, const char *filePath) {
    if (!prefabMng || !filePath) {
        fprintf(stderr, "Invalid prefab manager or file path\n");
        return;
    }

    FILE *fin = fopen(filePath, "r");
    if (!fin) {
        fprintf(stderr, "Failed to open prefab file: %s\n", filePath);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fin)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        char *typeStr = strtok(line, ":");
        char *key = strtok(NULL, ":");
        char *dataStr = strtok(NULL, "\n");

        if (!typeStr || !key || !dataStr) {
            fprintf(stderr, "Malformed line in prefab file: %s\n", line);
            continue;
        }

        #ifdef DEBUG
            printf("===============================================================================================\n");
            printf("Loading prefab - Type: %s, Key: %s, Data: %s\n", typeStr, key, dataStr);
        #endif

        BuilderType type;
        if (strcmp(typeStr, "WEAPON") == 0) {
            type = BUILDER_WEAPONS;
            char *projTexturePath = calloc(64, sizeof(char));
            char *projHitSoundPath = calloc(64, sizeof(char));
            double_t fireRate = 0.0, projSpeed = 0.0, projLifetime = 0.0;
            int projW = 0, projH = 0, dmg = 0;
            Uint8 isPiercing = 0, isExplosive = 0;
            sscanf(dataStr, "%lf:%d:%d:%lf:%d:%hhu:%hhu:%lf:%[^:]:%s",
                &fireRate, &projW, &projH, &projSpeed, &dmg,
                &isPiercing, &isExplosive, &projLifetime,
                projTexturePath, projHitSoundPath
            );
            #ifdef DEBUG
                printf(
                    "Parsed weapon prefab - name: %s, fireRate: %.2f, projW: %d, projH: %d, projSpeed: %.2f\n",
                    key, fireRate, projW, projH, projSpeed
                );
                printf(
                    "ProjLifetime: %.2f, isPiercing: %hhu, isExplosive: %hhu, dmg: %d\n",
                    projLifetime, isPiercing, isExplosive, dmg
                );
                printf("ProjTexturePath: %s, projHitSoundPath: %s\n", projTexturePath, projHitSoundPath);
            #endif

            WeaponPrefab *prefab = createWeaponPrefab(
                strdup(key), fireRate, projW, projH, projSpeed, dmg, isPiercing, isExplosive, projLifetime,
                projTexturePath, projHitSoundPath
            );
            addPrefab(prefabMng, key, (void *)prefab, type);
        } else if (strcmp(typeStr, "TANK") == 0) {
            type = BUILDER_TANKS;
            EntityType entityType = 0;
            Int32 maxHealth = 0;
            double_t speed = 0.0;
            int w = 0, h = 0;
            Uint8 isSolid = 0;
            char *texturePath = calloc(256, sizeof(char));
            sscanf(dataStr, "%d:%d:%lf:%d:%d:%hhu:%s",
                (int *)&entityType, &maxHealth, &speed, &w, &h, &isSolid, texturePath
            );
            #ifdef DEBUG
                printf("Parsed tank prefab - name: %s, entityType: %d, maxHealth: %d, maxSpeed: %.2f\n",
                    key, entityType, maxHealth, speed
                );
                printf("Width: %d, height: %d, isSolid: %hhu, texture: %s\n", w, h, isSolid, texturePath);
            #endif
            TankPrefab *prefab = createTankPrefab(
                strdup(key), entityType, maxHealth, speed, w, h, isSolid, texturePath
            );
            addPrefab(prefabMng, key, (void *)prefab, type);
        } else if (strcmp(typeStr, "PROJECTILE") == 0) {
            type = BUILDER_PROJECTILES;
        } else {
            fprintf(stderr, "Unknown prefab type '%s' in line: %s\n", typeStr, line);
            continue;
        }
    }

    fclose(fin);
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

    prefab->name = name;
    prefab->fireRate = fireRate;
    prefab->projW = projW;
    prefab->projH = projH;
    prefab->projSpeed = projSpeed;
    prefab->dmg = dmg;
    prefab->isPiercing = isPiercing;
    prefab->isExplosive = isExplosive;
    prefab->projLifeTime = projLifeTime;
    prefab->projTexturePath = projTexturePath;
    prefab->projHitSoundPath = projHitSoundPath;

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

    prefab->name = name;
    prefab->entityType = entityType;
    prefab->maxHealth = maxHealth;
    prefab->maxSpeed = maxSpeed;
    prefab->w = w;
    prefab->h = h;
    prefab->isSolid = isSolid;
    prefab->texturePath = texturePath;

    return prefab;
}