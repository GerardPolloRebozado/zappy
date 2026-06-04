//! # ECS Storage Module
//!
//! This module provides a simple HashMap-based ECS (Entity Component System) storage
//!
//! ## Example
//!
//! ```rust
//! use zappy_server::ecs::storage::World;
//!
//! // Define your components
//! struct Position { x: f32, y: f32 }
//! struct Velocity { dx: f32, dy: f32 }
//!
//! let mut world = World::default();
//!
//!
//! // Spawn an entity
//! let player = world.spawn();
//!
//! // Add components
//! world.add_component(player, Position { x: 0.0, y: 0.0 });
//! world.add_component(player, Velocity { dx: 1.0, dy: 1.0 });
//!
//! // Access components
//! if let Some(pos) = world.get_component_mut::<Position>(player) {
//!     pos.x += 10.0;
//! }
//!
//! // Query entities with both Position AND Velocity:
//! if let (Some(pos_storage), Some(vel_storage)) = (world.get_storage::<Position>(), world.get_storage::<Velocity>()) {
//!     for (entity, pos) in pos_storage.iter() {
//!         if let Some(vel) = vel_storage.get(*entity) {
//!             println!("Entity {:?} is at ({}, {}) moving by ({}, {})", entity, pos.x, pos.y, vel.dx, vel.dy);
//!         }
//!     }
//! }
//!
//! // despawn when done
//! world.despawn(player);
//! ```

use crate::ecs::map_size::MapSize;
use std::any::{Any, TypeId};
use std::collections::HashMap;

/// An `Entity` is a unique identifier used to associate components
/// It consists of an `id` and a `generation` to allow for ID reuse
#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
pub struct Entity {
    /// Unique incremental identifier
    id: u32,
    /// Counter for how many times this ID has been recycled
    generation: u32,
}

impl Entity {
    pub fn id(&self) -> u32 {
        self.id
    }
    pub fn generation(&self) -> u32 {
        self.generation
    }
}

/// trait for component storage
pub trait ComponentStorage: Any {
    /// Removes an entity's component from the storage
    fn remove(&mut self, entity: Entity);
    /// Helper to downcast the storage back to a concrete type
    fn as_any(&self) -> &dyn Any;
    /// Helper to downcast the storage back to a concrete type (mutable)
    fn as_any_mut(&mut self) -> &mut dyn Any;
}

/// a simple storage for a component using a HashMap
pub struct ComponentMap<T> {
    /// Maps an Entity to its component data
    entries: HashMap<Entity, T>,
}

impl<T: 'static> Default for ComponentMap<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T: 'static> ComponentMap<T> {
    pub fn new() -> Self {
        Self {
            entries: HashMap::new(),
        }
    }

    /// Inserts a component for an entity
    pub fn insert(&mut self, entity: Entity, component: T) {
        self.entries.insert(entity, component);
    }

    /// Removes a component for an entity
    pub fn remove(&mut self, entity: Entity) -> Option<T> {
        self.entries.remove(&entity)
    }

    /// Returns a reference to the component for an entity
    pub fn get(&self, entity: Entity) -> Option<&T> {
        self.entries.get(&entity)
    }

    /// Returns a mutable reference to the component for an entity
    pub fn get_mut(&mut self, entity: Entity) -> Option<&mut T> {
        self.entries.get_mut(&entity)
    }

    /// Returns an iterator over all components
    pub fn values(&self) -> std::collections::hash_map::Values<'_, Entity, T> {
        self.entries.values()
    }

    /// Returns an iterator over all entities and their components
    pub fn iter(&self) -> std::collections::hash_map::Iter<'_, Entity, T> {
        self.entries.iter()
    }

    /// Returns a mutable iterator over all entities and their components
    pub fn iter_mut(&mut self) -> std::collections::hash_map::IterMut<'_, Entity, T> {
        self.entries.iter_mut()
    }
}

impl<T: 'static> ComponentStorage for ComponentMap<T> {
    fn remove(&mut self, entity: Entity) {
        self.entries.remove(&entity);
    }
    fn as_any(&self) -> &dyn Any {
        self
    }
    fn as_any_mut(&mut self) -> &mut dyn Any {
        self
    }
}

