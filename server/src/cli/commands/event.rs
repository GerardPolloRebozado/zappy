use myteams::common::protocol::event::EventCode;
use myteams::common::protocol::response::Response;
use myteams::common::utils::parse_args;
use myteams::*;

pub fn handle_event(event: Response) {
    let code = match event.code {
        common::protocol::response::ResponseCode::Event(c) => c,
        _ => return,
    };

    let data = event.data.unwrap_or_default();
    let args = parse_args(&format!("NOTHING {}", data));
    let args: Vec<&str> = args.iter().skip(1).map(|s| s.as_str()).collect();

    match code {
        EventCode::LoggedIn => {
            if args.len() >= 2 {
                client_event_logged_in(args[0], args[1]);
            }
        }
        EventCode::LoggedOut => {
            if args.len() >= 2 {
                client_event_logged_out(args[0], args[1]);
            }
        }
        EventCode::MessageReceived => {
            if args.len() >= 2 {
                client_event_private_message_received(args[0], args[1]);
            }
        }
        EventCode::ThreadCreated => {
            if args.len() >= 5 {
                client_event_thread_created(
                    args[0],
                    args[1],
                    args[2].parse().unwrap_or(0),
                    args[3],
                    args[4],
                );
            }
        }
        EventCode::CommentCreated => {
            if args.len() >= 4 {
                client_event_thread_reply_received(args[0], args[1], args[2], args[3]);
            }
        }
        EventCode::TeamCreated => {
            if args.len() >= 3 {
                client_event_team_created(args[0], args[1], args[2]);
            }
        }
        EventCode::ChannelCreated => {
            if args.len() >= 3 {
                client_event_channel_created(args[0], args[1], args[2]);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::protocol::response::ResponseCode;

    #[test]
    fn test_handle_event_logged_in() {
        let event = Response {
            code: ResponseCode::Event(EventCode::LoggedIn),
            data: Some("\"uuid\" \"name\"".to_string()),
        };
        handle_event(event); // Should call client_event_logged_in and not panic
    }

    #[test]
    fn test_handle_event_message_received() {
        let event = Response {
            code: ResponseCode::Event(EventCode::MessageReceived),
            data: Some("\"sender\" \"body\"".to_string()),
        };
        handle_event(event);
    }

    #[test]
    fn test_handle_event_thread_created() {
        let event = Response {
            code: ResponseCode::Event(EventCode::ThreadCreated),
            data: Some("\"uuid\" \"user\" \"12345\" \"title\" \"body\"".to_string()),
        };
        handle_event(event);
    }
}

