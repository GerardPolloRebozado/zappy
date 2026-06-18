use crate::ecs::components::network::NetworkData;
use crate::protocol::Request;
use crate::server::Server;
use crate::utils::Config;
use std::ffi::{CStr, CString};
use std::os::raw::c_char;

#[unsafe(no_mangle)]
pub extern "C" fn zappy_init(
    width: u32,
    height: u32,
    freq: u32,
    team_names: *const *const c_char,
    team_count: usize,
    clients_nb: u32,
) -> *mut Server {
    let mut names = Vec::new();
    for i in 0..team_count {
        let c_str = unsafe { CStr::from_ptr(*team_names.add(i)) };
        names.push(c_str.to_string_lossy().into_owned());
    }

    let config = Config {
        port: 0, // Not used in headless mode
        width,
        height,
        names,
        clients_nb,
        freq,
    };

    let mut server = Server::new(config);
    server.load();
    Box::into_raw(Box::new(server))
}

#[unsafe(no_mangle)]
pub extern "C" fn zappy_free(server: *mut Server) {
    if !server.is_null() {
        unsafe {
            let _ = Box::from_raw(server);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn zappy_tick(server: *mut Server, delta_ms: u64) {
    let server = unsafe { &mut *server };
    server.world.current_time += delta_ms;
    crate::ecs::systems::run::run_systems(&mut server.world);
}

#[unsafe(no_mangle)]
pub extern "C" fn zappy_add_player(server: *mut Server, team_name: *const c_char) -> u32 {
    let server = unsafe { &mut *server };
    let c_str = unsafe { CStr::from_ptr(team_name) };
    let team_name = c_str.to_string_lossy().into_owned();

    let entity = server.world.spawn();
    server
        .world
        .add_component(entity, NetworkData::new_headless());
    server.world.add_component(
        entity,
        crate::ecs::components::team::Team::WaitingForTeamName,
    );

    let req = format!("{}\n", team_name).parse::<Request>().unwrap();
    crate::commands::handle_request(server, entity, req);

    entity.id()
}

#[unsafe(no_mangle)]
pub extern "C" fn zappy_send_command(server: *mut Server, player_id: u32, command: *const c_char) {
    let server = unsafe { &mut *server };
    let c_str = unsafe { CStr::from_ptr(command) };
    let cmd_str = c_str.to_string_lossy().into_owned();

    let entity = crate::ecs::storage::Entity::from_id(player_id, &server.world);
    if let Some(entity) = entity {
        if let Ok(req) = cmd_str.parse::<Request>() {
            crate::commands::handle_request(server, entity, req);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn zappy_get_response(server: *mut Server, player_id: u32) -> *mut c_char {
    let server = unsafe { &mut *server };
    let entity = crate::ecs::storage::Entity::from_id(player_id, &server.world);

    if let Some(entity) = entity {
        if let Some(nd) = server.world.get_component_mut::<NetworkData>(entity) {
            if !nd.pending_responses.is_empty() {
                let resp = nd.pending_responses.remove(0);
                let c_string = CString::new(resp.to_string()).unwrap();
                return c_string.into_raw();
            }
        }
    }
    std::ptr::null_mut()
}

#[unsafe(no_mangle)]
pub extern "C" fn zappy_free_string(s: *mut c_char) {
    if !s.is_null() {
        unsafe {
            let _ = CString::from_raw(s);
        }
    }
}
