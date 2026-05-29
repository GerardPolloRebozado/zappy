use crate::server::Server;
use crate::signal::{SIGNAL_RECEIVED, install_sigint_handler};
use std::io;
use zappy_server::ecs::storage::World;

mod server;
pub mod signal;

fn main() -> io::Result<()> {
    let _ = World::new();
    install_sigint_handler();
    let mut server = Server::new();
    server.load();
    loop {
        server.run();
        if SIGNAL_RECEIVED.load(std::sync::atomic::Ordering::SeqCst) > 0 {
            server.save();
            break;
        }
    }
    Ok(())
}
