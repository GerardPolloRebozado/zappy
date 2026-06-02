#include "Network/TcpSocket.hpp"
#include "NetworkManager.hpp"
#include <criterion/criterion.h>

Test(tcp_socket, connection_failure) {
    zappy::TcpSocket socket;
    // Connecting to a likely closed port on localhost
    cr_assert_neq(socket.connect("127.0.0.1", 12345), true,
                  "Socket should not connect to a closed port");
    cr_assert_eq(socket.isConnected(), false,
                 "Socket should be disconnected after failed connection");
}

Test(network_manager, connection_failure) {
    zappy::Register reg;
    zappy::RenderSystem rs;
    zappy::NetworkManager network(reg, rs);
    // Connecting to a likely closed port on localhost
    cr_assert_neq(network.connect("127.0.0.1", 12345), true,
                  "NetworkManager should not connect to a closed port");
    cr_assert_eq(network.isConnected(), false, "NetworkManager should report as disconnected");
}

Test(tcp_socket, initial_state) {
    zappy::TcpSocket socket;
    cr_assert_eq(socket.isConnected(), false, "Socket should be initially disconnected");
}

Test(network_manager, initial_state) {
    zappy::Register reg;
    zappy::RenderSystem rs;
    zappy::NetworkManager network(reg, rs);
    cr_assert_eq(network.isConnected(), false, "NetworkManager should be initially disconnected");
}
