#ifndef GAME_ECS_H
#define GAME_ECS_H

#include <cinttypes>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

#include "logging.h"


#define COMPONENT_STORAGE(type) static ComponentStorage<type>& getStoreStatically() { static ComponentStorage<type> store; return store; } ComponentStorage<type>* getComponentStorage() override { return &getStoreStatically(); }

#define MARKER_COMPONENT(name) struct name : Component<name> { COMPONENT_STORAGE(name); }

class EntityStorage;

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

    static Logging::Logger& getLogger() {
        static auto logger = NEW_LOGGER(ComponentStorage);
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

    // call contains before this
    [[nodiscard]] T& get(const Entity e) {
        if (const auto loc = sparse.find(e); loc != sparse.end()) {
            return dense[loc->second];
        }

        getLogger().logError("called get(Entity=%zu). No such entity.", e.id);
        throw std::out_of_range("Entity not found!");
    }

    void add(const Entity e, T&& comp) {
        sparse[e] = dense.size();
        dense.push_back(std::move(comp));
        entities.push_back(e);
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

template <typename T>
class Component {
public:
    [[nodiscard]] virtual ComponentStorage<T>* getComponentStorage() = 0;

    virtual ~Component() = default;
};

class DynamicComponent {
    void* store;

    std::function<void(Entity)> removeImpl;
    std::function<bool(Entity)> containsImpl;
public:
    template <typename T>
    bool compareStorePtr(T* other) {
        return store == other;
    }

    [[nodiscard]] bool contains(const Entity e) const {
        return containsImpl(e);
    }

    void remove(const Entity e) {
        removeImpl(e);
    }

    template <typename T>
    static DynamicComponent of(Component<T>& comp) {
        const auto store = comp.getComponentStorage();

        return DynamicComponent(static_cast<void *>(store), [store](Entity e) { return store->contains(e); }, [store](Entity e) { return store->remove(e); });
    }

    explicit DynamicComponent(void* store, const std::function<bool(Entity)>& containsImpl, const std::function<void(Entity)>& removeImpl) {
        this->store = store;
        this->removeImpl = removeImpl;
        this->containsImpl = containsImpl;
    }
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

namespace ECS_Internal {
    std::vector<DynamicComponent>* getComponentsFromStorage(EntityStorage* store, Entity e);
}

class EntityBuilder {
    Entity e = { .id = reserveEntityId()};
    EntityStorage* storage;

public:
    template <typename T>
        requires std::derived_from<T, Component<T>>
    EntityBuilder& addComponent(T&& comp) {
        auto dyn = ECS_Internal::getComponentsFromStorage(storage, e);
        dyn->push_back(DynamicComponent::of(comp));

        comp.getComponentStorage()->add(e, std::move(comp));
        return *this;
    }

    [[nodiscard]] Entity getEntity() const {
        return e;
    }

    friend EntityStorage;
private:
    explicit EntityBuilder(EntityStorage* storage) {
        this->storage = storage;
    }
};

namespace ECS {
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

    template <typename... Rest, typename Func>
    void queryComponentsFor(Entity e, Func&& f) {
        if ((Rest::getStoreStatically().contains(e) && ...)) {
            f(
                e,
                Rest::getStoreStatically().get(e)...
            );
        }

    }

    template <typename First, typename... Rest, typename Func>
    [[nodiscard]] std::function<void()> createCallableSystem(Func&& f) {
        return [&] {
            ECS::query<First, Rest...>(f);
        };
    }

    template <typename ... Funcs>
    [[nodiscard]] std::function<void()> chain(Funcs&&... f) {
        return [f...] {
              (f(), ...);
        };
    }

    template <typename First, typename ... Rest>
    [[nodiscard]] std::optional<std::tuple<First*, Rest*...>> findOneOf() {
        std::optional<std::tuple<First*, Rest*...>> res;

        // duplicated code... but its ok
        ComponentStorage<First>& firstStore = First::getStoreStatically();

        for (std::size_t i = 0; i < firstStore.size(); ++i) {
            if (const auto entity = firstStore.entities[i]; (Rest::getStoreStatically().contains(entity) && ...)) {
                res = std::make_tuple(&firstStore.dense[i], &Rest::getStoreStatically().get(entity)...);

                break; // break since we found a desirable entity, and we only needed one.
            }
        }

        return res;
    }

    template <typename First, typename... Rest> // perhaps make a pointer version?
    [[nodiscard]] std::vector<std::tuple<First, Rest...>> collect() {
        std::vector<std::tuple<First, Rest...>> res;

        ECS::query<First, Rest...>([&](const Entity, First& f, Rest&... r) {
            res.push_back(std::make_tuple(f, r...));
        });

        return res;
    }

    template <typename... Rest>
    [[nodiscard]] bool hasComponents(const Entity e) {
        return (Rest::getStoreStatically().contains(e) && ...);
    }
}

class Schedule {
    using Callback = std::function<void()>;

    std::vector<Callback> callbacks;
public:

    template <typename First, typename... Rest, typename Func>
    Schedule& registerSystem(Func&& f) {
        const auto sys = ECS::createCallableSystem<First, Rest...>(f);
        callbacks.push_back(sys);

        return *this;
    }

    Schedule& registerCallable(const Callback& f) {
        callbacks.push_back(f);
        return *this;
    }

    void tick() const {
        for (auto& c : callbacks) {
            c();
        }
    }
};

// DUPLICATE CODE!@!!!! its ok tho
class IrregularSchedule {
    using Callback = std::function<void()>;

    std::vector<Callback> callbacks;

    std::size_t maxStepsPerTick = 20;
    std::size_t progress = 0;

    double tickDelay = 0;
    double lastTick = 0;
public:

    explicit IrregularSchedule() = default;

    explicit IrregularSchedule(const std::size_t maxStepsPerTick, const double tickDelay) {
        this->maxStepsPerTick = maxStepsPerTick;
        this->tickDelay = tickDelay;
    }

    template <typename First, typename... Rest, typename Func>
    IrregularSchedule& registerSystem(Func&& f) {
        const auto sys = ECS::createCallableSystem<First, Rest...>(f);
        callbacks.push_back(sys);

        return *this;
    }

    IrregularSchedule& registerCallable(const Callback& f) {
        callbacks.push_back(f);
        return *this;
    }

    void tick(const double time) {
        if (lastTick + tickDelay > time) return;
        lastTick = time;

        std::size_t i = 0;
        for (; i < maxStepsPerTick; ++i) {
            if (i >= callbacks.size()) {
                progress = 0;
                return;
            }

            callbacks[progress + i]();
        }

        progress += i;
    }
};

class EntityStorage {
    std::unordered_map<Entity, std::vector<DynamicComponent>> entities;
public:
    EntityStorage(const EntityStorage& other) = delete;
    EntityStorage() = default;

    EntityBuilder makeEntity() {
        const auto builder = EntityBuilder(this);
        const auto entity = builder.getEntity();

        entities[entity] = {};
        return builder;
    }

    [[nodiscard]] bool isValid(const Entity e) const {
        return entities.contains(e);
    }

    [[nodiscard]] std::size_t size() const {
        return entities.size();
    }

    template <typename T>
    void insertComponent(Entity e, T comp) { // code duplication, but its ok
        auto dyn = getComponents(e);
        dyn->push_back(DynamicComponent::of(comp));

        comp.getComponentStorage()->add(e, std::move(comp));
    }

    [[nodiscard]] std::vector<DynamicComponent>* getComponents(const Entity e) {
        if (!entities.contains(e)) {
            Logging::logWarn("attempt to get components of non-existent entity. id=%zu", e.id);
            return nullptr;
        }

        return &entities.at(e);
    }

    // WARNING: NEVER access data from an entities components after this function is called.
    void destroyEntity(const Entity e) {
        if (!entities.contains(e)) {
            Logging::logWarn("attempted to destroy non-existent entity! id=%zu", e.id);
            return;
        }

        for (auto& c : *getComponents(e)) {
            c.remove(e);
        }

        entities.erase(e);
    }

    template <typename T>
    void removeComponent(const Entity e) {
        ComponentStorage<T>& store = T::getStoreStatically();
        store.remove(e);

        const auto comps = getComponents(e);
        for (auto i = comps->begin(); i != comps->end(); ++i) {
            if (i->compareStorePtr(&store)) {
                comps->erase(i);
                return;
            }
        }
        Logging::logWarn("dynamic component was not removed from entity storage. id=%zu", e.id);
    }
};

inline std::vector<DynamicComponent>* ECS_Internal::getComponentsFromStorage(EntityStorage* store, const Entity e) {
    return store->getComponents(e);
}

#endif //GAME_ECS_H