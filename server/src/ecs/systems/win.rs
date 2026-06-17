use std::collections::HashMap;

use log::error;

use crate::{
    ecs::{
        components::{level::Level, team::Team},
        storage::World,
        systems::task::broadcast_event,
    },
    protocol::ServerEvent,
};

pub fn check_team_won(world: &mut World) {
    let player_storage = world.get_storage::<Level>();
    if player_storage.is_none() {
        return;
    }
    let lvl_storage = player_storage.unwrap();
    let mut teams_with_max_lvl_amount: HashMap<String, u8> = HashMap::new();
    let mut team_name = String::new();
    for (e, lvl) in lvl_storage.iter() {
        if lvl.value == 8 {
            let team = world.get_component::<Team>(*e);
            if team.is_none() {
                error!("Entity: {} should have a team", *e);
                return;
            }
            let team = team.unwrap();
            match team {
                Team::WaitingForTeamName => todo!(),
                Team::AuthenticatedAI(name) => {
                    if teams_with_max_lvl_amount.contains_key(name) {
                        teams_with_max_lvl_amount
                            .insert(name.clone(), teams_with_max_lvl_amount[name] + 1);
                        if *teams_with_max_lvl_amount.get(name).unwrap() > 6_u8 {
                            team_name = name.clone();
                        }
                    } else {
                        teams_with_max_lvl_amount.insert(name.clone(), 1);
                    }
                }
                Team::AuthenticatedGUI => todo!(),
            }
        }
    }
    if !team_name.is_empty() {
        broadcast_event(
            world,
            ServerEvent::WinGame {
                team_name: team_name.clone(),
            },
        );
    }
}
