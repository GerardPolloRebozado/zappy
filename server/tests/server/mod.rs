use zappy_server::common::protocol::{Response, ResponseCode, ServerEvent, StatusCode};

#[test]
fn test_server_event_broadcast_response() {
    let event = ServerEvent::Dead { player_id: 5 };
    let resp = Response::new(
        ResponseCode::Status(StatusCode::Ok),
        event.to_gui_string(),
    );

    assert_eq!(resp.to_string(), "pdi #5\n");
}
