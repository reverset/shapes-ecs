# Shapes ECS

A custom type-safe entity component system and game framework written in modern C++ and Raylib

## Status

Core ECS functionality is implemented and usable. The project is under development, and the API needs cleanup to avoid the current use of macros and inheritance.

## Features
- Entity creation and destruction
- Type-safe component registration and storage
- Adding, retrieving, and removing components
- Queries over component combinations
- Systems operating on entities
- Sprite Rendering
- Fragment shader loading
- Keybindings and Input handling
- Vector math library

## Example ECS usage
```cpp

struct Position : Component<Position> {
    COMPONENT_STORAGE(Position);
    
    float x;
    float y;
};


// define a system

void updatePosition(const Entity, Position& position) {
    position.x += 1;
}

// register a system

Universe::registerSystem<Position>(updatePosition);

// create an entity

auto& store = Universe::getEntityStorage();
Entity entity = store.makeEntity()
    .addComponent(Position{0.5f, 1.0f})
    .getEntity();

```

## Building

Requires C++ 26 \
**Tested with GCC 16.1.1**

```bash
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```


Once built, you can run the sample game.\
`./build/game`

### Controls
| Keybind | Action                  |
|---------|-------------------------|
| `/`     | Spawn Enemy             |
| `.`     | Spawn Second Enemy Type |
| `,`     | Spawn healing heart     |

## Roadmap (TODO)

- Archetype-based storage
- Benchmarks
- Query caching
- Remove macro for component storage and register lazily.
- Cleanup to remove unnecessary inheritance
- Replace huge static `Universe` class that currently manages state.