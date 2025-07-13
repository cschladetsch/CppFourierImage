#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <any>

// Base event class
struct Event {
    virtual ~Event() = default;
};

// Frequency change event
struct FrequencyChangeEvent : Event {
    size_t newFrequencyCount;
    size_t maxFrequencies;
    
    FrequencyChangeEvent(size_t count, size_t max) 
        : newFrequencyCount(count), maxFrequencies(max) {}
};

// Image loaded event
struct ImageLoadedEvent : Event {
    size_t width;
    size_t height;
    
    ImageLoadedEvent(size_t w, size_t h) : width(w), height(h) {}
};

// Event dispatcher
class EventDispatcher {
public:
    using EventHandler = std::function<void(const Event&)>;
    using HandlerId = size_t;
    
    template<typename T>
    HandlerId subscribe(std::function<void(const T&)> handler) {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");
        
        auto typeIndex = std::type_index(typeid(T));
        HandlerId id = nextHandlerId_++;
        
        handlers_[typeIndex].emplace_back(id, [handler](const Event& e) {
            handler(static_cast<const T&>(e));
        });
        
        return id;
    }
    
    template<typename T>
    void unsubscribe(HandlerId id) {
        auto typeIndex = std::type_index(typeid(T));
        auto it = handlers_.find(typeIndex);
        if (it != handlers_.end()) {
            auto& handlersVec = it->second;
            handlersVec.erase(
                std::remove_if(handlersVec.begin(), handlersVec.end(),
                    [id](const auto& pair) { return pair.first == id; }),
                handlersVec.end()
            );
        }
    }
    
    template<typename T>
    void dispatch(const T& event) {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");
        
        auto typeIndex = std::type_index(typeid(T));
        auto it = handlers_.find(typeIndex);
        if (it != handlers_.end()) {
            for (const auto& [id, handler] : it->second) {
                handler(event);
            }
        }
    }
    
    // Global instance
    static EventDispatcher& getInstance() {
        static EventDispatcher instance;
        return instance;
    }
    
private:
    EventDispatcher() = default;
    
    std::unordered_map<std::type_index, std::vector<std::pair<HandlerId, EventHandler>>> handlers_;
    HandlerId nextHandlerId_ = 1;
};