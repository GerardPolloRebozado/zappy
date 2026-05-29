//! # ECS Storage Module

use std::any::{Any, TypeId};
use std::collections::{HashMap, HashSet};

/// an `Entity` is a unique identifier
#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
pub struct Entity {
    /// unique incremental identifier
    id: u32,
    /// counter for how many times this ID has been recycled
    generation: u32,
}

struct ComponentColumn {
    /// pointer to vec
    data: Box<dyn Any>,
    /// remove element from vector while keeping objects in the same memory
    swap_remove_fn: fn(&mut Box<dyn Any>, usize),
    /// function to move a component from this column to a target archetype.
    move_to_fn: fn(&mut Box<dyn Any>, usize, &mut Archetype),
}

impl ComponentColumn {
    /// we use function pointers due to dynamic size of the variables
    fn new<T: 'static>() -> Self {
        Self {
            data: Box::new(Vec::<T>::new()),
            swap_remove_fn: |any_vec, index| {
                if let Some(vec) = any_vec.downcast_mut::<Vec<T>>() {
                    vec.swap_remove(index);
                }
            },
            move_to_fn: |any_vec, index, target_archetype| {
                if let Some(vec) = any_vec.downcast_mut::<Vec<T>>() {
                    let component = vec.swap_remove(index);
                    target_archetype.put_component(component);
                }
            },
        }
    }

    fn get_vec_mut<T: 'static>(&mut self) -> Option<&mut Vec<T>> {
        self.data.downcast_mut::<Vec<T>>()
    }
}

/// an `Archetype` stores all entities that have the exact same set of component types
pub struct Archetype {
    /// the set of component types that define this archetype
    types: HashSet<TypeId>,
    /// list of components with its `TypeId` as index
    columns: HashMap<TypeId, ComponentColumn>,
    /// list of entities to this archetype
    entities: Vec<Entity>,
}

impl Archetype {
    /// creates a new `Archetype` with the given component types
    fn new(
        types: HashSet<TypeId>,
        column_creators: &HashMap<TypeId, fn() -> ComponentColumn>,
    ) -> Self {
        let mut columns = HashMap::new();
        for type_id in &types {
            if let Some(creator) = column_creators.get(type_id) {
                columns.insert(*type_id, creator());
            }
        }
        Self {
            types,
            columns,
            entities: Vec::new(),
        }
    }

    /// adds an entity to this archetype
    fn push_entity(&mut self, entity: Entity) {
        self.entities.push(entity);
    }

    /// inserts a component into the appropriate column
    fn put_component<T: 'static>(&mut self, component: T) {
        if let Some(column) = self.columns.get_mut(&TypeId::of::<T>())
            && let Some(vec) = column.get_vec_mut::<T>()
        {
            vec.push(component);
        }
    }

    /// moves an entity and all its components from this archetype to a target archetype
    fn move_entity_to(&mut self, row_index: usize, target: &mut Archetype) -> Option<Entity> {
        let entity = self.entities.swap_remove(row_index);
        target.push_entity(entity);

        for (type_id, column) in &mut self.columns {
            if target.columns.contains_key(type_id) {
                (column.move_to_fn)(&mut column.data, row_index, target);
            } else {
                (column.swap_remove_fn)(&mut column.data, row_index);
            }
        }

        if row_index < self.entities.len() {
            Some(self.entities[row_index])
        } else {
            None
        }
    }

    pub fn get_components<T: 'static>(&self) -> Option<&[T]> {
        self.columns
            .get(&TypeId::of::<T>())?
            .data
            .downcast_ref::<Vec<T>>()
            .map(|vec| vec.as_slice())
    }
    pub fn get_components_mut_ptr<T: 'static>(&mut self) -> Option<*mut T> {
        let column = self.columns.get_mut(&TypeId::of::<T>())?;
        let vec = column.get_vec_mut::<T>()?;
        Some(vec.as_mut_ptr())
    }
}

/// internal mapping to track where an entity's data is stored.
#[derive(Clone, Copy)]
struct EntityLocation {
    /// the index of the archetype in the `World`
    archetype_id: usize,
    /// the row index within the archetype's columns
    row_index: usize,
}

/// The `World` is the main container for all ECS data.
/// It manages entity lifecycle, component registration, and archetype transitions.
pub struct World {
    /// Generations for all entities (indexed by ID).
    entities: Vec<u32>,
    /// List of IDs available for reuse.
    free_entities: Vec<u32>,
    /// Map of entities to their current location in archetypes.
    entity_locations: HashMap<Entity, EntityLocation>,
    /// All archetypes in the world.
    pub archetypes: Vec<Archetype>,
    /// Factory functions for creating new component columns.
    column_creators: HashMap<TypeId, fn() -> ComponentColumn>,
}

