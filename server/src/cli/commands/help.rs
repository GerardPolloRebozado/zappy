pub fn help() {
    println!("Available commands:");
    println!("/help");
    println!("/login [\"user_name\"]");
    println!("/logout");
    println!("/users");
    println!("/user [\"user_uuid\"]");
    println!("/send [\"user_uuid\"] [\"message_body\"]");
    println!("/messages [\"user_uuid\"]");
    println!("/subscribe [\"team_uuid\"]");
    println!("/subscribed ?[\"team_uuid\"]");
    println!("/unsubscribe [\"team_uuid\"]");
    println!("/use ?[\"team_uuid\"] ?[\"channel_uuid\"] ?[\"thread_uuid\"]");
    println!("/create");
    println!("/list");
    println!("/info");
}
