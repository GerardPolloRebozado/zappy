//! Server-side game events and their protocol formatting.
//!
//! A [`ServerEvent`] represents something that happened in the game world.
//! The same logical event is serialized differently depending on the client:
//!
//! - GUI clients receive lines such as `pbc #n message\n` via [`ServerEvent::to_gui_string`].
//! - AI clients receive lines such as `message k, text\n` via [`ServerEvent::to_ai_string`].
//!
//! Player position updates use `ppo #n X Y O\n` per the Epitech GUI protocol
//! ([G-YEP-400_zappy_GUI_protocol.pdf](docs/epitech/G-YEP-400_zappy_GUI_protocol.pdf)).

use crate::{
    ecs::components::inhabitant::Inhabitant,
    utils::orientation::{RelativeOrientation, calc_k},
};

/// A logical game occurrence broadcast by the server.
///
/// Variants mirror the Zappy protocol notifications (`pbc`, `pex`, `pnw`, …).
/// Use the constructor helpers (e.g. [`ServerEvent::message`]) when building
/// events from live [`Inhabitant`] state.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ServerEvent {
    /// Player broadcast: GUI `pbc #n M`, AI `message k, M`.
    Message {
        player_id: u32,
        message: String,
        x: u32,
        y: u32,
    },
    /// Player ejected from a tile: GUI `pex #n`, AI `eject: k`.
    Eject { player_id: u32, x: u32, y: u32 },
    /// Player death: GUI `pdi #n`, AI `dead`.
    Dead { player_id: u32 },

    /// New player spawn: GUI `pnw #n X Y O L N`.
    NewPlayer {
        player_id: u32,
        x: u32,
        y: u32,
        orientation: RelativeOrientation,
        level: u8,
        team: String,
    },
    /// Player position/orientation: GUI `ppo #n X Y O` (Epitech GUI protocol v3.3).
    PlayerPosition {
        player_id: u32,
        x: u32,
        y: u32,
        orientation: RelativeOrientation,
    },
    /// Incantation started: GUI `pic X Y L #n ...`.
    StartIncantation {
        x: u32,
        y: u32,
        level: u8,
        player_ids: Vec<u32>,
    },
    /// Incantation finished: GUI `pie X Y R`.
    EndIncantation { x: u32, y: u32, result: u8 },
    /// Player fork request: GUI `pfk #n`.
    EggLay { player_id: u32 },
    /// Resource dropped: GUI `pdr #n i`.
    ResourceDrop { player_id: u32, resource: u8 },
    /// Resource collected: GUI `pgt #n i`.
    ResourceCollect { player_id: u32, resource: u8 },
    /// Egg laid on the map: GUI `enw #e #n X Y`.
    EggLaid {
        egg_id: u32,
        player_id: u32,
        x: u32,
        y: u32,
    },
    /// Egg connection: GUI `ebo #e`.
    EggConnect { egg_id: u32 },
    /// Egg death: GUI `edi #e`.
    EggDeath { egg_id: u32 },
    /// Team victory: GUI `seg N`.
    EndOfGame { team: String },
    /// Server info message: GUI `smg M`.
    ServerMessage { message: String },
    /// A team with more than 6 players at max lvl
    WinGame { team_name: String },
    /// Explicit tile content broadcast (e.g. after resource spawn): GUI `bct X Y ...`
    TileContent { content: String },
}

