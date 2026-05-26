pub mod common;
pub mod ffi;

use std::ffi::CString;

pub fn server_event_team_created(team_uuid: &str, team_name: &str, user_uuid: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_team_name = CString::new(team_name).expect("team_name contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe {
        ffi::server_event_team_created(
            c_team_uuid.as_ptr(),
            c_team_name.as_ptr(),
            c_user_uuid.as_ptr(),
        )
    }
}

pub fn server_event_channel_created(
    team_uuid: &str,
    channel_uuid: &str,
    channel_name: &str,
) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_channel_uuid = CString::new(channel_uuid).expect("channel_uuid contains null byte");
    let c_channel_name = CString::new(channel_name).expect("channel_name contains null byte");
    unsafe {
        ffi::server_event_channel_created(
            c_team_uuid.as_ptr(),
            c_channel_uuid.as_ptr(),
            c_channel_name.as_ptr(),
        )
    }
}

pub fn server_event_user_created_safe(user_uuid: &str, user_name: &str) -> i32 {
    let c_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_name = CString::new(user_name).expect("user_name contains null byte");
    unsafe { ffi::server_event_user_created(c_uuid.as_ptr(), c_name.as_ptr()) }
}

pub fn server_event_user_logged_in_safe(user_uuid: &str) -> i32 {
    let c_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe { ffi::server_event_user_logged_in(c_uuid.as_ptr()) }
}

pub fn server_event_user_logged_out_safe(user_uuid: &str) -> i32 {
    let c_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe { ffi::server_event_user_logged_out(c_uuid.as_ptr()) }
}

pub fn server_event_thread_created(
    channel_uuid: &str,
    thread_uuid: &str,
    user_uuid: &str,
    thread_title: &str,
    thread_body: &str,
) -> i32 {
    let c_channel_uuid = CString::new(channel_uuid).expect("channel_uuid contains null byte");
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_thread_title = CString::new(thread_title).expect("thread_title contains null byte");
    let c_thread_body = CString::new(thread_body).expect("thread_body contains null byte");
    unsafe {
        ffi::server_event_thread_created(
            c_channel_uuid.as_ptr(),
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            c_thread_title.as_ptr(),
            c_thread_body.as_ptr(),
        )
    }
}

pub fn server_event_reply_created(thread_uuid: &str, user_uuid: &str, reply_body: &str) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_reply_body = CString::new(reply_body).expect("reply_body contains null byte");
    unsafe {
        ffi::server_event_reply_created(
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            c_reply_body.as_ptr(),
        )
    }
}

pub fn server_event_user_subscribed(team_uuid: &str, user_uuid: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe { ffi::server_event_user_subscribed(c_team_uuid.as_ptr(), c_user_uuid.as_ptr()) }
}

pub fn server_event_user_unsubscribed(team_uuid: &str, user_uuid: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe { ffi::server_event_user_unsubscribed(c_team_uuid.as_ptr(), c_user_uuid.as_ptr()) }
}

pub fn server_event_user_loaded(user_uuid: &str, user_name: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_name = CString::new(user_name).expect("user_name contains null byte");
    unsafe { ffi::server_event_user_loaded(c_user_uuid.as_ptr(), c_name.as_ptr()) }
}

pub fn server_event_user_logged_in(user_uuid: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe { ffi::server_event_user_logged_in(c_user_uuid.as_ptr()) }
}

pub fn server_event_user_logged_out(user_uuid: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe { ffi::server_event_user_logged_out(c_user_uuid.as_ptr()) }
}

pub fn server_event_private_message_sended(
    sender_uuid: &str,
    receiver_uuid: &str,
    message_body: &str,
) -> i32 {
    let c_sender_uuid = CString::new(sender_uuid).expect("sender_uuid contains null byte");
    let c_receiver_uuid = CString::new(receiver_uuid).expect("receiver_uuid contains null byte");
    let c_message_body = CString::new(message_body).expect("message_body contains null byte");
    unsafe {
        ffi::server_event_private_message_sended(
            c_sender_uuid.as_ptr(),
            c_receiver_uuid.as_ptr(),
            c_message_body.as_ptr(),
        )
    }
}

pub fn client_event_logged_in(user_uuid: &str, user_name: &str) -> i32 {
    let c_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_name = CString::new(user_name).expect("user_name contains null byte");

    unsafe { ffi::client_event_logged_in(c_uuid.as_ptr(), c_name.as_ptr()) }
}

pub fn client_event_logged_out(user_uuid: &str, user_name: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_name = CString::new(user_name).expect("user_name contains null byte");
    unsafe { ffi::client_event_logged_out(c_user_uuid.as_ptr(), c_name.as_ptr()) }
}

pub fn client_event_private_message_received(user_uuid: &str, message_body: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_message_body = CString::new(message_body).expect("message_body contains null byte");
    unsafe {
        ffi::client_event_private_message_received(c_user_uuid.as_ptr(), c_message_body.as_ptr())
    }
}

