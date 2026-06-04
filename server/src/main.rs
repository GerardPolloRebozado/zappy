use std::io;
use zappy_server::server::Server;
use zappy_server::server::signal::{SIGNAL_RECEIVED, install_sigint_handler};
use zappy_server::utils::parse_args;
use zappy_server::utils::parse_server_args;
use zappy_server::utils::constants::ERROR_EXIT_CODE;

fn main() -> io::Result<()> {
    install_sigint_handler();

    let args: Vec<String> = std::env::args().collect();
    let config = match parse_server_args(&args) {
        Ok(config) => config,
        Err(error) => {
            eprintln!("Error: {error}");
            eprintln!("Usage: ./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq");
            std::process::exit(ERROR_EXIT_CODE);
        }
    };

    let mut server = Server::new(config);
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
