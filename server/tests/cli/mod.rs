use myteams::common::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use std::str::FromStr;

#[test]
fn test_cli_request_creation_feature() {
    // Testing that the protocol handles all the commands used by CLI
    let commands = vec![
        Command::Users,
        Command::User("u1".to_string()),
        Command::Messages("u2".to_string()),
        Command::Subscribe("t1".to_string()),
        Command::Unsubscribe("t1".to_string()),
        Command::List,
        Command::Info,
    ];

    for cmd in commands {
        let req = Request {
            command: cmd.clone(),
        };
        let req_str = req.to_string();
        let parsed_req = Request::from_str(&req_str).unwrap();
        assert_eq!(parsed_req.command, cmd);
    }
}

#[test]
fn test_cli_error_response_handling_protocol() {
    let error_codes = vec![
        StatusCode::Unauthorized,
        StatusCode::NotFound,
        StatusCode::BadRequest,
        StatusCode::Conflict,
    ];

    for code in error_codes {
        let resp = Response {
            code: ResponseCode::Status(code),
            data: None,
        };
        let resp_str = resp.to_string();
        let parsed_resp = Response::from_str(&resp_str).unwrap();
        assert_eq!(parsed_resp.code, ResponseCode::Status(code));
    }
}

#[test]
fn test_cli_create_commands_protocol() {
    let scenarios = vec![
        Command::Create(vec!["team".to_string(), "desc".to_string()]),
        Command::Create(vec!["chan".to_string(), "desc".to_string()]),
        Command::Create(vec!["thread".to_string(), "msg".to_string()]),
        Command::Create(vec!["reply".to_string()]),
    ];

    for cmd in scenarios {
        let req = Request {
            command: cmd.clone(),
        };
        let req_str = req.to_string();
        let parsed = Request::from_str(&req_str).unwrap();
        assert_eq!(parsed.command, cmd);
    }
}

#[test]
fn test_cli_use_command_variations_protocol() {
    let variations = vec![
        Command::Use(None, None, None),
        Command::Use(Some("t".to_string()), None, None),
        Command::Use(Some("t".to_string()), Some("c".to_string()), None),
        Command::Use(
            Some("t".to_string()),
            Some("c".to_string()),
            Some("th".to_string()),
        ),
    ];

    for cmd in variations {
        let req = Request {
            command: cmd.clone(),
        };
        let req_str = req.to_string();
        let parsed = Request::from_str(&req_str).unwrap();
        assert_eq!(parsed.command, cmd);
    }
}
