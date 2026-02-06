#include "ecs.h"
#include "global/global.h"
#include <stdlib.h>
#include <stdio.h>
#include "global/debug.h"

void initECS(ECS *ecs) {
    (*ecs) = malloc(sizeof(struct EeSiEs));
    if (!(*ecs)) {
        fprintf(stderr, "Failed to allocate memory for ECS\n");
        exit(EXIT_FAILURE);
    }

    (*ecs)->nextEntityID = 0;
    (*ecs)->entityCount = 0;
    (*ecs)->capacity = INIT_CAPACITY;
    (*ecs)->activeEntities = calloc(INIT_CAPACITY, sizeof(Entity));
    ASSERT((*ecs)->activeEntities != NULL, "Failed to allocate memory for ECS active entities\n");

	(*ecs)->entityToActiveIndex = calloc(INIT_CAPACITY, sizeof(Uint64));
    ASSERT((*ecs)->entityToActiveIndex != NULL, "Failed to allocate memory for the ECS entity->active index array map");

    (*ecs)->freeEntities = calloc(INIT_CAPACITY, sizeof(Entity));
    ASSERT((*ecs)->freeEntities != NULL, "Failed to allocate memory for the ECS free entities\n");

    (*ecs)->freeEntityCount = 0;
    (*ecs)->freeEntityCapacity = INIT_CAPACITY;

    // initialize the component arrays
    (*ecs)->componentsFlags = calloc((*ecs)->capacity, sizeof(bitset));
    ASSERT((*ecs)->componentsFlags != NULL, "Failed to allocate memory for ECS components flags\n");

    (*ecs)->components = calloc(COMPONENT_TYPE_COUNT, sizeof(ComponentTypeSet));
    ASSERT((*ecs)->components != NULL, "Failed to allocate memory for ECS components\n");

    for (Uint64 i = 0; i < COMPONENT_TYPE_COUNT; i++)
        (*ecs)->components[i].type = i;

    (*ecs)->depGraph = initDependencyGraph();
    kahnTopSort((*ecs)->depGraph);
}

/**
 * =====================================================================================================================
 */

Entity createEntity(ECS ecs, StateTagComponent state) {
    Entity entitty;
    if (ecs->freeEntityCount > 0) {
        // reuse an ID from the free entities array
        entitty = ecs->freeEntities[--ecs->freeEntityCount];
    } else {
        // no free entities, create a new one
        if (ecs->entityCount >= ecs->capacity) {
            // resize the ECS if needed
            Uint64 oldCapacity = ecs->capacity;
            ecs->capacity *= 2;
            LOG(DEBUG, "Resizing ECS from %lu to %lu\n", oldCapacity, ecs->capacity);

            Entity *tmpActive = realloc(ecs->activeEntities, ecs->capacity * sizeof(Entity));
            ASSERT(tmpActive != NULL, "Failed to reallocate memory for ECS active entities\n");
            ecs->activeEntities = tmpActive;

            bitset *tmpFlags = realloc(ecs->componentsFlags, ecs->capacity * sizeof(bitset));
            ASSERT(tmpFlags != NULL, "Failed to reallocate memory for ECS components flags\n");
            ecs->componentsFlags = tmpFlags;

            Uint64 *tmpEntityToActive = realloc(ecs->entityToActiveIndex, ecs->capacity * sizeof(Uint64));
            ASSERT(tmpEntityToActive != NULL, "Failed to reallocate memory for ECS entity->active array map\n");
            ecs->entityToActiveIndex = tmpEntityToActive;

            Entity *tmpFreeEntities = realloc(ecs->freeEntities, ecs->capacity * sizeof(Entity));
            ASSERT(tmpFreeEntities != NULL, "Failed to reallocate memory for ECS free entities\n");
            ecs->freeEntities = tmpFreeEntities;
            ecs->freeEntityCapacity = ecs->capacity;

            // initialize the new flags to 0
            for (Uint64 i = oldCapacity; i < ecs->capacity; i++) {
                ecs->componentsFlags[i] = 0;
            }

            // resize the components arrays - only dense part
            for (Uint64 i = 0; i < COMPONENT_TYPE_COUNT; i++) {
                if (ecs->components[i].dense) {
                    void **tmpDense = realloc(ecs->components[i].dense, ecs->capacity * sizeof(void *));
                    ASSERT(tmpDense != NULL, "Failed to reallocate memory for the dense array of component set %lu", i);
                    ecs->components[i].dense = tmpDense;

                    Entity *tmpDenseToEntity = realloc(
                        ecs->components[i].denseToEntity, ecs->capacity * sizeof(Entity));
                    ASSERT(tmpDenseToEntity != NULL,
                        "Failed to reallocate memory for dense index -> entity array map of component set %lu", i);
                    ecs->components[i].denseToEntity = tmpDenseToEntity;
                }
            }
        }
        entitty = ecs->nextEntityID++;
    }

    ecs->componentsFlags[entitty] = 0;  // the new entity has no components
    ecs->activeEntities[ecs->entityCount++] = entitty;  // add it to the active entities array
    // Create the mapping for later removal
    ecs->entityToActiveIndex[entitty] = ecs->entityCount - 1;

    // Add the state tag component
    StateTagComponent *stateTag = calloc(1, sizeof(StateTagComponent));
    ASSERT(stateTag != NULL, "Failed to allocate memory for the state tag of entity %lu\n", entitty);
    *stateTag = state; 
    addComponent(ecs, entitty, STATE_TAG_COMPONENT, (void*)stateTag);

    ActiveTagComponent *activeTag = calloc(1, sizeof(ActiveTagComponent));
    ASSERT(activeTag != NULL, "Failed allocating memory for the active tag of an entity");
    *activeTag = 1;
    addComponent(ecs, entitty, ACTIVE_TAG_COMPONENT, activeTag);

    LOG(DEBUG, "Created entity %lu belonging to state %d\n", entitty, *stateTag);
    return entitty;
}

