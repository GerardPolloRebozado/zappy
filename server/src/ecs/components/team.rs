use crate::ecs::storage::{ComponentMap, World};

#[derive(Clone, PartialEq, Eq, Debug, Default)]
pub enum Team {
    #[default]
    WaitingForTeamName,
    AuthenticatedAI(String),
    AuthenticatedGUI,
}

impl Team {
    pub fn team_members(team_name: String, world: &World) -> ComponentMap<Team> {
        let mut members = ComponentMap::new();

        if let Some(team_storage) = world.get_storage::<Team>() {
            let target_team = Team::AuthenticatedAI(team_name);

            for (entity, team) in team_storage.iter() {
                if *team == target_team {
                    members.insert(*entity, team.clone());
                }
            }
        }

        members
    }
}
