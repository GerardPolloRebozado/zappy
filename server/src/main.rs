use env_logger::Builder;
use log::LevelFilter;
use std::io;

use zappy_engine::{
    server::Server,
    utils::{constants::ERROR_EXIT_CODE, parse_server_args},
};

fn main() -> io::Result<()> {
    Builder::from_default_env()
        .filter_level(LevelFilter::Trace)
        .init();

    let args: Vec<String> = std::env::args().collect();
    let config = match parse_server_args(&args) {
        Ok(config) => config,
        Err(error) => {
            eprintln!("Error: {error}");
            eprintln!(
                "Usage: ./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq"
            );
            std::process::exit(ERROR_EXIT_CODE);
        }
    };

    let mut server = Server::new(config);
    server.load();
    let start_time = std::time::Instant::now();
    let initial_time = server.world.current_time;
    loop {
        server.world.current_time = initial_time + start_time.elapsed().as_millis() as u64;
        server.run();
    }
}
