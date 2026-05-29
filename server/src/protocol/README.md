## How to use it

### 1. Client: Sending a Command
Turn a Rust object into a string and send it over the socket.
```rust
let req = Request { command: Command::Login("Alex".into()) };
stream.write_all(req.to_string().as_bytes())?; // Sends "LOGIN Alex\n"
```

### 2. Server: Receiving a Command
Turn the raw text from the socket back into a Rust object.
```rust
let msg = read_string_from_socket();
let req = Request::from_str(&msg).unwrap();
match req.command {
    Command::Login(name) => { /* */ },
    _ => { /* etc */ }
}
```

### 3. Server: Sending a Response (or Event)
Format a result code and some data into a standard string.
```rust
let res = Response { 
    code: ResponseCode::Status(StatusCode::Ok), 
    data: Some("Success".into()) 
};
socket.write_all(res.to_string().as_bytes())?; // Sends "200 Success\n"
```

### 4. Client: Receiving a Response
Turn the server's text back into a Rust object to see what happened.
```rust
let msg = read_from_server();
let res = Response::from_str(&msg).unwrap();
match res.code {
    ResponseCode::Status(StatusCode::Ok) => { },
    ResponseCode::Event(EventCode::LoggedIn) => { },
    _ => { /* Handle error or other codes */ }
}
```
