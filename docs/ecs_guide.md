# Entity Component System (ECS) Guide

This guide details the architectural philosophy, data structures, and operational mechanics of the custom **Entity Component System (ECS)** built for the R-Type engine.

---

## 1. Paradigm Shift: ECS vs Classic OOP

### The Inheritance Trap in Object-Oriented Game Engines

In classic Object-Oriented Programming (OOP), game entities are organized into deep class inheritance hierarchies. While intuitive for small projects, this model quickly leads to architectural bottlenecks:

- **The Diamond Problem:** If an entity needs traits from multiple branches (e.g., a flying destructible enemy ship that is also an interactive obstacle), multiple inheritance causes ambiguous dispatch and bloated interfaces.
- **Tight Coupling:** Data and behavior are fused together into monolithic base classes (`GameObject`, `Actor`, `Pawn`).
- **Cache Inefficiency:** In OOP, entities are allocated on the heap as scattered objects connected by pointers (`std::vector` of heap pointers). Iterating over hundreds of objects triggers frequent CPU cache misses due to pointer chasing.

```mermaid
graph TD
    subgraph Classic OOP: Deep Rigid Inheritance
        GameObject["GameObject<br/><i>(Position, Transform, NetworkID)</i>"]
        Movable["MovableObject<br/><i>(Velocity, Speed)</i>"]
        Destructible["DestructibleObject<br/><i>(Health, Armor)</i>"]
        PlayerShip["PlayerShip<br/><i>(Input, Shield)</i>"]
        BydoEnemy["BydoEnemy<br/><i>(AI, Patrol)</i>"]
        DestructibleWall["DestructibleWall<br/><i>(Collision Box)</i>"]

        GameObject --> Movable
        GameObject --> Destructible
        Movable --> PlayerShip
        Destructible --> PlayerShip
        Movable --> BydoEnemy
        Destructible --> BydoEnemy
        Destructible --> DestructibleWall
    end
```

### The ECS Composition Model

The Entity Component System embraces **Data-Oriented Design (DOD)** by strictly decoupling identity, data, and behavior:

- **Entities:** Pure numeric handles (IDs) without fields or methods.
- **Components:** Plain Old Data (POD) structs with no logic.
- **Systems:** Stateless algorithms that process batches of matching components.

```mermaid
graph TD
    subgraph ECS: Flexible Dynamic Composition
        E1["Entity #1: Player Ship"]
        E2["Entity #2: Bydo Slave"]
        E3["Entity #3: Laser Beam"]
        E4["Entity #4: Space Debris"]

        CPos["Position Component"]
        CVel["Velocity Component"]
        CHp["Health Component"]
        CCol["Collider Component"]
        CSpr["Sprite Component"]
        CWpn["Weapon Component"]

        E1 -.-> CPos
        E1 -.-> CVel
        E1 -.-> CHp
        E1 -.-> CCol
        E1 -.-> CSpr
        E1 -.-> CWpn

        E2 -.-> CPos
        E2 -.-> CVel
        E2 -.-> CHp
        E2 -.-> CCol
        E2 -.-> CSpr

        E3 -.-> CPos
        E3 -.-> CVel
        E3 -.-> CCol
        E3 -.-> CSpr

        E4 -.-> CPos
        E4 -.-> CHp
        E4 -.-> CCol
        E4 -.-> CSpr
    end
```

---

## 2. Core Concepts in Our Engine

### 2.1 Entities: Generational ID Packing

In our engine, an `Entity` is not a heap-allocated object, but a lightweight 32-bit unsigned integer (`std::uint32_t`).

```mermaid
classDiagram
    class Entity_ID_32_Bits {
        +8 bits : Generation Counter
        +24 bits : Slot Index
    }
```

- **Slot Index (24 bits):** Identifies the entity's row in component arrays. Supports up to 16,777,215 simultaneous slots (2^24 - 1).
- **Generation Counter (8 bits):** Tracks how many times a given slot index has been recycled (0 to 255).
- **Null Entity (`0xFFFFFFFF`):** A sentinel value denoting an invalid or unassigned entity.

#### Stale Handle Prevention & Slot Recycling

A common pitfall in game development is the **dangling entity reference** (e.g., a homing missile tracking an enemy entity that was destroyed in the previous frame).

