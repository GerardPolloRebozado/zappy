use nix::poll::PollFlags;

use crate::server::Server;

pub fn network_system(server: &mut Server) {
    let fds = server.get_server_events();

    if fds.is_empty() {
        return;
    }

    let listener_ready = if server.listener.is_some() {
        fds[0]
            .revents()
            .is_some_and(|f| f.contains(PollFlags::POLLIN))
    } else {
        false
    };

    let client_revents_start = if server.listener.is_some() { 1 } else { 0 };

    let client_revents: Vec<PollFlags> = fds[client_revents_start..]
        .iter()
        .map(|fd| fd.revents().unwrap_or(PollFlags::empty()))
        .collect();

    server.process_client_events(client_revents);

    if listener_ready {
        server.accept_connections();
    }
}
