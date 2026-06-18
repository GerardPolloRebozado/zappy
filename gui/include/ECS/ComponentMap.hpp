#ifndef ZAPPY_COMPONENTMAP_HPP
#define ZAPPY_COMPONENTMAP_HPP

#include "IComponentStorage.hpp"

#include <iostream>
#include <memory>
#include <unordered_map>

namespace zappy {
template <typename T>
/**
 * @class ComponentMap
 * @brief Implements the IComponentStorage interface using an unordered map to store components
 * associated with entities.
 * The ComponentMap class provides methods to insert, remove, and retrieve
 * components for entities. It uses std::shared_ptr to manage components and allows for storing and
 * getting components based on entity IDs.
 */
class ComponentMap : public IComponentStorage {
    std::unordered_map<Entity, std::shared_ptr<T>> entries;

  public:
    /**
     * Allows adding ataching a component to an entity
     * @param entity the entity to add a component to.
     * @param component the component instance to add to the entity
     */
    void insert(Entity entity, T component) {
        entries[entity] = std::make_shared<T>(std::move(component));
    }

    /**
     * Allows adding ataching a component to an entity
     * @param entity the entity to add a component to.
     * @param component the component instance to add to the entity
     */
    void insert(Entity entity, std::shared_ptr<T> component) {
        entries[entity] = std::move(component);
    }

    /**
     * Removes a component from
     * @param entity entity to remove
     */
    void remove(Entity entity) override { entries.erase(entity); }

    /**
     * Gets a component pointer from the map
     * @param entity entity from which to retrieve the component. If the entity does not have the
     * component or if the
     * @return a shared pointer to the component
     */
    std::shared_ptr<T> get(Entity entity) {
        auto it = entries.find(entity);
        if (it != entries.end()) {
            return it->second;
        }
        return nullptr;
    }

    /**
     * Gets a component pointer from the map
     * @param entity entity from which to retrieve the component. If the entity does not have the
     * component or if the
     * @return a shared pointer to the component
     */
    std::shared_ptr<const T> get(Entity entity) const {
        auto it = entries.find(entity);
        if (it != entries.end()) {
            return it->second;
        }
        return nullptr;
    }

    auto begin() { return entries.begin(); }
    auto end() { return entries.end(); }
    auto begin() const { return entries.begin(); }
    auto end() const { return entries.end(); }
    void clear() override { entries.clear(); }

    size_t size() const { return entries.size(); }
};
} // namespace zappy

#endif // ZAPPY_COMPONENTMAP_HPP
