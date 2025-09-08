# Crimson Shells

**Crimson Shells** is a 2D top-down tank game in development built from scratch in **C** using **SDL2**, including *SDL2_mixer*, *SDL2_image*, and *SDL2_ttf*

---

## Features

- **Gameplay**
  - Player-controlled tank with destructible environment (walls, obstacles)
  - Static, data-driven arena levels

- **Engine Architecture**
  - Written in C with SDL2 for graphics, audio, and input
  - **Engine structure**:
    ```c
    typedef struct engine {
        DisplayManager display;
        ResourceManager resources;
        InputManager inputMng;
        StateManager stateMng;
        ECS ecs;
        Arena map;
    } *ZENg;
    ```
  - Custom **Entity-Component-System (ECS)** using **sparse sets with pagination** for cache-friendly component access
  - **Systems** are connected in a **dependency graph**; execution order is determined by **topological sorting using Kahn's algorithm**
  - System dirtiness - only “dirty” systems are run each frame for efficiency
  - Fine/coarse-grained system execution - fine-grained systems track only entities marked dirty, while coarse-grained ones loop through all the entities with the needed component

- **State Management**
  - Game states are handled as **stack nodes**
  - Each state is described by a set of functions that handle its lifecycle: *onEnter*, *onExit*, *handleEvents*, *handleInput*
  - New states are **pushed** on top (e.g., main menu → play state), and that also makes all the previous states available and the transitions easy

- **Game Loop**
  - Delta time calculation for frame-independent updates
  - FPS capped at 60 frames per second (for now)
  - Event polling integrated with state manager
  - Renderer clears screen and presents the current frame every loop iteration

---

## Engine Highlights

- **ECS Structure**
    ```c
    typedef struct EeSiEs {
        Entity nextEntityID;
        Entity *activeEntities;
        Uint64 entityCount;
        Uint64 capacity;
        Entity *freeEntities;
        Uint64 freeEntityCount;
        Uint64 freeEntityCapacity;

        bitset *componentsFlags;
        Component *components;

        DependencyGraph *depGraph;
    } *ECS;
    ```
  - Sparse set implementation for component storage
  - Dirty tracking for efficient system updates

- **Component Structure**
    ```c
    typedef struct {
        Uint64 **sparse;
        void **dense;
        Entity *denseToEntity;
        Uint64 denseSize;
        Uint64 pageCount;
        ComponentType type;

        Entity *dirtyEntities;
        Uint64 dirtyCount;
        Uint64 dirtyCapacity;
    } Component;
    ```

- **Systems**
  - Dependency-aware execution
  - Dirty propagation ensures only relevant systems run
  - Fine/coarse-grained control for system execution

---

## Installation & Build

**Requirements**:  
- SDL2, SDL2_image, SDL2_mixer, SDL2_ttf  
- C compiler (tested with GCC 11.4.0)  
- CMake 3.10+  

**Build Instructions**:
```bash
git clone https://github.com/Zzzargo/crimson-shells.git
cd crimson-shells
mkdir build && cd build
cmake ..
make
