#include <criterion/criterion.h>
#include "ECS/Register.hpp"
#include "Commands/FactoryCommands.hpp"
#include "Commands/ACommand.hpp"

using namespace zappy;

Test(FactoryCommandsTest, CreateValidCommands) {
    auto cmdMsz = FactoryCommands::createCommand("msz");
    auto cmdBct = FactoryCommands::createCommand("bct");
    auto cmdInvalid = FactoryCommands::createCommand("comando_inventado");

    cr_assert_not_null(cmdMsz.get());
    cr_assert_not_null(cmdBct.get());
    cr_assert_null(cmdInvalid.get());
}

Test(CommandMapSizeTest, ExecuteValidMapSize) {
    Register registry;
    auto cmd = FactoryCommands::createCommand("msz");

    cr_assert_not_null(cmd.get());

    cmd->execute("20 10", registry);

    cr_assert_eq(registry._sizes.size(), 1);

    auto it = registry._sizes.begin();
    int entityId = it->first;
    auto sizeComponent = it->second;

    cr_assert_eq(sizeComponent.width, 20);
    cr_assert_eq(sizeComponent.height, 10);
    cr_assert(registry._mapTags.find(entityId) != registry._mapTags.end());
}

Test(CommandMapSizeTest, ExecuteInvalidMapSize) {
    Register registry;
    auto cmd = FactoryCommands::createCommand("msz");

    cmd->execute("20 hola", registry);

    cr_assert_eq(registry._sizes.size(), 0);
}

Test(CommandTileContentTest, ExecuteValidTileContent) {
    Register registry;
    auto cmd = FactoryCommands::createCommand("bct");

    cr_assert_not_null(cmd.get());

    cmd->execute("5 4 10 1 2 0 0 0 0 1", registry);

    cr_assert_eq(registry._positions.size(), 1);
    cr_assert_eq(registry._inventories.size(), 1);
    cr_assert_eq(registry._terrainTypes.size(), 1);
    cr_assert_eq(registry._tileTags.size(), 1);

    auto it = registry._positions.begin();
    int entityId = it->first;
    auto pos = it->second;

    cr_assert_eq(pos.x, 5);
    cr_assert_eq(pos.y, 4);

    auto inv = registry._inventories[entityId];
    cr_assert_eq(inv.food, 10);
    cr_assert_eq(inv.linemate, 1);
    cr_assert_eq(inv.deraumere, 2);
    cr_assert_eq(inv.sibur, 0);
    cr_assert_eq(inv.mendiane, 0);
    cr_assert_eq(inv.phiras, 0);
    cr_assert_eq(inv.thystame, 0);

    auto terrain = registry._terrainTypes[entityId];
    cr_assert_eq(terrain.current_type, static_cast<zappy::TerrainType::Type>(1));
}