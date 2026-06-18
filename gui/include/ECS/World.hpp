#ifndef ZAPPY_WORLD_HPP
#define ZAPPY_WORLD_HPP

#include "ComponentMap.hpp"
#include "ECS/Entity.hpp"
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace zappy {
/**
 * @class World
 * @brief Manages entities and their associated components in an Entity-Component-System (ECS)
 * architecture.
 *
 * The World class is responsible for creating and destroying entities, as well as managing the
 * storage of components associated with those entities. It provides methods to register component
 * types, add/remove components to/from entities, and check the validity of entities
 *
 */
class World {
    /**
     * @brief Vector that tracks the generation of each entity ID. The generation is incremented
     * each time an entity is despawned so we can reuse IDs without risking conflicts with old
     * entities
     */
    std::vector<uint32_t> entity_generations;
    /**
     * @brief Vector of free entity IDs that can be reused when spawning new entities. When an
     * entity is despawned, its ID is added to this vector for future reuse
     */
    std::vector<uint32_t> free_ids;
    /**
     * @brief Map that associates component types (identified by std::type_index) with their
     * corresponding storage (IComponentStorage). This allows for dynamic management of different
     * component types
     */
    std::unordered_map<std::type_index, std::shared_ptr<IComponentStorage>> storages;

  public:
    World() = default;

    /**
     *
     * @tparam T class of the component to register
     */
    template <typename T> void register_component() {
        auto type = std::type_index(typeid(T));
        if (storages.find(type) == storages.end()) {
            storages[type] = std::make_shared<ComponentMap<T>>();
        }
    }

    /**
     * Creates a new entity with a unique ID and generation. Reuses IDs from despawned entities when
     * possible
     * @return a new Entity with a unique ID and generation
     */
    Entity spawn() {
        uint32_t id;
        uint32_t generation;

        if (!free_ids.empty()) {
            id = free_ids.back();
            free_ids.pop_back();
            generation = entity_generations[id];
        } else {
            id = static_cast<uint32_t>(entity_generations.size());
            entity_generations.push_back(0);
            generation = 0;
        }
        return {id, generation};
    }

    /**
     *
     * @param entity the entity to despawn. Marks the entity as dead by incrementing its generation
     * and adding its ID to the free list for reuse
     */
    void despawn(const Entity entity) {
        if (!is_alive(entity)) {
            return;
        }

        for (auto& [type, storage] : storages) {
            storage->remove(entity);
        }

        entity_generations[entity.id()]++;
        free_ids.push_back(entity.id());
    }

    /**
     *
     * @param entity to check if an entity still exists or not
     * @return true if alive false if has been despawned or never existed
     */
    bool is_alive(const Entity entity) const {
        if (entity.id() >= entity_generations.size()) {
            return false;
        }
        return entity_generations[entity.id()] == entity.generation();
    }

    /**
     *
     * @tparam T class of the component to get the storage for
     * @return a shared pointer to the ComponentMap<T> if it exists, or nullptr if the component
     * type has not been registered
     */
    template <typename T> std::shared_ptr<ComponentMap<T>> get_storage() {
        auto type = std::type_index(typeid(T));
        auto it = storages.find(type);
        if (it != storages.end()) {
            return std::static_pointer_cast<ComponentMap<T>>(it->second);
        }
        return nullptr;
    }

    /**
     *
     * @tparam T class of the component to add to the entity
     * @param entity the entity to which the component will be added. The entity must be alive (not
     * despawned) for the component to be added successfully
     * @param component the component instance to add to the entity. The component will be moved
     * into the storage, so it should not be used after this call
     */
    template <typename T> void add_component(Entity entity, T component) {
        auto storage = get_storage<T>();
        if (!storage) {
            register_component<T>();
            storage = get_storage<T>();
        }
        storage->insert(entity, std::move(component));
    }

    /**
     *
     * @tparam T class of the component to add to the entity
     * @param entity the entity to which the component will be added. The entity must be alive (not
     * despawned) for the component to be added successfully
     * @param component a shared pointer to the component instance to add to the entity.
     */
    template <typename T> void add_component(Entity entity, std::shared_ptr<T> component) {
        auto storage = get_storage<T>();
        if (!storage) {
            register_component<T>();
            storage = get_storage<T>();
        }
        storage->insert(entity, std::move(component));
    }

    /**
     * Retrieves a shared pointer to the component with the given entity. If the entity does not
     * have the component it returns nullptr.
     * @tparam T class of the component to get from the entity
     * @param entity the entity from which to retrieve the component.
     * @return a shared pointer to the component of type T associated with the entity, or nullptr if
     * the entity does not have that component or if the component type has not been registered. The
     * caller should check if the returned pointer is not null before dereferencing it
     */
    template <typename T> std::shared_ptr<T> get_component(Entity entity) {
        auto storage = get_storage<T>();
        if (storage) {
            return storage->get(entity);
        }
        return nullptr;
    }

    /**
     * Allows to remove a component from an entity. If the entity does not have the component or if
     * the component type has not been registered, this function does nothing
     * @tparam T class of the component to remove from the entity
     * @param entity entity from which to remove the component. If the entity does not have the
     * component or if the
     */
    template <typename T> void remove_component(Entity entity) {
        auto storage = get_storage<T>();
        if (storage) {
            storage->remove(entity);
        }
    }

    /**
     *
     */
    void despawn_all_entities() {
        for (auto& [_, storage] : storages) {
            storage->clear();
        }
        entity_generations.clear();
        free_ids.clear();
    }
};

} // namespace zappy

#endif // ZAPPY_WORLD_HPP
