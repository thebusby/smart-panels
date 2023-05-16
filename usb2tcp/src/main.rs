use std::{
    io::{prelude::*, BufReader, BufWriter},
    net::{TcpListener, TcpStream},
};
use std::env;
use std::collections::HashMap;




fn main() -> std::io::Result<()> {

    // Get ports from command line
    /*
    let addr = env::args()
        .nth(1)
        .unwrap_or_else(|| "127.0.0.1:8080".to_string());
    */


    let ports = serialport::available_ports().expect("No ports found!");
    for p in ports {
        println!("Port: {}", p.port_name);
    }

    // let port = serialport::new("/dev/ttyUSB0", 115_200)
    //    .timeout(Duration::from_millis(10))
    //    .open().expect("Failed to open port");

    // Writing to a port:
    // let output = "This is a test. This is only a test.".as_bytes();
    // port.write(output).expect("Write failed!");

    // Reading from a port (default is blocking with a 0ms timeout):
    // let mut serial_buf: Vec<u8> = vec![0; 64];
    // port.read(serial_buf.as_mut_slice()).expect("Found no data!");

    let listener = TcpListener::bind("127.0.0.1:1984")?;

    for stream in listener.incoming() {
        let stream = stream?;

        println!("Connection established!");
        handle_connection(stream);
    }

    Ok(())
}


fn handle_connection(mut stream: TcpStream) -> std::io::Result<()> {
    let mut reader = BufReader::new(&stream);
    let mut writer = BufWriter::new(&stream);

    for mline in reader.lines() {
        if let Ok(line) = mline {
            println!("Line: {:#?}", line);

            let response = "ACK\r\n";
            writer.write_all(response.as_bytes())?;
            writer.flush()?;
        }else{
            println!("Some(line) failed");
            break;
        }
    }

    Ok(())
}

fn handle_connection_old(mut stream: TcpStream) {
    let buf_reader = BufReader::new(&mut stream);
    let http_request: Vec<_> = buf_reader
        .lines()
        .map(|result| result.unwrap())
        .take_while(|line| !line.is_empty())
        .collect();

    println!("Request: {:#?}", http_request);

    let response = "HTTP/1.1 200 OK\r\n\r\n";

    stream.write_all(response.as_bytes()).unwrap();
}