/**
 * =====================================================================================================================
 */

void addComponent(ECS ecs, Entity id, ComponentType compType, void *component) {
    Uint64 page = id / PAGE_SIZE;  // determine the page for the entity
    Uint64 index = id % PAGE_SIZE;  // determine the index within the page

    if (compType >= COMPONENT_TYPE_COUNT) {
        fprintf(stderr, "Invalid component type %d\n", compType);
        return;
    }

    // Check if the page exists, if not, allocate it
    if (ecs->components[compType].sparse == NULL || ecs->components[compType].pageCount <= page) {
        #ifdef DEBUG
            printf("Allocating memory for a page of the component %d's array\n", compType);
        #endif

        // Allocate memory for a new page
        ecs->components[compType].sparse = realloc(ecs->components[compType].sparse, (page + 1) * sizeof(Uint64 *));
        if (!ecs->components[compType].sparse) {
            fprintf(stderr, "Failed to reallocate memory for ECS sparse set\n");
            exit(EXIT_FAILURE);
        }
        // Allocate PAGE_SIZE memory for each page
        for (Uint64 i = ecs->components[compType].pageCount; i <= page; i++) {
            ecs->components[compType].sparse[i] = calloc(PAGE_SIZE, sizeof(Uint64));
            if (!ecs->components[compType].sparse[i]) {
                fprintf(stderr, "Failed to allocate memory for ECS sparse set page %lu\n", i);
                exit(EXIT_FAILURE);
            }
        }
        ecs->components[compType].pageCount = page + 1;
    }
    
    // Check if the dense array exists, if not, allocate it
    if (ecs->components[compType].dense == NULL) {
        #ifdef DEBUG
            printf("Allocating memory for the dense array of the component %d\n", compType);
        #endif

        // initially, it will have INIT_CAPACITY memory allocated
        ecs->components[compType].dense = calloc(ecs->capacity, sizeof(void *));
        ecs->components[compType].denseToEntity = calloc(ecs->capacity, sizeof(Entity));
        if (!ecs->components[compType].dense || !ecs->components[compType].denseToEntity) {
            fprintf(stderr, "Failed to allocate memory for ECS dense set\n");
            exit(EXIT_FAILURE);
        }
    }

    // Check if the entity ID will fit in the dense array
    if (id >= ecs->capacity) {
        #ifdef DEBUG
            printf("Reallocating memory for the dense array of the component %d's array\n", compType);
        #endif

        // Resize the dense array if needed
        void **tmpDense = realloc(ecs->components[compType].dense, ecs->capacity * sizeof(void *));
        Entity *tmpDenseToEntity = realloc(ecs->components[compType].denseToEntity, ecs->capacity * sizeof(Entity));
        if (!tmpDense || !tmpDenseToEntity) {
            fprintf(stderr, "Failed to reallocate memory for ECS dense set\n");
            exit(EXIT_FAILURE);
        }
        ecs->components[compType].dense = tmpDense;
        ecs->components[compType].denseToEntity = tmpDenseToEntity;
    }

    // Allocations done, now add the component
    Uint64 denseIdx = ecs->components[compType].denseSize;
    ecs->components[compType].sparse[page][index] = denseIdx;
    ecs->components[compType].dense[denseIdx] = component;
    ecs->components[compType].denseToEntity[denseIdx] = id;  // map the component to its entity ID
    ecs->components[compType].denseSize++;

    // And set the corresponding bit
    ecs->componentsFlags[id] |= (1 << compType);

    const ComponentType fineGrainedType[] = {
        HEALTH_COMPONENT,
    };
    const size_t fineGrainedCount = sizeof(fineGrainedType) / sizeof(ComponentType);
    for (size_t i = 0; i < fineGrainedCount; i++) {
        if (compType == fineGrainedType[i]) {
            markComponentDirty(ecs, id, compType);  // Mark fine-grained components as dirty at creation
            break;
        }
    }

    #ifdef DEBUG
        printf("Added component %d to entity %ld\n", compType, id);
    #endif
}

