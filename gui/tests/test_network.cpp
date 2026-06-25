#include "Commands/ACommand.hpp"
#include "Commands/CommandsErrors.hpp"
#include "Commands/FactoryCommands.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Components/ComponentTile.hpp"
#include "ECS/World.hpp"
#include "errors/IError.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(FactoryCommandsTest, CreateValidCommands) {
    // getCommand throws if invalid, so completing this block means success
    auto& cmdMsz = FactoryCommands::getCommand("msz");
    auto& cmdBct = FactoryCommands::getCommand("bct");
    auto& cmdGev = FactoryCommands::getCommand("gev");
    auto& cmdMev = FactoryCommands::getCommand("mev");
}

Test(FactoryCommandsTest, CreateInvalidCommandThrows) {
    try {
        FactoryCommands::getCommand("comando_inventado");
        cr_assert_fail("Expected ErrorProtocol to be thrown");
    } catch (const IError& e) {
        cr_assert_not_null(e.what());
    }
}

Test(CommandMapSizeTest, ExecuteValidMapSize) {
    World world;
    auto& cmd = FactoryCommands::getCommand("msz");

    cmd.execute("20 10", world);

    auto storage = world.get_storage<Size>();
    cr_assert_not_null(storage);

    int count = 0;
    for (auto const& [entity, size] : *storage) {
        cr_assert_eq(size->width, 20);
        cr_assert_eq(size->height, 10);
        cr_assert_not_null(world.get_component<MapTag>(entity));
        count++;
    }
    cr_assert_eq(count, 1);
}

Test(CommandMapSizeTest, ExecuteInvalidMapSize) {
    World world;
    auto& cmd = FactoryCommands::getCommand("msz");

    cmd.execute("20 hola", world);

    auto storage = world.get_storage<Size>();
    if (storage) {
        int count = 0;
        for (auto const& _ : *storage) {
            count++;
        }
        cr_assert_eq(count, 0);
    }
}

Test(CommandTileContentTest, ExecuteValidTileContent) {
    World world;
    auto& cmd = FactoryCommands::getCommand("bct");

    cmd.execute("5 4 10 1 2 0 0 0 0 1", world);

    auto tileStorage = world.get_storage<TileTag>();
    cr_assert_not_null(tileStorage);

    int count = 0;
    for (auto const& [entity, tag] : *tileStorage) {
        auto pos = world.get_component<Position>(entity);
        cr_assert_not_null(pos);
        cr_assert_eq(pos->x, 5);
        cr_assert_eq(pos->y, 4);

        auto inv = world.get_component<Inventory>(entity);
        cr_assert_not_null(inv);
        cr_assert_eq(inv->food, 0); // Delayed until landing
        cr_assert_eq(inv->linemate, 0);

        auto terrain = world.get_component<TerrainType>(entity);
        cr_assert_not_null(terrain);
        cr_assert_eq(terrain->current_type, static_cast<zappy::TerrainType::Type>(1));

        count++;
    }
    cr_assert_eq(count, 1);

    // Verify that the correct number of AnimatedResources were spawned
    auto animStorage = world.get_storage<AnimatedResource>();
    cr_assert_not_null(animStorage);

    int foodCount = 0;
    int linemateCount = 0;
    int deraumereCount = 0;

    for (auto const& [ent, anim] : *animStorage) {
        if (anim->addToTileOnLand) {
            if (anim->resourceId == ResourceType::FOOD) {
                foodCount++;
            } else if (anim->resourceId == ResourceType::LINEMATE) {
                linemateCount++;
            } else if (anim->resourceId == ResourceType::DERAUMERE) {
                deraumereCount++;
            }
        }
    }

    cr_assert_eq(foodCount, 10);
    cr_assert_eq(linemateCount, 1);
    cr_assert_eq(deraumereCount, 2);
}
