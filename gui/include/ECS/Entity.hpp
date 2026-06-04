#ifndef ZAPPY_ENTITY_HPP
#define ZAPPY_ENTITY_HPP

#include <cstdint>
#include <functional>

namespace zappy {
/**
 * @class Entity
 * @brief Represents an entity in an Entity-Component-System (ECS) architecture. An entity is a
 * unique identifier that can have various components associated with it
 */
class Entity {
    /**
     * @brief Unique identifier for the entity. This ID is used to associate components with the
     * entity
     */
    uint32_t _id;
    /**
     * @brief Generation number for the entity. This is used to track the validity of an entity ID.
     * Each time an entity is despawned, its generation is incremented to prevent conflicts with new
     * entities that may reuse the same ID
     */
    uint32_t _generation;

  public:
    /**
     * NEVER CREATE AND ENTITY DIRECTLY, USE World::spawn() INSTEAD.
     * @param id unique identifier for the entity
     * @param generation generation number for the entity, used to track validity. Should be set to
     * 0 for new entities and incremented each time an entity is despawned
     */
    Entity(const uint32_t id, const uint32_t generation) : _id(id), _generation(generation) {}
    ~Entity() = default;

    /**
     *
     * @param other the entity to compare with. Two entities are considered equal if they have the
     * same ID and generation
     * @return true if the entities are equal (same ID and generation), false otherwise
     */
    bool operator==(const Entity& other) const {
        return _id == other._id && _generation == other._generation;
    }

    bool operator!=(const Entity& other) const { return !(*this == other); }

    bool operator<(const Entity& other) const {
        if (_id != other._id) {
            return _id < other._id;
        }
        return _generation < other._generation;
    }

    /**
     * id getter
     * @return id value of the entity
     */
    uint32_t id() const { return _id; }
    /**
     * generation getter
     * @return generation value of the entity
     */
    uint32_t generation() const { return _generation; }
};
} // namespace zappy

namespace std {
template <> struct hash<zappy::Entity> {
    size_t operator()(const zappy::Entity& e) const {
        size_t h1 = hash<uint32_t>{}(e.id());
        size_t h2 = hash<uint32_t>{}(e.generation());
        // magic number to join hashes
        // https://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
} // namespace std

#endif // ZAPPY_ENTITY_HPP