impl Default for World {
    fn default() -> Self {
        Self::new()
    }
}

impl World {
    /// Creates a new, empty ECS world.
    pub fn new() -> Self {
        Self {
            entities: Vec::new(),
            free_entities: Vec::new(),
            entity_locations: HashMap::new(),
            archetypes: Vec::new(),
            column_creators: HashMap::new(),
        }
    }

    /// adds a new component type so it can be used by entities
    pub fn register_component<T: 'static>(&mut self) {
        self.column_creators
            .insert(TypeId::of::<T>(), || ComponentColumn::new::<T>());
    }

    /// new entity constructor
    pub fn spawn(&mut self) -> Entity {
        let (id, generation) = if let Some(free_id) = self.free_entities.pop() {
            let generation = self.entities[free_id as usize];
            (free_id, generation)
        } else {
            let id = self.entities.len() as u32;
            self.entities.push(0);
            (id, 0)
        };

        let entity = Entity { id, generation };
        let empty_set = HashSet::new();
        let arch_id = self.get_or_create_archetype(empty_set);

        let archetype = &mut self.archetypes[arch_id];
        let row_index = archetype.entities.len();
        archetype.push_entity(entity);

        self.entity_locations.insert(
            entity,
            EntityLocation {
                archetype_id: arch_id,
                row_index,
            },
        );
        entity
    }

    /// adds a component to an existing entity
    /// this triggers an archetype transition: the entity's data is moved to a new archetype
    pub fn add_component<T: 'static>(&mut self, entity: Entity, component: T) {
        let loc = self
            .entity_locations
            .get(&entity)
            .cloned()
            .expect("Entity does not exist");

        let mut new_types = self.archetypes[loc.archetype_id].types.clone();
        new_types.insert(TypeId::of::<T>());

        let target_arch_id = self.get_or_create_archetype(new_types);

        if target_arch_id == loc.archetype_id {
            self.archetypes[loc.archetype_id].put_component(component);
        } else {
            let (left, right) = self
                .archetypes
                .split_at_mut(std::cmp::max(loc.archetype_id, target_arch_id));
            let (source_arch, target_arch) = if loc.archetype_id < target_arch_id {
                (&mut left[loc.archetype_id], &mut right[0])
            } else {
                (&mut right[0], &mut left[target_arch_id])
            };

            let target_row = target_arch.entities.len();
            let swapped_entity = source_arch.move_entity_to(loc.row_index, target_arch);

            target_arch.put_component(component);

            self.entity_locations.insert(
                entity,
                EntityLocation {
                    archetype_id: target_arch_id,
                    row_index: target_row,
                },
            );
            if let Some(swapped) = swapped_entity
                && let Some(swapped_loc) = self.entity_locations.get_mut(&swapped)
            {
                swapped_loc.row_index = loc.row_index;
            }
        }
    }

    /// gets an existing archetype or creates a new one for the given component set.
    fn get_or_create_archetype(&mut self, types: HashSet<TypeId>) -> usize {
        if let Some(pos) = self.archetypes.iter().position(|a| a.types == types) {
            pos
        } else {
            let arch = Archetype::new(types, &self.column_creators);
            self.archetypes.push(arch);
            self.archetypes.len() - 1
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::ecs::storage::World;
    use std::any::TypeId;

    #[test]
    fn create_entity() {
        let mut world = World::new();
        let new_ent = world.spawn();
        assert_eq!(new_ent.id, 0);
    }

    #[test]
    fn add_component() {
        struct TestComponent {
            value: i32,
        };
        let mut world = World::new();
        let ent = world.spawn();

        world.register_component::<TestComponent>();
        world.add_component(ent, TestComponent { value: 42 });

        let archetype = &world.archetypes[world.entity_locations[&ent].archetype_id];
        assert!(archetype.types.contains(&TypeId::of::<TestComponent>()));
    }

    #[test]
    fn archetype_transition() {
        struct ComponentA;
        struct ComponentB;

        let mut world = World::new();
        let ent = world.spawn();

        world.register_component::<ComponentA>();
        world.register_component::<ComponentB>();

        world.add_component(ent, ComponentA);
        let arch_id_a = world.entity_locations[&ent].archetype_id;

        world.add_component(ent, ComponentB);
        let arch_id_b = world.entity_locations[&ent].archetype_id;

        assert_ne!(arch_id_a, arch_id_b);
    }
}
