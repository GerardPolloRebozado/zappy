#import "template.typ": rfc

#let name = "MyTeams"
#let group = "Rewriting the World in Rust"
#let groupAbrev = "RWR"
#let completeGroup = group + " (" + groupAbrev + ")"

#show: rfc.with(
  title: name,
  group: completeGroup,
  authors: ("A. Laguna", "G. Du Pre"),
  rfc-number: "4242",
  category: "Communication",
  date: "18 April 2026",
)

= Abstract
This document describes the protocol and architecture of #name, a text-based communication protocol that enables real-time messaging between users. The protocol supports group messaging via teams, channels, and threads, as well as private messaging. This document defines the message formats, stateful context management, and the asynchronous event system of the #name protocol.

= Status of this memo
This document is a product of the #completeGroup. It represents the consensus of the #group community. It is the official specification of the #name protocol. Distribution of this memo is unlimited.

= Copyright Notice
Copyright (c) 2026 European Institute of Technology and the persons identified as the document authors. All rights reserved.

= Introduction
#name is a text-based communication protocol operating over TCP. It is designed to be easy to implement while providing a rich set of features for collaboration. The protocol uses a client-server architecture where the server manages the global state (users, teams, messages) and broadcasts updates to connected clients.

= Conventions used in this document
The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

= Protocol Overview
The protocol is line-oriented and UTF-8 encoded. Each message is terminated by a newline character (`\n`).

== Message Format

=== Request Format
Requests consist of a command name followed by optional arguments. Arguments containing spaces or special characters MUST be enclosed in double quotes and escaped using a backslash (`\`).
Syntax: `COMMAND_NAME ["arg1" ["arg2" ...]]`

=== Response Format
Responses consist of a three-digit code followed by an optional data payload. The data payload often contains space-separated quoted strings.
Syntax: `CODE [DATA]`

== Status Codes
- `200 OK`: Command executed successfully.
- `201 CREATED`: Resource created successfully.
- `400 BAD_REQUEST`: Invalid syntax or arguments.
- `401 UNAUTHORIZED`: Authentication required.
- `403 FORBIDDEN`: Permission denied.
- `404 NOT_FOUND`: Resource not found.
- `409 CONFLICT`: Resource already exists.
- `442 I'M A COFFEE POT`: Server refuses to brew tea.
- `500 INTERNAL_SERVER_ERROR`: Unexpected server error.

== Asynchronous Events (6xx)
Events are pushed by the server to notify clients of external actions.
- `600 LOGGED_IN`: A user logged in. Data: `"user_uuid" "user_name"`
- `601 LOGGED_OUT`: A user logged out. Data: `"user_uuid" "user_name"`
- `602 MESSAGE_RECEIVED`: Private message received. Data: `"sender_uuid" "body"`
- `603 THREAD_CREATED`: New thread created. Data: `"thread_uuid" "user_uuid" "timestamp" "title" "body"`
- `604 COMMENT_CREATED`: New reply created. Data: `"team_uuid" "thread_uuid" "user_uuid" "body"`
- `605 TEAM_CREATED`: New team created. Data: `"team_uuid" "team_name" "team_description"`
- `606 CHANNEL_CREATED`: New channel created. Data: `"channel_uuid" "channel_name" "channel_description"`

= Resource Hierarchy and Messaging
The protocol enforces a strict hierarchical structure for group communication and a separate path for private communication.

== Resource Hierarchy
- **Team**: The top-level container for collaboration.
- **Channel**: Created within a Team to group related Threads. A Team MUST support multiple Channels.
- **Thread**: Created within a Channel. It represents a specific discussion topic. A Channel MUST support multiple Threads.
- **Reply**: A message sent within a Thread.

== Messaging Types
- **Private Messaging**: Direct 1-to-1 communication between users, independent of the Team/Channel hierarchy. Handled via the `SEND` and `MESSAGES` commands.
- **Threaded Messaging**: Messages (Replies) sent within the context of a specific Thread. To send a reply, the user MUST be in a Thread context (inside a Channel, inside a Team).

= Commands

== Authentication
- `LOGIN "user_name"`: Authenticates or creates a user. Success: `200 "user_uuid" "user_name"`.
- `LOGOUT`: Disconnects the user. Success: Connection is closed by the server.

== User Information
- `USERS`: Lists all users. Success: `200 ["user_uuid" "user_name" "status"]*`. Status is 1 (online) or 0 (offline).
- `USER "user_uuid"`: Gets user details. Success: `200 "user_uuid" "user_name" "status"`.

== Messaging
- `SEND "user_uuid" "body"`: Sends a private message. Success: `200`.
- `MESSAGES "user_uuid"`: Lists private messages. Success: `200 ["sender_uuid" "timestamp" "body"]*`.

== Subscriptions
- `SUBSCRIBE "team_uuid"`: Subscribes to a team. Success: `200 "user_uuid" "team_uuid"`.
- `UNSUBSCRIBE "team_uuid"`: Unsubscribes from a team. Success: `200 "user_uuid" "team_uuid"`.
- `SUBSCRIBED`: Lists teams subscribed to. Success: `200 ["team_uuid" "team_name" "team_description"]*`.
- `SUBSCRIBED "team_uuid"`: Lists users in a team. Success: `200 ["user_uuid" "user_name" "status"]*`.

== Context and Resource Management
- `USE ["team_uuid" ["channel_uuid" ["thread_uuid"]]]`: Sets the current scope for `CREATE`, `LIST`, and `INFO`.

=== CREATE
Creates a resource in the current context:
- Global: `CREATE "name" "desc"` -> `200 TEAM "uuid" "name" "desc"`
- Team: `CREATE "name" "desc"` -> `200 CHANNEL "uuid" "name" "desc"`
- Channel: `CREATE "title" "body"` -> `200 THREAD "uuid" "user_uuid" "ts" "title" "body"`
- Thread: `CREATE "body"` -> `200 REPLY "thread_uuid" "user_uuid" "ts" "body"`

=== LIST
Lists items in the current context:
- Global: `200 TEAM ["uuid" "name" "desc"]*`
- Team: `200 CHANNEL ["uuid" "name" "desc"]*`
- Channel: `200 THREAD ["uuid" "user_uuid" "ts" "title" "body"]*`
- Thread: `200 REPLY ["thread_uuid" "user_uuid" "ts" "body"]*`

=== INFO
Gets details of the current resource:
- Global: `200 USER "uuid" "name" "status"`
- Team: `200 TEAM "uuid" "name" "desc"`
- Channel: `200 CHANNEL "uuid" "name" "desc"`
- Thread: `200 THREAD "uuid" "user_uuid" "ts" "title" "body"`

= Constraints
- Max Name: 32 chars.
- Max Description: 255 chars.
- Max Body: 512 chars.

= Authors' Addresses
A. Laguna \
European Institute of Technology \
Email: alejandro.laguna\@epitech.eu

G. Du Pre \
European Institute of Technology \
Email: gerard.du-pre\@epitech.eu
