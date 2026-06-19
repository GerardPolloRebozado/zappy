#ifndef ZAPPY_ICOMPONENTSTORAGE_HPP
#define ZAPPY_ICOMPONENTSTORAGE_HPP

#include "Entity.hpp"

namespace zappy {
class IComponentStorage {
  public:
    virtual ~IComponentStorage() = default;
    virtual void remove(Entity entity) = 0;
    virtual void clear() = 0;
};
} // namespace zappy

#endif // ZAPPY_ICOMPONENTSTORAGE_HPP
