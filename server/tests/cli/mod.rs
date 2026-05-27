use zappy::common::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use std::str::FromStr;

#[test]
fn test_cli_request_creation_feature() {
    // Testing that the protocol handles all the commands used by CLI
    let commands = vec![
        Command::Users,
        Command::User("u1".to_string()),
        Command::Logout,
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
