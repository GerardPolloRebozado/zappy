use env_logger::Builder;
use log::LevelFilter;
use std::io;

use crate::{
    server::Server,
    utils::{constants::ERROR_EXIT_CODE, parse_server_args},
};

pub mod commands;
pub mod ecs;
pub mod protocol;
pub mod server;
pub mod utils;

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
    loop {
        server.run();
    }
}