impl ServerEvent {
    // Builder functions
    pub fn message(broadcaster: &Inhabitant, message: impl Into<String>) -> Self {
        Self::Message {
            player_id: broadcaster.id(),
            message: message.into(),
            x: broadcaster.x(),
            y: broadcaster.y(),
        }
    }
    pub fn eject(victim: &Inhabitant, source: &Inhabitant) -> Self {
        Self::Eject {
            player_id: victim.id(),
            x: source.x(),
            y: source.y(),
        }
    }
    pub fn death(player: &Inhabitant) -> Self {
        Self::Dead {
            player_id: player.id(),
        }
    }
    pub fn new_player(player: &Inhabitant, level: u8, team: impl Into<String>) -> Self {
        Self::NewPlayer {
            player_id: player.id(),
            x: player.x(),
            y: player.y(),
            orientation: player.orientation(),
            level,
            team: team.into(),
        }
    }
    pub fn player_position(player: &Inhabitant) -> Self {
        Self::PlayerPosition {
            player_id: player.id(),
            x: player.x(),
            y: player.y(),
            orientation: player.orientation(),
        }
    }
    pub fn start_incantation(x: u32, y: u32, level: u8, player_ids: Vec<u32>) -> Self {
        Self::StartIncantation {
            x,
            y,
            level,
            player_ids,
        }
    }
    pub fn egg_lay(player: &Inhabitant) -> Self {
        Self::EggLay {
            player_id: player.id(),
        }
    }
    pub fn resource_drop(player: &Inhabitant, resource: u8) -> Self {
        Self::ResourceDrop {
            player_id: player.id(),
            resource,
        }
    }
    pub fn resource_collect(player: &Inhabitant, resource: u8) -> Self {
        Self::ResourceCollect {
            player_id: player.id(),
            resource,
        }
    }
    pub fn egg_laid(egg_id: u32, player: &Inhabitant) -> Self {
        Self::EggLaid {
            egg_id,
            player_id: player.id(),
            x: player.x(),
            y: player.y(),
        }
    }

    pub fn win_game(team_name: String) -> Self {
        Self::WinGame { team_name }
    }

    pub fn tile_content(content: String) -> Self {
        Self::TileContent { content }
    }

    /// Formats this event for GUI clients.
    ///
    /// Returns `None` only when the variant has no GUI representation
    /// (currently all variants produce a GUI line).
    pub fn to_gui_string(&self) -> Option<String> {
        match self {
            ServerEvent::Message {
                player_id, message, ..
            } => Some(format!("pbc #{player_id} {message}")),
            ServerEvent::Eject { player_id, .. } => Some(format!("pex #{player_id}")),
            ServerEvent::Dead { player_id } => Some(format!("pdi #{player_id}")),
            ServerEvent::NewPlayer {
                player_id,
                x,
                y,
                orientation,
                level,
                team,
            } => Some(format!(
                "pnw #{player_id} {x} {y} {} {level} {team}",
                orientation.as_gui_orientation()
            )),
            ServerEvent::PlayerPosition {
                player_id,
                x,
                y,
                orientation,
            } => Some(format!(
                "ppo #{player_id} {x} {y} {}",
                orientation.as_gui_orientation()
            )),
            ServerEvent::StartIncantation {
                x,
                y,
                level,
                player_ids,
            } => {
                let players = player_ids
                    .iter()
                    .map(|id| format!("#{id}"))
                    .collect::<Vec<_>>()
                    .join(" ");
                Some(format!("pic {x} {y} {level} {players}"))
            }
            ServerEvent::EndIncantation { x, y, result } => Some(format!("pie {x} {y} {result}")),
            ServerEvent::EggLay { player_id } => Some(format!("pfk #{player_id}")),
            ServerEvent::ResourceDrop {
                player_id,
                resource,
            } => Some(format!("pdr #{player_id} {resource}")),
            ServerEvent::ResourceCollect {
                player_id,
                resource,
            } => Some(format!("pgt #{player_id} {resource}")),
            ServerEvent::EggLaid {
                egg_id,
                player_id,
                x,
                y,
            } => Some(format!("enw #{egg_id} #{player_id} {x} {y}")),
            ServerEvent::EggConnect { egg_id } => Some(format!("ebo #{egg_id}")),
            ServerEvent::EggDeath { egg_id } => Some(format!("edi #{egg_id}")),
            ServerEvent::EndOfGame { team } => Some(format!("seg {team}")),
            ServerEvent::ServerMessage { message } => Some(format!("smg {message}")),
            ServerEvent::WinGame { team_name: team_id } => Some(format!("seg {team_id}")),
            ServerEvent::TileContent { content } => Some(content.clone()),
        }
    }

