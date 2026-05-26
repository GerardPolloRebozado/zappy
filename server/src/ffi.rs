use std::ffi::{c_char, c_int};

// time_t is i64 on 64-bit linux
#[allow(non_camel_case_types)] // be consistent with C type
pub type time_t = i64;

#[link(name = "myteams")]
unsafe extern "C" {
    /*
     * logging_server.h bindings
     */
    pub fn server_event_team_created(
        team_uuid: *const c_char,
        team_name: *const c_char,
        user_uuid: *const c_char,
    ) -> c_int;

    pub fn server_event_channel_created(
        team_uuid: *const c_char,
        channel_uuid: *const c_char,
        channel_name: *const c_char,
    ) -> c_int;

    pub fn server_event_thread_created(
        channel_uuid: *const c_char,
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        thread_title: *const c_char,
        thread_body: *const c_char,
    ) -> c_int;

    pub fn server_event_reply_created(
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        reply_body: *const c_char,
    ) -> c_int;

    pub fn server_event_user_subscribed(
        team_uuid: *const c_char,
        user_uuid: *const c_char,
    ) -> c_int;
    pub fn server_event_user_unsubscribed(
        team_uuid: *const c_char,
        user_uuid: *const c_char,
    ) -> c_int;
    pub fn server_event_user_created(user_uuid: *const c_char, user_name: *const c_char) -> c_int;
    pub fn server_event_user_loaded(user_uuid: *const c_char, user_name: *const c_char) -> c_int;
    pub fn server_event_user_logged_in(user_uuid: *const c_char) -> c_int;
    pub fn server_event_user_logged_out(user_uuid: *const c_char) -> c_int;
    pub fn server_event_private_message_sended(
        sender_uuid: *const c_char,
        receiver_uuid: *const c_char,
        message_body: *const c_char,
    ) -> c_int;

    /*
     * logging_client.h bindings
     */
    pub fn client_event_logged_in(user_uuid: *const c_char, user_name: *const c_char) -> c_int;
    pub fn client_event_logged_out(user_uuid: *const c_char, user_name: *const c_char) -> c_int;
    pub fn client_event_private_message_received(
        user_uuid: *const c_char,
        message_body: *const c_char,
    ) -> c_int;
    pub fn client_event_thread_reply_received(
        team_uuid: *const c_char,
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        reply_body: *const c_char,
    ) -> c_int;
    pub fn client_event_team_created(
        team_uuid: *const c_char,
        team_name: *const c_char,
        team_description: *const c_char,
    ) -> c_int;
    pub fn client_event_channel_created(
        channel_uuid: *const c_char,
        channel_name: *const c_char,
        channel_description: *const c_char,
    ) -> c_int;
    pub fn client_event_thread_created(
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        thread_timestamp: time_t,
        thread_title: *const c_char,
        thread_body: *const c_char,
    ) -> c_int;
    pub fn client_print_users(
        user_uuid: *const c_char,
        user_name: *const c_char,
        user_status: c_int,
    ) -> c_int;
    pub fn client_print_teams(
        team_uuid: *const c_char,
        team_name: *const c_char,
        team_description: *const c_char,
    ) -> c_int;
    pub fn client_team_print_channels(
        channel_uuid: *const c_char,
        channel_name: *const c_char,
        channel_description: *const c_char,
    ) -> c_int;
    pub fn client_channel_print_threads(
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        thread_timestamp: time_t,
        thread_title: *const c_char,
        thread_body: *const c_char,
    ) -> c_int;
    pub fn client_thread_print_replies(
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        reply_timestamp: time_t,
        reply_body: *const c_char,
    ) -> c_int;
    pub fn client_private_message_print_messages(
        sender_uuid: *const c_char,
        message_timestamp: time_t,
        message_body: *const c_char,
    ) -> c_int;
    pub fn client_error_unknown_team(team_uuid: *const c_char) -> c_int;
    pub fn client_error_unknown_channel(channel_uuid: *const c_char) -> c_int;
    pub fn client_error_unknown_thread(thread_uuid: *const c_char) -> c_int;
    pub fn client_error_unknown_user(user_uuid: *const c_char) -> c_int;
    pub fn client_error_unauthorized() -> c_int;
    pub fn client_error_already_exist() -> c_int;
    pub fn client_print_user(
        user_uuid: *const c_char,
        user_name: *const c_char,
        user_status: c_int,
    ) -> c_int;
    pub fn client_print_team(
        team_uuid: *const c_char,
        team_name: *const c_char,
        team_description: *const c_char,
    ) -> c_int;
    pub fn client_print_channel(
        channel_uuid: *const c_char,
        channel_name: *const c_char,
        channel_description: *const c_char,
    ) -> c_int;
    pub fn client_print_thread(
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        thread_timestamp: time_t,
        thread_title: *const c_char,
        thread_body: *const c_char,
    ) -> c_int;
    pub fn client_print_team_created(
        team_uuid: *const c_char,
        team_name: *const c_char,
        team_description: *const c_char,
    ) -> c_int;
    pub fn client_print_channel_created(
        channel_uuid: *const c_char,
        channel_name: *const c_char,
        channel_description: *const c_char,
    ) -> c_int;
    pub fn client_print_thread_created(
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        thread_timestamp: time_t,
        thread_title: *const c_char,
        thread_body: *const c_char,
    ) -> c_int;
    pub fn client_print_reply_created(
        thread_uuid: *const c_char,
        user_uuid: *const c_char,
        reply_timestamp: time_t,
        reply_body: *const c_char,
    ) -> c_int;
    pub fn client_print_subscribed(user_uuid: *const c_char, team_uuid: *const c_char) -> c_int;
    pub fn client_print_unsubscribed(user_uuid: *const c_char, team_uuid: *const c_char) -> c_int;
}
