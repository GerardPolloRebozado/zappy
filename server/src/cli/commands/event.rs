use zappy::common::protocol::response::Response;

pub fn handle_event(event: Response) {
    let code = match event.code {
        zappy::common::protocol::response::ResponseCode::Event(c) => c,
        _ => return,
    };

    let data = event.data.unwrap_or_default();
    println!("Event received: {:?} data: {}", code, data);
}