Our `EntityManager` prevents stale handle bugs through generational validation:

```mermaid
sequenceDiagram
    autonumber
    participant App as System / Game Logic
    participant EM as EntityManager
    participant Queue as Free Index Queue

    App->>EM: create() -> Allocates Slot 42, Gen 0 (ID: 0x0000002A)
    Note over App,EM: Entity is active and receives components
    App->>EM: destroy(0x0000002A)
    Note over EM: Slot 42 marked dead
    Note over EM: Generation 42 bumped from 0 -> 1
    EM->>Queue: Push Slot 42 to back of queue
    
    Note over EM: Slot 42 waits until MIN_FREE_INDICES (1024) recycle
    EM->>App: create() -> Recycles Slot 42, Gen 1 (ID: 0x0100002A)
    
    App->>EM: isAlive(0x0000002A) [Old Handle]
    Note over EM: Checks: Stored Gen (1) != Handle Gen (0)
    EM-->>App: false (Stale handle rejected!)
```

The lifecycle rules for entities are:

- **Destruction**: When an entity is destroyed, its slot's generation counter is incremented modulo 256, its signature bitmask is wiped, and the index is pushed into `_freeIndices`.
- **Delayed Recycling**: A freed slot index is not reused immediately. It remains queued until more than `MIN_FREE_INDICES` (1024) entities have been recycled (or when maximum capacity is reached). This ensures a long buffer period before an index reappears.
- **Liveness Validation**: `isAlive(Entity)` checks that index is in bounds, alive flag is true, and stored generation matches the entity's generation.

---

### 2.2 Components: Contiguous Sparse Sets

Components are pure data structures (e.g., `Position`, `Velocity`, `Health`, `Collider`, `Sprite`).

To achieve O(1) lookups, O(1) additions, O(1) removals, and linear memory traversal for CPU cache locality, our `ComponentArray` template implements a **Sparse Set** pattern:

```mermaid
graph TD
    subgraph SparseArray["Sparse Index Array (Indexed by Entity Index)"]
        S0["Slot 0: NONE"]
        S1["Slot 1 -> Dense 0"]
        S2["Slot 2: NONE"]
        S3["Slot 3 -> Dense 1"]
        S4["Slot 4: NONE"]
        S5["Slot 5 -> Dense 2"]
    end

    subgraph DenseArray["Packed Entity Array"]
        D0["Dense 0: Entity #1"]
        D1["Dense 1: Entity #3"]
        D2["Dense 2: Entity #5"]
    end

    subgraph DataArray["Packed Contiguous Component Buffer (T)"]
        C0["Data 0: Position {x: 10, y: 20}"]
        C1["Data 1: Position {x: 50, y: 80}"]
        C2["Data 2: Position {x: 15, y: 40}"]
    end

    S1 --> D0
    S3 --> D1
    S5 --> D2
    D0 --- C0
    D1 --- C1
    D2 --- C2
```

- **Sparse Vector (`_sparse`):** Indexed directly by `Entity::index()`. Maps an entity's slot to its location in the packed dense arrays, or `NONE` if the component is absent.
- **Dense Vector (`_dense`):** Tightly packed list of active `Entity` handles.
- **Data Vector (`_data`):** Tightly packed contiguous buffer of component data `T`. Iterating over components is a direct contiguous memory loop with zero pointer indirection.
- **Swap-and-Pop Removal:** When an entity is removed, the last component in `_data` and its corresponding entity in `_dense` are swapped into the vacated index, updating the moved entity's sparse pointer in O(1) time without shifting elements.

#### Component Signatures

Every entity has a component bitmask:
```cpp
using Signature = std::bitset<64>; // MAX_COMPONENTS = 64
```
When a component `T` is added or removed, its unique bit (assigned by `ComponentManager::getType<T>()`) is toggled.

---

### 2.3 Systems: Signature Filtering

Systems encapsulate gameplay logic. They do not hold entity data; instead, each system defines a required component signature:
```cpp
System Signature = Bitmask of Required Component Types
```

```mermaid
flowchart TD
    SigChange["Entity Signature Modified<br/>(Component Added or Removed)"]
    Eval{"(EntitySignature & SystemSignature)<br/>== SystemSignature"}
    Add["System::entities.insert(entity)"]
    Remove["System::entities.erase(entity)"]

    SigChange --> Eval
    Eval -- "Match (Has all required components)" --> Add
    Eval -- "Mismatch (Missing at least one)" --> Remove
```

