#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <functional>
#include <vector>

#define EVENT_STORAGE(type) static EventStorage<type>& getEventStoreStatically() { static EventStorage<type> eventStorage; return eventStorage; }

template <typename T>
class EventStorage {
    // todo, event canceling? Priority?
    using Listener = std::function<void(T&)>;
    std::vector<Listener> listeners{};

public:
    void propagate(T& event) {
        for (auto& listener : listeners) listener(event);
    }

    void listen(const Listener listener) {
        listeners.push_back(listener);
    }
};

template <typename T>
class Event {
public:
    EventStorage<T>& getEventStorage() const {
        return T::getEventStoreStatically();
    }

    void send() {
        getEventStorage().propagate(*static_cast<T*>(this));
    }

    static void listen(const std::function<void(T&)>& listener) {
        T::getEventStoreStatically().listen(listener);
    }
};


#endif