/**
 * =====================================================================================================================
 */

void markComponentDirty(ECS ecs, Entity id, ComponentType compType) {
    if (!ecs) {
        fprintf(stderr, "ECS is NULL, cannot mark component dirty\n");
        return;
    }

    if (id >= ecs->capacity || id < 0) {
        printf("Warning: Attempting to mark component dirty for entity %ld which is out of bounds\n", id);
        return;
    }

    if (ecs->components[compType].dirtyCount >= ecs->components[compType].dirtyCapacity) {
        // Resize the dirty entities array if needed

        // Double the capacity if it is full, begin with 4
        ecs->components[compType].dirtyCapacity =
        ecs->components[compType].dirtyCapacity == 0 ? 4 : ecs->components[compType].dirtyCapacity * 2;
        Entity *newDirtyEntities = realloc(
            ecs->components[compType].dirtyEntities,
            ecs->components[compType].dirtyCapacity * sizeof(Entity)
        );
        if (!newDirtyEntities) {
            fprintf(stderr, "Failed to resize dirty entities array for component %d\n", compType);
            return;
        }
        ecs->components[compType].dirtyEntities = newDirtyEntities;
    }

    ecs->components[compType].dirtyEntities[ecs->components[compType].dirtyCount++] = id;
    // Propagate the dirty state through the dependency graph
    propagateSystemDirtiness(ecs->depGraph->nodes[componentToSystem(compType)]);
}

/**
 * =====================================================================================================================
 */

void propagateSystemDirtiness(SystemNode *node) {
    if (!node) {
        fprintf(stderr, "Cannot propagate dirty state, node is NULL\n");
        return;
    }

    for (Uint64 i = 0; i < node->dependentsCount; i++) {
        SystemNode *dependent = node->dependents[i];
        if (dependent->isFineGrained) {
            continue; // Fine-grained systems handle their own dirty jobs
        }
        if (dependent->isDirty == 0) {
            dependent->isDirty = 1;  // Mark the dependent system as dirty
            propagateSystemDirtiness(dependent);  // Recursively propagate the dirty state
        }
    }
}

/**
 * =====================================================================================================================
 */

void unmarkComponentDirty(ECS ecs, ComponentType compType) {
    if (!ecs) {
        fprintf(stderr, "ECS is NULL, cannot unmark component dirty\n");
        return;
    }

    if (compType >= COMPONENT_TYPE_COUNT) {
        fprintf(stderr, "Invalid component type %d\n", compType);
        return;
    }

    // The first entity becomes clean
    ComponentTypeSet *comp = &ecs->components[compType];
    comp->dirtyEntities[0] = comp->dirtyEntities[--comp->dirtyCount];
}

/**
 * =====================================================================================================================
 */

