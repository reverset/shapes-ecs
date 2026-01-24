#ifndef GAME_ECS_H
#define GAME_ECS_H

#include <cinttypes>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "iter.h"
#include "logging.h"

#define COMPONENT_STORAGE(type) static ComponentStorage<type>& getStoreStatically() { static ComponentStorage<type> store; return store; } ComponentStorage<type>* getComponentStorage() override { return &getStoreStatically(); }

struct Entity {
    std::uint32_t id;

    bool operator==(const Entity& e) const {
        return id == e.id;
    }
};

template <>
struct std::hash<Entity> {
    std::size_t operator()(const Entity entity) const noexcept{
        return std::hash<std::uint32_t>{}(entity.id);
    }
};

template <typename T>
struct ComponentStorage {
    std::vector<T> dense;
    std::vector<Entity> entities;
    std::unordered_map<Entity, std::size_t> sparse;

    [[nodiscard]] std::size_t size() const {
#ifndef NDEBUG
        if (dense.size() != entities.size() || entities.size() != sparse.size()) {
            Logging::logWarn("ComponentStorage sizes are out of sync! dense=%d, entities=%d, sparse=%d", dense.size(), entities.size(), sparse.size());
        }
#endif
        return dense.size();
    }

    bool contains(const Entity e) const {
        return sparse.contains(e);
    }

    T& get(const Entity e) {
        if (const auto loc = sparse.find(e); loc != sparse.end()) {
            return dense[loc->second];
        }

        throw std::out_of_range("Entity not found!");
    }

    void add(const Entity e, T&& comp) {
        sparse[e] = dense.size();
        dense.push_back(std::move(comp));
        entities.push_back(e);
    }

    void remove(const Entity e) {
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

template <typename T>
class Component {
public:
    [[nodiscard]] virtual ComponentStorage<T>* getComponentStorage() = 0;

    virtual ~Component() = default;
};

inline std::uint32_t reserveEntityId() {
    static std::uint32_t id = 0;
    constexpr auto max = std::numeric_limits<std::uint32_t>::max();
    constexpr auto threshold = max - 1000;

    if (id > threshold) {
        Logging::logWarn("Running low on entity IDS. Current id=%zu, max=%zu", id, threshold);
    } else if (id == max) {
        throw std::logic_error("RAN OUT OF ENTITY IDS!!!");
    }
    return id++;
}

class EntityBuilder {
    Entity e = { .id = reserveEntityId()};
public:
    template <typename T>
        requires std::derived_from<T, Component<T>>
    EntityBuilder& addComponent(T&& comp) {
        comp.getComponentStorage()->add(e, std::move(comp));
        return *this;
    }

    [[nodiscard]] Entity getEntity() const {
        return e;
    }
};

namespace ECS {
    template <typename First, typename... Rest, typename Func>
    std::function<void()> createCallableSystem(Func&& f) {
        return [&] {
            query<First, Rest..., Func>(f);
        };
    }

    template <typename First, typename... Rest, typename Func>
    void query(Func&& f) { // absolute magic
        ComponentStorage<First>& firstStore = First::getStoreStatically();

        for (std::size_t i = 0; i < firstStore.size(); ++i) {
            if (const auto entity = firstStore.entities[i]; (Rest::getStoreStatically().contains(entity) && ...)) {
                f(
                    entity,
                    firstStore.dense[i],
                    Rest::getStoreStatically().get(entity)...
                );
            }
        }
    }
}

// todo, entity manager (entity -> get all components, remove entities, etc)

class Schedule {
    using Callback = std::function<void()>;

    std::vector<Callback> callbacks;
public:

    template <typename First, typename... Rest, typename Func>
    Schedule& registerSystem(Func&& f) {
        const auto sys = ECS::createCallableSystem<First, Rest..., Func>(f);
        callbacks.push_back(sys);

        return *this;
    }

    void tick() const {
        for (auto& c : callbacks) {
            c();
        }
    }
};


#endif //GAME_ECS_H