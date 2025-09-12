use fctools::*;
use std::fs::File;
use std::io::{BufReader, BufWriter};
use std::net::SocketAddr;
use std::net::SocketAddrV4;
use std::net::SocketAddrV6;
use std::net::TcpListener;
use std::net::TcpStream;

fn main() {
    let mut vm = VM::new(VMConfig::default()).unwrap();
    vm.start().unwrap();
    vm.wait_for_ssh().unwrap();
    vm.run_command("ls -l").unwrap();
    vm.stop().unwrap();
    println!("Hello, world!");
}