/// The `World` is the main container for all ECS data
pub struct World {
    pub map_size: MapSize,
    /// List of generations for all entities, indexed by their ID
    entity_generations: Vec<u32>,
    /// List of Entity IDs that have been despawned and are available for reuse
    free_ids: Vec<u32>,
    /// A map of component types to their respective `ComponentMap`
    storages: HashMap<TypeId, Box<dyn ComponentStorage>>,
    /// curent frequency used to calculate time for instance, if f=1, ”forward” takes 7 / 1 = 7 seconds.
    pub freq: u64,
}

impl Default for World {
    fn default() -> Self {
        Self::new(100)
    }
}

impl World {
    /// Creates a new, empty ECS world
    pub fn new(freq: u64) -> Self {
        Self {
            map_size: MapSize {
                width: 100,
                height: 100,
            },
            entity_generations: Vec::new(),
            free_ids: Vec::new(),
            storages: HashMap::new(),
            freq,
        }
    }

    /// Registers a component type with the world, this is only used internally
    fn register_component<T: 'static>(&mut self) {
        self.storages
            .entry(TypeId::of::<T>())
            .or_insert_with(|| Box::new(ComponentMap::<T>::new()));
    }

    /// Spawns a new entity.
    pub fn spawn(&mut self) -> Entity {
        let (id, generation) = if let Some(free_id) = self.free_ids.pop() {
            (free_id, self.entity_generations[free_id as usize])
        } else {
            let id = self.entity_generations.len() as u32;
            self.entity_generations.push(0);
            (id, 0)
        };
        Entity { id, generation }
    }

    /// Adds a component to an entity
    pub fn add_component<T: 'static>(&mut self, entity: Entity, component: T) {
        let mut storage = self.storages.get_mut(&TypeId::of::<T>());
        if storage.is_none() {
            self.register_component::<T>();
            storage = self.storages.get_mut(&TypeId::of::<T>());
        }
        let storage = storage
            .and_then(|s| s.as_any_mut().downcast_mut::<ComponentMap<T>>())
            .unwrap();
        storage.insert(entity, component);
    }

    /// Gets a component from an entity
    pub fn get_component<T: 'static>(&self, entity: Entity) -> Option<&T> {
        self.get_storage::<T>()?.get(entity)
    }

    /// Gets a mutable component from an entity
    pub fn get_component_mut<T: 'static>(&mut self, entity: Entity) -> Option<&mut T> {
        self.get_storage_mut::<T>()?.get_mut(entity)
    }

    /// Returns a reference to the storage for a component type
    pub fn get_storage<T: 'static>(&self) -> Option<&ComponentMap<T>> {
        self.storages
            .get(&TypeId::of::<T>())
            .and_then(|s| s.as_any().downcast_ref::<ComponentMap<T>>())
    }

    /// Returns a mutable reference to the storage for a component type
    pub fn get_storage_mut<T: 'static>(&mut self) -> Option<&mut ComponentMap<T>> {
        self.storages
            .get_mut(&TypeId::of::<T>())
            .and_then(|s| s.as_any_mut().downcast_mut::<ComponentMap<T>>())
    }

    /// Removes a component from an entity.
    pub fn remove_component<T: 'static>(&mut self, entity: Entity) -> Option<T> {
        self.storages
            .get_mut(&TypeId::of::<T>())
            .and_then(|s| s.as_any_mut().downcast_mut::<ComponentMap<T>>())?
            .remove(entity)
    }

    /// Despawns an entity
    pub fn despawn(&mut self, entity: Entity) {
        if !self.is_alive(entity) {
            return;
        }
        for storage in self.storages.values_mut() {
            storage.remove(entity);
        }
        self.entity_generations[entity.id as usize] += 1;
        self.free_ids.push(entity.id);
    }

    /// Checks if an entity is alive
    pub fn is_alive(&self, entity: Entity) -> bool {
        self.entity_generations.get(entity.id as usize) == Some(&entity.generation)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ecs_lifecycle() {
        let mut world = World::default();
        world.register_component::<i32>();

        let ent = world.spawn();
        world.add_component(ent, 42);

        assert_eq!(*world.get_component::<i32>(ent).unwrap(), 42);

        world.despawn(ent);
        assert!(world.get_component::<i32>(ent).is_none());
        assert!(!world.is_alive(ent));
    }
}
