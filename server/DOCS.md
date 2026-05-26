# MyTeams Project Documentation

## Part 1: Protocol Specification (RFC Style)

### 1. Introduction
This document defines the application-layer protocol used for communication between the `myteams_server` and `myteams_cli`. The protocol is designed to be stateful, asynchronous, and handled by a single-threaded event loop.

### 2. Message Format
All messages exchanged between the client and server shall follow this structure:
* **Command/Status Header:** ???
* **Payload Length:** ???
* **Payload:** ???

### 3. Client-to-Server Commands
| Command | Arguments | Description |
| :--- | :--- | :--- |
| `LOGIN` | `["user_name"]` | Initiates a session for the user. |
| `SUBSCRIBE` | `["team_uuid"]` | Subscribes the user to a specific team. |
| `CREATE` | `[context_args] ["name"] ["desc"]` | Creates a resource based on current `/use` context. |
| `SEND` | `["user_uuid"] ["body"]` | Sends a personal message to another user. |

### 4. Server-to-Client Responses
* **Success Codes (2xx):** Action completed successfully (e.g., `201 Created`).
* **Error Codes (4xx/5xx):** Handled via standard output in the CLI.
* **??? Async Events:** Pushed to the client when a subscribed team has activity.

---

## Part 2: Data Model Specification

### 1. Global Constraints
The following field lengths are strictly enforced across all entities:
* **MAX_NAME_LENGTH:** ???
* **MAX_DESCRIPTION_LENGTH:** ???
* **MAX_BODY_LENGTH:** ???

### 2. Entity Definitions

#### A. User
* **UUID:**
* **Username:** String (Max 32 chars).
* **Status:** Online/Offline.

#### B. Team
* **UUID:** Unique identifier.
* **Name:** String (Max 32 chars).
* **Description:** String (Max 255 chars).
* **Subscribers:** List of User UUIDs.

#### C. Channel (Parent: Team)
* **UUID:** Unique identifier.
* **Name:** String (Max 32 chars).
* **Description:** String (Max 255 chars).

#### D. Thread (Parent: Channel)
* **UUID:** Unique identifier.
* **Title:** String (Max 32 chars).
* **Message:** String (Max 512 chars).
* **Author:** User UUID.
* **Timestamp:** Creation time.

#### E. Comment (Parent: Thread)
* **Body:** String (Max 512 chars).
* **Author:** User UUID.

### 3. Persistence Strategy
The server must implement a save/load mechanism:
* **Restoration:** On startup, the server checks for existing save files to rebuild the internal data structures.
