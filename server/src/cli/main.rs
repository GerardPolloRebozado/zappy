mod commands;
pub mod client;

use crate::client::Cli;
use std::env;
use std::io::{self};

fn main() -> io::Result<()> {
    let args: Vec<String> = env::args().collect();
    if args.len() != 3 {
        print_usage();
        return Ok(());
    }
    let addr = format!("{}:{}", args[1], args[2]);
    let mut cli = Cli::new(addr.as_str());
    cli.run();
    Ok(())
}

fn print_usage() {
    println!("USAGE: ./myteams_cli ip port");
    println!("ip is the server ip address");
    println!("port is the port number");
}
