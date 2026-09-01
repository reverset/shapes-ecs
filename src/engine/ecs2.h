#ifndef GAME_ECS2_H
#define GAME_ECS2_H

#include <unordered_map>
#include <vector>

#include "logging.h"

namespace ECS2 {
    struct Entity {
        std::uint32_t id;

        bool operator==(const Entity& e) const {
            return id == e.id;
        }
    };
}

template <>
struct std::hash<ECS2::Entity> {
    std::size_t operator()(const ECS2::Entity entity) const noexcept {
        return std::hash<std::uint32_t>{}(entity.id);
    }
};

namespace ECS2 {
    using TypeId = std::size_t;

    inline TypeId nextTypeId() {
        static TypeId counter = 0;
        return counter++;
    }

    template<typename T>
    TypeId typeId() {
        static const TypeId id = nextTypeId();
        return id;
    }

    template<typename T>
    const char* typeName() {
        return typeid(T).name();
    }

    template <typename T>
    struct ComponentStore {
        std::vector<T> dense;
        std::unordered_map<Entity, std::size_t> sparse;
        std::vector<Entity> entities;

        static Logging::Logger& getLogger() {
            static auto logger = NEW_LOGGER(ComponentStore);
            return logger;
        }

        [[nodiscard]] std::size_t size() const {
#ifndef NDEBUG
            if (dense.size() != entities.size() || entities.size() != sparse.size()) {
                Logging::logWarn("ComponentStorage sizes are out of sync! dense=%d, entities=%d, sparse=%d", dense.size(), entities.size(), sparse.size());
            }
#endif
            return dense.size();
        }

        [[nodiscard]] bool contains(const Entity e) const {
            return sparse.contains(e);
        }

        void add(const Entity e, T&& comp) {
            if (sparse.contains(e)) {
                Logging::logWarn("entity (id=%zu) already has this component", e.id);
                return;
            }

            sparse[e] = dense.size();
            dense.push_back(std::move(comp));
            entities.push_back(e);
        }

        template <typename... Args>
        T& emplace(const Entity e, Args&&... args) {
            if (sparse.contains(e)) {
                getLogger().logWarn("entity (id=%zu) already has this component", e.id);
                return dense[sparse.at(e)];
            }

            sparse[e] = dense.size();
            T& ref = dense.emplace_back(std::forward<Args>(args)...);
            entities.push_back(e);
            return ref;
        }

        // call contains before this
        [[nodiscard]] T& get(const Entity e) {
            if (const auto loc = sparse.find(e); loc != sparse.end()) {
                return dense[loc->second];
            }

            getLogger().logError("called get(Entity=%zu). No such entity.", e.id);
            throw std::out_of_range("Entity not found!");
        }

        void remove(const Entity e) {
            if (!sparse.contains(e)) {
                getLogger().logWarn("attempt to remove entity from component storage, when it never was present. id=%zu", e.id);
                return;
            }

            auto i = sparse[e];
            auto last = dense.size() - 1;

            dense[i] = std::move(dense[last]);
            entities[i] = entities[last];

            sparse[entities[i]] = i;

            dense.pop_back();
            entities.pop_back();
            sparse.erase(e);
        }
    };

    template<typename... T>
    struct Archetype {
        std::vector<Entity> entities;
        std::vector<std::tuple<T...>> dense;
        std::unordered_map<Entity, std::size_t> sparse;

        void add(const Entity e, T&&... comps) {
            if (contains(e)) {
                Logging::logWarn("entity (id=%zu) already has this component '%s'", e.id, typeid(this).name());
                return;
            }
            sparse[e] = dense.size();
            dense.push_back(std::make_tuple(std::move(comps)...));
            entities.push_back(e);
        }

        [[nodiscard]] bool contains(const Entity e) {
            return sparse.contains(e);
        }

        [[nodiscard]] std::size_t size() const {
            return dense.size();
        }
    };

    struct ErasedStore {
        void* store = nullptr;
        void (*destroy)(void*) = nullptr;

        template <typename T>
        static ErasedStore make() {
            return {
                new ComponentStore<T>,
                [](void* p) { delete static_cast<ComponentStore<T>*>(p); },
            };
        }
    };

    struct ErasedArchetype {
        void* store = nullptr;
        void (*destroy)(void*) = nullptr;

        template <typename... T>
        static ErasedArchetype make() {
            return {
                new Archetype<T...>,
                [](void* p) { delete static_cast<Archetype<T...>*>(p); },
            };
        }
    };

    class World {
        std::vector<ErasedStore> stores;
        std::vector<ErasedArchetype> archetypes;
    public:

        World() = default;
        World(const World&) = delete;
        World& operator=(const World&) = delete;

        ~World() {
            // dropStores();
            // dropArchetypes();
        }

        template<typename T>
        ComponentStore<T>& getStore() {
            const TypeId id = typeId<T>();
            if (id >= stores.size()) stores.resize(id + 1);
            auto& slot = stores[id];
            if (!slot.store) slot = ErasedStore::make<T>();
            return *static_cast<ComponentStore<T>*>(slot.store);
        }

        template<typename... T>
        Archetype<T...>& getArchetype() {
            const TypeId id = typeId<Archetype<T...>>(); // type order changes ID FIXME
            if (id >= archetypes.size()) archetypes.resize(id + 1);
            auto& slot = archetypes[id];
            if (!slot.store) slot = ErasedArchetype::make<T...>();
            return *static_cast<Archetype<T...>*>(slot.store);
        }

        void dropStores() {
            for (auto& store : stores) {
                store.destroy(&store);
            }
            stores.clear();
        }

        void dropArchetypes() {
            for (auto& store : archetypes) {
                store.destroy(&store);
            }
            archetypes.clear();
        }

        template<typename First, typename... Rest, typename Func>
        void query(Func&& func) {
            ComponentStore<First> firstStore = getStore<First>();
            for (std::size_t i = 0; i < firstStore.size(); ++i) { // TODO archetypes
                const Entity e = firstStore.entities[i];
                if ((getStore<Rest>().contains(e) && ...)) {
                    func(
                        e,
                        firstStore.dense[i],
                        getStore<Rest>().get(e)...
                    );
                }
            }
        }
    };

    template<typename... T>
    struct QueryIterator {
        World* world;
        std::size_t ptr;

        [[nodiscard]] const std::tuple<T...>& operator*() {
            const Archetype<T...>& archetype = world->getArchetype<T...>();
            const Entity e = archetype.entities[ptr];
            return archetype.dense[archetype.sparse.at(e)];
        }

        [[nodiscard]] const std::tuple<T...>& operator++() {
            const auto& current = this->operator*();
            ptr += 1;
            return current;
        }

        [[nodiscard]] bool operator==(const QueryIterator& other) const {
            return world == other.world && ptr == other.ptr;
        }

        [[nodiscard]] bool operator!=(const QueryIterator& other) const {
            return world != other.world || ptr != other.ptr;
        }
    };

    template<typename... T>
    struct Query {
        World* world;

        [[nodiscard]] QueryIterator<T...> begin() const {
            return QueryIterator<T...>{ .world = world, .ptr = 0 };
        }

        [[nodiscard]] QueryIterator<T...> end() const {
            const Archetype<T...>& archetype = world->getArchetype<T...>();
            return QueryIterator<T...>{ .world = world, .ptr = archetype.size() };
        }
    };


}

#endif //GAME_ECS2_H
