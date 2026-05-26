#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EventCode {
    LoggedIn = 600,
    LoggedOut = 601,
    MessageReceived = 602,
    ThreadCreated = 603,
    CommentCreated = 604,
    TeamCreated = 605,
    ChannelCreated = 606,
}
