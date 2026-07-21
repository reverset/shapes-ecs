# Shapes ECS

A custom entity component system written in modern C++


## Features
- Entity creation and desctruction
- Type-safe component registration and storage
- Adding, retrieving, and removing components
- Queries over component combinations
- Systems operating on entities

## Building

Requires C++ 26

`cmake -S . -B build/ --DCMAKE_BUILD_TYPE=Debug` \
`cmake --build build` \

Once built, you can run the 'sample' game.\
`./build/game`

### Controls
| Keybind | Action                  |
|---------|-------------------------|
| `/`     | Spawn Enemy             |
| `.`     | Spawn 2nd kind of enemy |
| `,`     | Spawn healing heart     |

## Roadmap (TODO)

- Archetype-based storage
- Benchmarks
- Query caching