pub fn client_event_thread_reply_received(
    team_uuid: &str,
    thread_uuid: &str,
    user_uuid: &str,
    reply_body: &str,
) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_reply_body = CString::new(reply_body).expect("reply_body contains null byte");
    unsafe {
        ffi::client_event_thread_reply_received(
            c_team_uuid.as_ptr(),
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            c_reply_body.as_ptr(),
        )
    }
}

pub fn client_event_team_created(team_uuid: &str, team_name: &str, team_description: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_name = CString::new(team_name).expect("team_name contains null byte");
    let c_description =
        CString::new(team_description).expect("team_description contains null byte");
    unsafe {
        ffi::client_event_team_created(
            c_team_uuid.as_ptr(),
            c_name.as_ptr(),
            c_description.as_ptr(),
        )
    }
}

pub fn client_event_channel_created(
    channel_uuid: &str,
    channel_name: &str,
    channel_description: &str,
) -> i32 {
    let c_channel_uuid = CString::new(channel_uuid).expect("channel_uuid contains null byte");
    let c_channel_name = CString::new(channel_name).expect("channel_name contains null byte");
    let c_description =
        CString::new(channel_description).expect("channel_description contains null byte");
    unsafe {
        ffi::client_event_channel_created(
            c_channel_uuid.as_ptr(),
            c_channel_name.as_ptr(),
            c_description.as_ptr(),
        )
    }
}

pub fn client_event_thread_created(
    thread_uuid: &str,
    user_uuid: &str,
    thread_timestamp: i64,
    thread_title: &str,
    thread_body: &str,
) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_thread_title = CString::new(thread_title).expect("thread_title contains null byte");
    let c_thread_body = CString::new(thread_body).expect("thread_body contains null byte");
    unsafe {
        ffi::client_event_thread_created(
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            thread_timestamp,
            c_thread_title.as_ptr(),
            c_thread_body.as_ptr(),
        )
    }
}

pub fn client_print_users(user_uuid: &str, user_name: &str, user_status: i32) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_user_name = CString::new(user_name).expect("user_name contains null byte");
    unsafe { ffi::client_print_users(c_user_uuid.as_ptr(), c_user_name.as_ptr(), user_status) }
}

pub fn client_print_teams(team_uuid: &str, team_name: &str, team_description: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_team_name = CString::new(team_name).expect("team_name contains null byte");
    let c_team_description =
        CString::new(team_description).expect("team_description contains null byte");
    unsafe {
        ffi::client_print_teams(
            c_team_uuid.as_ptr(),
            c_team_name.as_ptr(),
            c_team_description.as_ptr(),
        )
    }
}

pub fn client_team_print_channels(
    channel_uuid: &str,
    channel_name: &str,
    channel_description: &str,
) -> i32 {
    let c_channel_uuid = CString::new(channel_uuid).expect("channel_uuid contains null byte");
    let c_channel_name = CString::new(channel_name).expect("channel_name contains null byte");
    let c_channel_description =
        CString::new(channel_description).expect("channel_description contains null byte");
    unsafe {
        ffi::client_team_print_channels(
            c_channel_uuid.as_ptr(),
            c_channel_name.as_ptr(),
            c_channel_description.as_ptr(),
        )
    }
}

pub fn client_channel_print_threads(
    thread_uuid: &str,
    user_uuid: &str,
    thread_timestamp: i64,
    thread_title: &str,
    thread_body: &str,
) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_thread_title = CString::new(thread_title).expect("thread_title contains null byte");
    let c_thread_body = CString::new(thread_body).expect("thread_body contains null byte");
    unsafe {
        ffi::client_channel_print_threads(
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            thread_timestamp,
            c_thread_title.as_ptr(),
            c_thread_body.as_ptr(),
        )
    }
}

pub fn client_thread_print_replies(
    thread_uuid: &str,
    user_uuid: &str,
    reply_timestamp: i64,
    reply_body: &str,
) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_reply_body = CString::new(reply_body).expect("reply_body contains null byte");
    unsafe {
        ffi::client_thread_print_replies(
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            reply_timestamp,
            c_reply_body.as_ptr(),
        )
    }
}

pub fn client_private_message_print_messages(
    sender_uuid: &str,
    message_timestamp: i64,
    message_body: &str,
) -> i32 {
    let c_sender_uuid = CString::new(sender_uuid).expect("sender_uuid contains null byte");
    let c_message_body = CString::new(message_body).expect("message_body contains null byte");
    unsafe {
        ffi::client_private_message_print_messages(
            c_sender_uuid.as_ptr(),
            message_timestamp,
            c_message_body.as_ptr(),
        )
    }
}

pub fn client_error_unknown_team(team_uuid: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    unsafe { ffi::client_error_unknown_team(c_team_uuid.as_ptr()) }
}

pub fn client_error_unknown_channel(channel_uuid: &str) -> i32 {
    let c_channel_uuid = CString::new(channel_uuid).expect("channel_uuid contains null byte");
    unsafe { ffi::client_error_unknown_channel(c_channel_uuid.as_ptr()) }
}

pub fn client_error_unknown_thread(thread_uuid: &str) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    unsafe { ffi::client_error_unknown_thread(c_thread_uuid.as_ptr()) }
}