- **Automated Membership:** Whenever `Coordinator::addComponent` or `Coordinator::removeComponent` is called, `SystemManager::entitySignatureChanged` tests the entity's signature against all registered system signatures via a fast bitwise AND:
  ```cpp
  (entitySignature & systemSignature) == systemSignature
  ```
- **Filtered Entity Set:** Matching entities are stored in each system's `std::set<Entity> entities`.
- **System Execution:** During updates, systems iterate strictly over their filtered entity set, querying the `ComponentManager` for the exact data required.

---

## 3. Engine Architecture & Execution Flow

The `Coordinator` acts as the unifying facade that controls the three managers: `EntityManager`, `ComponentManager`, and `SystemManager`.

```mermaid
classDiagram
    class Coordinator {
        -EntityManager _entitiesManager
        -ComponentManager _componentsManager
        -SystemManager _systemsManager
        +createEntity() Entity
        +destroyEntity(Entity e)
        +isAlive(Entity e) bool
        +registerComponent()
        +addComponent(Entity e, T comp)
        +removeComponent(Entity e)
        +getComponent(Entity e) T&
        +hasComponent(Entity e) bool
        +registerSystem() T&
        +setSystemSignature(Signature s)
        +getSystem() T&
    }

    class EntityManager {
        -vector _generations
        -vector _alive
        -vector _signatures
        -queue _freeIndices
        +create() optional
        +destroy(Entity e) bool
        +isAlive(Entity e) bool
        +setSignature(Entity e, Signature s)
        +getSignature(Entity e) Signature
    }

    class ComponentManager {
        -array _arrays
        -unordered_map _types
        +registerComponent()
        +add(Entity e, T comp)
        +remove(Entity e)
        +get(Entity e) T&
        +entityDestroyed(Entity e)
    }

    class SystemManager {
        -unordered_map _signatures
        -unordered_map _systems
        +registerSystem() T&
        +setSignature(Signature s)
        +entityDestroyed(Entity e)
        +entitySignatureChanged(Entity e, Signature s)
    }

    Coordinator *-- EntityManager
    Coordinator *-- ComponentManager
    Coordinator *-- SystemManager
```

### Complete Entity Lifecycle Walkthrough

```mermaid
sequenceDiagram
    autonumber
    participant User as Game World / Gameplay Code
    participant Coord as Coordinator
    participant EM as EntityManager
    participant CM as ComponentManager
    participant SM as SystemManager
    participant Sys as MovementSystem

    Note over User,Sys: 1. Creation & Component Binding
    User->>Coord: createEntity()
    Coord->>EM: create() -> returns Entity(index=5, gen=0)
    Coord-->>User: Entity #5
    
    User->>Coord: addComponent(Entity #5, Position{0, 0})
    Coord->>CM: add(Entity #5, Position)
    Coord->>EM: Update signature bit 0 -> ON
    Coord->>SM: entitySignatureChanged(Entity #5, Sig: 0b001)
    
    User->>Coord: addComponent(Entity #5, Velocity{10, 0})
    Coord->>CM: add(Entity #5, Velocity)
    Coord->>EM: Update signature bit 1 -> ON
    Coord->>SM: entitySignatureChanged(Entity #5, Sig: 0b011)
    SM->>Sys: Signature matches! entities.insert(Entity #5)

    Note over User,Sys: 2. Game Tick Loop
    User->>Sys: update(Coordinator, dt)
    loop For each entity in Sys::entities
        Sys->>Coord: getComponent Position(Entity #5)
        Sys->>Coord: getComponent Velocity(Entity #5)
        Note over Sys: pos.x += vel.x * dt
    end

    Note over User,Sys: 3. Destruction & Memory Reclaiming
    User->>Coord: destroyEntity(Entity #5)
    Coord->>EM: destroy(Entity #5) -> Invalidate gen & queue index
    Coord->>CM: entityDestroyed(Entity #5) -> Swap-and-pop in all arrays
    Coord->>SM: entityDestroyed(Entity #5) -> Erase from all system sets
```
