use zappy_server::common::protocol::{Response, ResponseCode, event::EventCode};
use zappy_server::common::{Team, User};

#[test]
fn test_server_event_serialization_feature() {
    let events = vec![
        (EventCode::LoggedIn, Some("\"u1\" \"name\"".to_string())),
        (EventCode::LoggedOut, Some("\"u1\" \"name\"".to_string())),
    ];

    for (code, data) in events {
        let resp = Response {
            code: ResponseCode::Event(code),
            data: data.clone(),
        };
        let resp_str = resp.to_string();
        assert!(resp_str.contains(&code.to_u32().to_string()));
        if let Some(d) = data {
            assert!(resp_str.contains(&d[..4]));
        }
    }
}
