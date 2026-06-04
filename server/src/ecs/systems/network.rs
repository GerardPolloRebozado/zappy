use nix::poll::PollFlags;

use crate::server::Server;

pub fn network_system(server: &mut Server) {
    let fds = server.get_server_events();

    let listener_ready = fds[0]
        .revents()
        .is_some_and(|f| f.contains(PollFlags::POLLIN));

    let client_revents: Vec<PollFlags> = fds[1..]
        .iter()
        .map(|fd| fd.revents().unwrap_or(PollFlags::empty()))
        .collect();

    server.process_client_events(client_revents);

    if listener_ready {
        server.accept_connections();
    }
}
