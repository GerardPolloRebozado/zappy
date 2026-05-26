use myteams::common::message::Location;
use myteams::common::{Channel, Date, Message, Team, Thread};
use std::fs::{File, remove_file};

#[test]
fn test_feature_hierarchy_save_load() {
    let mut team = Team::new("FeatureTeam".to_string());
    let mut channel = Channel::new("FeatureChannel");
    let mut thread = Thread::new("FeatureThread".to_string());
    let message = Message::new(
        "u1".to_string(),
        Location::THREAD,
        "th1".to_string(),
        "hi".to_string(),
    );

    thread.messages.push(message);
    channel.threads.insert("th1".to_string(), thread);
    team.channels.insert("c1".to_string(), channel);

    let filename = "feature_hierarchy.bin";
    {
        let mut file = File::create(filename).unwrap();
        team.write_to_file(&mut file);
    }

    {
        let mut file = File::open(filename).unwrap();
        let loaded = Team::read_from_file(&mut file);
        assert_eq!(loaded.name, "FeatureTeam");
        assert_eq!(loaded.channels.len(), 1);
    }
    let _ = remove_file(filename);
}

#[test]
fn test_feature_protocol_roundtrip() {
    use myteams::common::protocol::{Command, Request, Response, ResponseCode, StatusCode};
    use std::str::FromStr;

    // Login -> Response
    let login_raw = "LOGIN \"user\"\n";
    let req = Request::from_str(login_raw).unwrap();
    assert!(matches!(req.command, Command::Login(_)));

    let resp = Response {
        code: ResponseCode::Status(StatusCode::Ok),
        data: Some("\"uuid\" \"user\"".to_string()),
    };
    let resp_raw = resp.to_string();
    let parsed = Response::from_str(&resp_raw).unwrap();
    assert_eq!(parsed.code, ResponseCode::Status(StatusCode::Ok));
}