    /// Formats this event for a specific AI client.
    ///
    /// Returns `None` when the event is not relevant to `for_player`, or when
    /// the variant is GUI-only (e.g. [`ServerEvent::NewPlayer`]).
    ///
    /// # Examples
    ///
    /// ```
    /// use zappy_engine::Inhabitant; use zappy_engine::protocol::ServerEvent;
    /// use zappy_engine::utils::orientation::RelativeOrientation;
    ///
    /// let player = Inhabitant::default().with_id(5);
    /// let event = ServerEvent::Dead { player_id: 5 };
    /// assert_eq!(event.to_ai_string(Some(&player), 10, 10), Some("dead".to_string()));
    /// ```
    pub fn to_ai_string(
        &self,
        for_player: Option<&Inhabitant>,
        map_width: u32,
        map_height: u32,
    ) -> Option<String> {
        match self {
            ServerEvent::Dead { player_id } => {
                if let Some(p) = for_player {
                    if p.id() == *player_id {
                        Some("dead".to_string())
                    } else {
                        None
                    }
                } else {
                    None
                }
            }
            ServerEvent::Eject { player_id, x, y } => {
                let for_player = for_player?;
                if for_player.id() != *player_id {
                    return None;
                }
                let k = calc_k(*x, *y, for_player, map_width, map_height);
                Some(format!("eject: {k}"))
            }
            ServerEvent::Message { x, y, message, .. } => {
                let for_player = for_player?;
                let k = calc_k(*x, *y, for_player, map_width, map_height);
                Some(format!("message {k}, {message}"))
            }
            ServerEvent::NewPlayer { .. }
            | ServerEvent::PlayerPosition { .. }
            | ServerEvent::StartIncantation { .. }
            | ServerEvent::EndIncantation { .. }
            | ServerEvent::EggLay { .. }
            | ServerEvent::ResourceDrop { .. }
            | ServerEvent::ResourceCollect { .. }
            | ServerEvent::EggLaid { .. }
            | ServerEvent::EggConnect { .. }
            | ServerEvent::EggDeath { .. }
            | ServerEvent::TileContent { .. }
            | ServerEvent::EndOfGame { .. } => None,
            ServerEvent::ServerMessage { message } => {
                let _ = for_player?;
                if message.starts_with("event_start ") || message.starts_with("event_end ") {
                    Some(message.clone())
                } else {
                    None
                }
            }
            ServerEvent::WinGame { .. } => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_dead_formats() {
        let event = ServerEvent::Dead { player_id: 5 };
        assert_eq!(event.to_gui_string(), Some("pdi #5".to_string()));

        let for_player = Inhabitant::default().with_id(5);
        assert_eq!(
            event.to_ai_string(Some(&for_player), 10, 10),
            Some("dead".to_string())
        );
    }

    #[test]
    fn test_eject_and_broadcast_gui() {
        let eject = ServerEvent::Eject {
            player_id: 3,
            x: 1,
            y: 2,
        };
        assert_eq!(eject.to_gui_string(), Some("pex #3".to_string()));

        let message = ServerEvent::Message {
            player_id: 2,
            message: "hello".to_string(),
            x: 4,
            y: 5,
        };
        assert_eq!(message.to_gui_string(), Some("pbc #2 hello".to_string()));
    }

    #[test]
    fn test_eject_k_relative_to_orientation() {
        let event = ServerEvent::Eject {
            player_id: 1,
            x: 5,
            y: 4,
        };

        let facing_north = Inhabitant::default().with_id(1).with_pos(5, 5);
        assert_eq!(
            event.to_ai_string(Some(&facing_north), 10, 10),
            Some("eject: 1".to_string())
        );

        let facing_east = Inhabitant::default()
            .with_id(1)
            .with_pos(5, 5)
            .with_orientation(RelativeOrientation::Right);
        assert_eq!(
            event.to_ai_string(Some(&facing_east), 10, 10),
            Some("eject: 3".to_string())
        );
    }

    #[test]
    fn test_constructors_use_player_fields() {
        let broadcaster = Inhabitant::default().with_id(7).with_pos(2, 3);
        let event = ServerEvent::message(&broadcaster, "hi");
        assert!(
            matches!(event, ServerEvent::Message { player_id: 7, message, x: 2, y: 3, } if message == "hi")
        );
    }

    #[test]
    fn test_player_position_gui_format() {
        assert_eq!(
            ServerEvent::PlayerPosition {
                player_id: 3,
                x: 7,
                y: 9,
                orientation: RelativeOrientation::Right,
            }
            .to_gui_string(),
            Some("ppo #3 7 9 2".to_string())
        );
    }

    #[test]
    fn test_player_position_constructor() {
        let player = Inhabitant::default()
            .with_id(4)
            .with_pos(1, 2)
            .with_orientation(RelativeOrientation::ForwardRight);
        let event = ServerEvent::player_position(&player);
        assert!(matches!(
            event,
            ServerEvent::PlayerPosition {
                player_id: 4,
                x: 1,
                y: 2,
                orientation: RelativeOrientation::ForwardRight
            }
        ));
    }

    #[test]
    fn test_player_position_ai_string_is_none() {
        let event = ServerEvent::PlayerPosition {
            player_id: 1,
            x: 0,
            y: 0,
            orientation: RelativeOrientation::Forward,
        };
        let player = Inhabitant::default().with_id(1);
        assert_eq!(event.to_ai_string(Some(&player), 10, 10), None);
    }

    #[test]
    fn test_gui_event_formats() {
        assert_eq!(
            ServerEvent::NewPlayer {
                player_id: 1,
                x: 3,
                y: 4,
                orientation: RelativeOrientation::Right,
                level: 5,
                team: "TeamA".to_string()
            }
            .to_gui_string(),
            Some("pnw #1 3 4 2 5 TeamA".to_string())
        );

        assert_eq!(
            ServerEvent::StartIncantation {
                x: 1,
                y: 2,
                level: 3,
                player_ids: vec![4, 5]
            }
            .to_gui_string(),
            Some("pic 1 2 3 #4 #5".to_string())
        );

        assert_eq!(
            ServerEvent::EndIncantation {
                x: 1,
                y: 2,
                result: 1
            }
            .to_gui_string(),
            Some("pie 1 2 1".to_string())
        );

        assert_eq!(
            ServerEvent::EggLay { player_id: 2 }.to_gui_string(),
            Some("pfk #2".to_string())
        );

        assert_eq!(
            ServerEvent::ResourceDrop {
                player_id: 1,
                resource: 3
            }
            .to_gui_string(),
            Some("pdr #1 3".to_string())
        );

        assert_eq!(
            ServerEvent::ResourceCollect {
                player_id: 1,
                resource: 0
            }
            .to_gui_string(),
            Some("pgt #1 0".to_string())
        );

        assert_eq!(
            ServerEvent::EggLaid {
                egg_id: 10,
                player_id: 2,
                x: 5,
                y: 6
            }
            .to_gui_string(),
            Some("enw #10 #2 5 6".to_string())
        );

        assert_eq!(
            ServerEvent::EggConnect { egg_id: 7 }.to_gui_string(),
            Some("ebo #7".to_string())
        );
        assert_eq!(
            ServerEvent::EggDeath { egg_id: 8 }.to_gui_string(),
            Some("edi #8".to_string())
        );

        assert_eq!(
            ServerEvent::EndOfGame {
                team: "TeamA".to_string()
            }
            .to_gui_string(),
            Some("seg TeamA".to_string())
        );

        assert_eq!(
            ServerEvent::ServerMessage {
                message: "server info".to_string()
            }
            .to_gui_string(),
            Some("smg server info".to_string())
        );
    }

    #[test]
    fn test_map_event_server_message_to_ai() {
        let player = Inhabitant::default().with_id(1);
        let start = ServerEvent::ServerMessage {
            message: "event_start meteor_shower".to_string(),
        };
        assert_eq!(
            start.to_ai_string(Some(&player), 10, 10),
            Some("event_start meteor_shower".to_string())
        );

        let end = ServerEvent::ServerMessage {
            message: "event_end solar_flare".to_string(),
        };
        assert_eq!(
            end.to_ai_string(Some(&player), 10, 10),
            Some("event_end solar_flare".to_string())
        );

        let other = ServerEvent::ServerMessage {
            message: "server info".to_string(),
        };
        assert_eq!(other.to_ai_string(Some(&player), 10, 10), None);
    }
}