pub fn client_error_unknown_user(user_uuid: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    unsafe { ffi::client_error_unknown_user(c_user_uuid.as_ptr()) }
}

pub fn client_error_unauthorized() -> i32 {
    unsafe { ffi::client_error_unauthorized() }
}

pub fn client_error_already_exist() -> i32 {
    unsafe { ffi::client_error_already_exist() }
}

pub fn client_print_user(user_uuid: &str, user_name: &str, user_status: i32) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_user_name = CString::new(user_name).expect("user_name contains null byte");
    unsafe { ffi::client_print_user(c_user_uuid.as_ptr(), c_user_name.as_ptr(), user_status) }
}

pub fn client_print_team(team_uuid: &str, team_name: &str, team_description: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_team_name = CString::new(team_name).expect("team_name contains null byte");
    let c_team_description =
        CString::new(team_description).expect("team_description contains null byte");
    unsafe {
        ffi::client_print_team(
            c_team_uuid.as_ptr(),
            c_team_name.as_ptr(),
            c_team_description.as_ptr(),
        )
    }
}

pub fn client_print_channel(
    channel_uuid: &str,
    channel_name: &str,
    channel_description: &str,
) -> i32 {
    let c_channel_uuid = CString::new(channel_uuid).expect("channel_uuid contains null byte");
    let c_channel_name = CString::new(channel_name).expect("channel_name contains null byte");
    let c_channel_description =
        CString::new(channel_description).expect("channel_description contains null byte");
    unsafe {
        ffi::client_print_channel(
            c_channel_uuid.as_ptr(),
            c_channel_name.as_ptr(),
            c_channel_description.as_ptr(),
        )
    }
}

pub fn client_print_thread(
    thread_uuid: &str,
    user_uuid: &str,
    thread_timestamp: i64,
    thread_title: &str,
    thread_body: &str,
) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_thread_title = CString::new(thread_title).expect("thread_title contains null byte");
    let c_thread_body = CString::new(thread_body).expect("thread_body contains null byte");
    unsafe {
        ffi::client_print_thread(
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            thread_timestamp,
            c_thread_title.as_ptr(),
            c_thread_body.as_ptr(),
        )
    }
}

pub fn client_print_team_created(team_uuid: &str, team_name: &str, team_description: &str) -> i32 {
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    let c_team_name = CString::new(team_name).expect("team_name contains null byte");
    let c_team_description =
        CString::new(team_description).expect("team_description contains null byte");
    unsafe {
        ffi::client_print_team_created(
            c_team_uuid.as_ptr(),
            c_team_name.as_ptr(),
            c_team_description.as_ptr(),
        )
    }
}

pub fn client_print_channel_created(
    channel_uuid: &str,
    channel_name: &str,
    channel_description: &str,
) -> i32 {
    let c_channel_uuid = CString::new(channel_uuid).expect("channel_uuid contains null byte");
    let c_channel_name = CString::new(channel_name).expect("channel_name contains null byte");
    let c_channel_description =
        CString::new(channel_description).expect("channel_description contains null byte");
    unsafe {
        ffi::client_print_channel_created(
            c_channel_uuid.as_ptr(),
            c_channel_name.as_ptr(),
            c_channel_description.as_ptr(),
        )
    }
}

pub fn client_print_thread_created(
    thread_uuid: &str,
    user_uuid: &str,
    thread_timestamp: i64,
    thread_title: &str,
    thread_body: &str,
) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_thread_title = CString::new(thread_title).expect("thread_title contains null byte");
    let c_thread_body = CString::new(thread_body).expect("thread_body contains null byte");
    unsafe {
        ffi::client_print_thread_created(
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            thread_timestamp,
            c_thread_title.as_ptr(),
            c_thread_body.as_ptr(),
        )
    }
}

pub fn client_print_reply_created(
    thread_uuid: &str,
    user_uuid: &str,
    reply_timestamp: i64,
    reply_body: &str,
) -> i32 {
    let c_thread_uuid = CString::new(thread_uuid).expect("thread_uuid contains null byte");
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_reply_body = CString::new(reply_body).expect("reply_body contains null byte");
    unsafe {
        ffi::client_print_reply_created(
            c_thread_uuid.as_ptr(),
            c_user_uuid.as_ptr(),
            reply_timestamp,
            c_reply_body.as_ptr(),
        )
    }
}

pub fn client_print_subscribed(user_uuid: &str, team_uuid: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    unsafe { ffi::client_print_subscribed(c_user_uuid.as_ptr(), c_team_uuid.as_ptr()) }
}

pub fn client_print_unsubscribed(user_uuid: &str, team_uuid: &str) -> i32 {
    let c_user_uuid = CString::new(user_uuid).expect("user_uuid contains null byte");
    let c_team_uuid = CString::new(team_uuid).expect("team_uuid contains null byte");
    unsafe { ffi::client_print_unsubscribed(c_user_uuid.as_ptr(), c_team_uuid.as_ptr()) }
}

pub fn client_error_already_exist_safe() -> i32 {
    unsafe { ffi::client_error_already_exist() }
}