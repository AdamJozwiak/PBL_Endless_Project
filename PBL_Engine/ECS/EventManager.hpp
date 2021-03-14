#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <any>
#include <functional>
#include <list>
#include <mutex>
#include <typeindex>
#include <unordered_map>

#include "EngineAPI.hpp"
#include "Event.hpp"
#include "Utilities.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API EventManager {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    static EventManager& instance();

    EventManager(EventManager const&) = delete;
    EventManager(EventManager&&) = delete;
    EventManager& operator=(EventManager const&) = delete;
    EventManager& operator=(EventManager&&) = delete;

    // --------------------------------------------- Main functionality -- == //
    template <typename EventType>
    void listen(std::function<void(EventType const&)> const& listener) {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        listeners[std::type_index(typeid(EventType))].push_back(listener);
    }

    template <typename EventType>
    void send(EventType const& event) {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        if (listeners.empty() ||
            !listeners.contains(std::type_index(typeid(EventType)))) {
            return;
        }

        for (auto const& listener :
             listeners.at(std::type_index(typeid(EventType)))) {
            std::any_cast<std::function<void(EventType const&)>>(listener)(
                event);
        }
    }

  private:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    EventManager() = default;
    ~EventManager() = default;

    // ============================================================== Data == //
    std::recursive_mutex mutex;

    std::unordered_map<std::type_index, std::list<std::any>> listeners;
};

// /////////////////////////////////////////////////////////////////// Macros //
#define MethodListener(listener) \
    std::bind(&listener, this, std::placeholders::_1)

#define FunctionListener(listener) std::bind(&listener, std::placeholders::_1)

// ////////////////////////////////////////////////////////////////////////// //
