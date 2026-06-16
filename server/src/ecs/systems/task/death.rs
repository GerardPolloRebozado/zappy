use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};

/// Runs the AI `Death` task for an entity.
///
/// Returns `ok` with `"dead"` as response data. The caller writes that response
/// straight to the socket and despawns the entity.
pub fn execute_death() -> (Response, Option<ServerEvent>) {
    (
        Response::new(
            ResponseCode::Status(StatusCode::Ok),
            Some("dead".to_string()),
        ),
        None,
    )
}