SystemNode* createSystemNode(SystemType type, void (*update)(ZENg, double_t), Uint8 isFineGrained) {
    SystemNode *node = calloc(1, sizeof(SystemNode));
    if (!node) {
        fprintf(stderr, "Failed to allocate memory for system node #%d\n", type);
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->update = update;
    node->isDirty = 1;  // At creation force the system to update
    node->isFineGrained = isFineGrained;
    return node;
}

/**
 * =====================================================================================================================
 */

SystemType componentToSystem(ComponentType compType) {
    switch (compType) {
        case LIFETIME_COMPONENT:
            return SYS_LIFETIME;
        case WEAPON_COMPONENT:
            return SYS_WEAPONS;
        case VELOCITY_COMPONENT:
            return SYS_VELOCITY;
        case POSITION_COMPONENT:
            return SYS_POSITION;
        case HEALTH_COMPONENT:
            return SYS_HEALTH;
        case RENDER_COMPONENT:
            return SYS_RENDER;
        default:
            fprintf(stderr, "No corresponding system for component type %d\n", compType);
            return -1;  // Invalid system type
    }
}

 /**
 * =====================================================================================================================
 */

void insertSystem(DependencyGraph *graph, SystemNode *node) {
    if (!graph || !node) {
        fprintf(stderr, "Graph or node is NULL, cannot insert system\n");
        return;
    }

    if (graph->nodeCount >= SYS_COUNT) {
        fprintf(stderr, "Dependency graph is full, cannot insert more systems\n");
        return;
    }

    graph->nodes[node->type] = node;
    graph->nodeCount++;
}

/**
 * =====================================================================================================================
 */

void addSystemDependency(SystemNode *dependency, SystemNode *dependent) {
    if (!dependency || !dependent) {
        fprintf(stderr, "Cannot add dependency, one of the nodes is NULL\n");
        return;
    }

    // Add the dependent node to the dependency's dependents array
    SystemNode **newDependencies = realloc(
        dependency->dependents,
        (dependency->dependentsCount + 1) * sizeof(SystemNode *)
    );
    if (!newDependencies) {
        fprintf(stderr, "Failed to resize dependents array for system %d\n", dependency->type);
        return;
    }
    dependency->dependents = newDependencies;
    dependency->dependents[dependency->dependentsCount++] = dependent;

    // Add the dependency node to the dependent's dependencies array
    SystemNode **newDependents = realloc(
        dependent->dependencies,
        (dependent->dependenciesCount + 1) * sizeof(SystemNode *)
    );
    if (!newDependents) {
        fprintf(stderr, "Failed to resize dependencies array for system %d\n", dependent->type);
        return;
    }
    dependent->dependencies = newDependents;
    dependent->dependencies[dependent->dependenciesCount++] = dependency;
}

/**
 * =====================================================================================================================
 */

void kahnTopSort(DependencyGraph *graph) {
    if (!graph) {
        fprintf(stderr, "Dependency graph is NULL, cannot perform topological sort\n");
        return;
    }

    #ifdef DEBUG
    const char* sysNames[] = {
        "SYS_LIFETIME",
        "SYS_WEAPONS",
        "SYS_VELOCITY",
        "SYS_WORLD_COLLISIONS",
        "SYS_ENTITY_COLLISIONS",
        "SYS_POSITION",
        "SYS_HEALTH",
        "SYS_TRANSFORM",
        "SYS_RENDER",
        "SYS_UI"
    };
    printf("Performing Kahn's topological sort on the dependency graph with %lu nodes:\n", graph->nodeCount);
    for (Uint64 i = 0; i < graph->nodeCount; i++) {
        printf("%s ", sysNames[graph->nodes[i]->type]);
    }
    printf("\n");
    #endif
    
    graph->sortedNodes = calloc(SYS_COUNT, sizeof(SystemNode *));
    if (!graph->sortedNodes) {
        fprintf(stderr, "Failed to allocate memory for sorted nodes array\n");
        return;
    }

    Uint64 inDegree[SYS_COUNT] = {0};

    // Calculate in-degrees of each node
    for (Uint64 i = 0; i < SYS_COUNT; i++) {
        SystemNode *node = graph->nodes[i];
        if (node) {
            for (Uint64 j = 0; j < node->dependentsCount; j++) {
                inDegree[node->dependents[j]->type]++;
            }
        }
    }

    // Create a queue and populate it with nodes of 0 incoming edges
    Uint64 queueSize = 0;
    SystemNode *queue[SYS_COUNT] = {0};

    for (Uint64 i = 0; i < SYS_COUNT; i++) {
        if (inDegree[i] == 0 && graph->nodes[i]) {
            queue[queueSize++] = graph->nodes[i];
        }
    }

    // Kahn's algorithm
    Uint64 sortedIndex = 0;
    while (queueSize > 0) {
        SystemNode *current = queue[--queueSize];
        graph->sortedNodes[sortedIndex++] = current;

        for (Uint64 j = 0; j < current->dependentsCount; j++) {
            SystemNode *dependent = current->dependents[j];
            // Decrease the in-degree of the dependent nodes of each processed node
            inDegree[dependent->type]--;

            if (inDegree[dependent->type] == 0) {
                // When the in-degree of a node becomes 0, queue it to be processed
                queue[queueSize++] = dependent;
            }
        }
    }

    if (sortedIndex != SYS_COUNT) {
        fprintf(stderr, "Kahn's sort failed: graph may have cycles or missing nodes (sorted %lu of %d)\n", sortedIndex, SYS_COUNT);
    }

    #ifdef DEBUG
        printf("Sorted systems order:\n");
        for (Uint64 i = 0; i < graph->nodeCount; i++) {
            printf("%s -> ", sysNames[graph->sortedNodes[i]->type]);
        }
        printf("\n");
    #endif
}

/**
 * =====================================================================================================================
 */

void deleteEntity(ECS ecs, Entity id) {
    if (!ecs) {
        fprintf(stderr, "ECS is NULL, cannot delete entity\n");
        return;
    }

    if (id >= ecs->capacity || id < 0) {
        printf("Warning: Attempting to delete entity %ld which is out of bounds\n", id);
        return;
    }

    if (ecs->componentsFlags[id] == 0) {
        // entity has no components, just free the ID
        ecs->freeEntities[ecs->freeEntityCount++] = id;
        ecs->entityCount--;
        return;
    }
    
    // Free all components associated with this entity
    for (Uint64 i = 0; i < COMPONENT_TYPE_COUNT; i++) {
        bitset componentFlag = 1 << i;
        
        // Check if entity has this component
        if (ecs->componentsFlags[id] & componentFlag) {
            // Find the component in the sparse set
            Uint64 page = id / PAGE_SIZE;
            Uint64 index = id % PAGE_SIZE;
            ComponentTypeSet *compSet = &ecs->components[i];
            
            if (
                compSet->sparse && 
                page < compSet->pageCount &&
                compSet->dense
            ) {
                Uint64 denseIndex = compSet->sparse[page][index];
                void *component = compSet->dense[denseIndex];
                
                // Free the component data based on its type
                if (component) {
                    switch (i) {
                        case COLLISION_COMPONENT: {
                            CollisionComponent *colComp = (CollisionComponent*)component;
                            if (colComp->hitbox) free(colComp->hitbox);
                            break;
                        }
                        case RENDER_COMPONENT: {
                            RenderComponent *render = (RenderComponent*)component;
                            if (render->destRect) free(render->destRect);
                            break;
                        }
                        case LOADOUT_COMPONENT: {
                            LoadoutComponent *loadout = (LoadoutComponent*)component;
                            // Only the nodes get freed, the weapons are freed as separate entities
                            freeList(&loadout->currSecondaryGun);
                            break;
                        }
                        case WEAPON_COMPONENT: {
                            WeaponComponent *weapComp = (WeaponComponent*)component;
                            if (weapComp->name) free(weapComp->name);
                            if (weapComp->projComp) free(weapComp->projComp);
                            break;
                        }
                    }
                    free(component);
                    
                    // Remove from dense array by swapping with the last element
                    Uint64 lastDenseIndex = compSet->denseSize - 1;
                    if (denseIndex != lastDenseIndex) {
                        // Move the last element to the position of the removed element
                        compSet->dense[denseIndex] = compSet->dense[lastDenseIndex];
                        
                        // Use the reverse mapping
                        Entity lastEntity = compSet->denseToEntity[lastDenseIndex];
                        
                        // Update the sparse pointer for the moved entity
                        Uint64 lastPage = lastEntity / PAGE_SIZE;
                        Uint64 lastIndex = lastEntity % PAGE_SIZE;
                        compSet->sparse[lastPage][lastIndex] = denseIndex;

                        // update the denseToEntity mapping
                        compSet->denseToEntity[denseIndex] = lastEntity;
                    }
                    
                    // Decrease the size of the dense array
                    compSet->denseSize--;
                    compSet->dense[lastDenseIndex] = NULL;  // avoid dangling pointer
                }
            }
        }
    }
    
    // Reset the components flags for this entity
    ecs->componentsFlags[id] = 0;
    
    // Add the entity ID to the free list for reuse
    if (ecs->freeEntityCount >= ecs->freeEntityCapacity) {
        // Resize the free entities array if needed
        ecs->freeEntityCapacity *= 2;
        Entity *newFreeEntities = realloc(
            ecs->freeEntities,
            ecs->freeEntityCapacity * sizeof(Entity)
        );
        if (!newFreeEntities) {
            fprintf(stderr, "Failed to resize free entities array\n");
            return;
        }
        ecs->freeEntities = newFreeEntities;
    }

    // Remove the entity from the active entities array
    Uint64 indexInActiveEntities = ecs->entityToActiveIndex[id];
	Entity lastEntity = ecs->activeEntities[ecs->entityCount - 1];
    // Swap the removed entity with the last one. Count decremental will make it inactive
    ecs->activeEntities[indexInActiveEntities] = lastEntity;
	ecs->entityToActiveIndex[lastEntity] = indexInActiveEntities;
    // Add this entity to the free list
    ecs->freeEntities[ecs->freeEntityCount++] = id;
    ecs->entityCount--;
    
    #ifdef DEBUG
        printf("Deleted entity with ID %ld\n", id);
    #endif
}

/**
 * =====================================================================================================================
 */

void sweepState(ECS ecs, GameStateType stateType) {
    // Iterating backwards because the deletion changes the entities array order
    for (Int64 i = ecs->components[STATE_TAG_COMPONENT].denseSize - 1; i >= 0; i--) {
        StateTagComponent *stateTag = ecs->components[STATE_TAG_COMPONENT].dense[i];
        if (*stateTag == stateType) {
            Entity owner = ecs->components[STATE_TAG_COMPONENT].denseToEntity[i];
            deleteEntity(ecs, owner);
        }
    }
}

/**
 * =====================================================================================================================
 */

void freeECS(ECS ecs) {
    if (ecs) {
        // Free all entities' components first
        if (ecs->entityCount > 0) {
            for (Int64 i = ecs->activeEntities[ecs->entityCount - 1]; i >= 0; i--) {
                deleteEntity(ecs, ecs->activeEntities[i]);
            }
        }
        if (ecs->componentsFlags) {
            free(ecs->componentsFlags);
        }
        if (ecs->activeEntities) {
            free(ecs->activeEntities);
        }
		if (ecs->entityToActiveIndex) {
			free(ecs->entityToActiveIndex);
		}
        if (ecs->components) {
            for (Uint64 i = 0; i < COMPONENT_TYPE_COUNT; i++) {
                if (ecs->components[i].dirtyEntities) {
                    free(ecs->components[i].dirtyEntities);
                    ecs->components[i].dirtyEntities = NULL;
                }
                if (ecs->components[i].sparse) {
                    for (Uint64 j = 0; j < ecs->components[i].pageCount; j++) {
                        if (ecs->components[i].sparse[j]) {
                            free(ecs->components[i].sparse[j]);
                        }
                    }
                    free(ecs->components[i].sparse);
                    ecs->components[i].sparse = NULL;
                }
                if (ecs->components[i].dense) {
                    free(ecs->components[i].dense);
                    ecs->components[i].dense = NULL;
                }
                if (ecs->components[i].denseToEntity) {
                    free(ecs->components[i].denseToEntity);
                    ecs->components[i].denseToEntity = NULL;
                }
            }
            free(ecs->components);
        }
        if (ecs->freeEntities) {
            free(ecs->freeEntities);
        }
        if (ecs->depGraph) {
            if (ecs->depGraph->nodes) {
                for (Uint64 i = 0; i < ecs->depGraph->nodeCount; i++) {
                    SystemNode *curr = ecs->depGraph->nodes[i];
                    if (curr) {
                        if (curr->dependencies) {
                            free(curr->dependencies);
                        }
                        if (curr->dependents) {
                            free(curr->dependents);
                        }
                        free(curr);
                    }
                }
                free(ecs->depGraph->nodes);
            }
            if (ecs->depGraph->sortedNodes) free(ecs->depGraph->sortedNodes);
            free(ecs->depGraph);
        }
        free(ecs);
    }
}
