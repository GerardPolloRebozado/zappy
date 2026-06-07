/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIScoreboardPanel.cpp
*/

#include "UI/UIScoreboardPanel.hpp"
#include "Components/ComponentInhabitant.hpp"
#include <algorithm>
#include <map>
#include <raylib.h>
#include <string>
#include <vector>

namespace zappy {

UIScoreboardPanel::UIScoreboardPanel(raylib::Rectangle bounds, World& world, int zIndex)
    : AUIComponent(bounds, zIndex), _world(world) {}

void UIScoreboardPanel::render() {
    DrawRectangleRec(_bounds, Fade(DARKGRAY, 0.85f));
    DrawRectangleLinesEx(_bounds, 2.0f, RAYWHITE);

    ::DrawText("SCOREBOARD", _bounds.x + 15, _bounds.y + 15, 22, RAYWHITE);

    // Collect team sizes and max levels
    std::map<std::string, int> teamSizes;
    std::map<std::string, int> teamMaxLevel;

    auto teamStorage = _world.get_storage<TeamName>();
    if (teamStorage) {
        for (auto const& [entity, team] : *teamStorage) {
            teamSizes[team->team_name]++;

            int lvl = 1;
            auto levelComp = _world.get_component<Level>(entity);
            if (levelComp) {
                lvl = levelComp->level;
            }

            if (lvl > teamMaxLevel[team->team_name]) {
                teamMaxLevel[team->team_name] = lvl;
            }
        }
    }

    int yOffset = _bounds.y + 50;

    if (teamSizes.empty()) {
        ::DrawText("No players connected.", _bounds.x + 15, yOffset, 16, GRAY);
        return;
    }

    // Sort by size
    std::vector<std::pair<std::string, int>> sortedTeams(teamSizes.begin(), teamSizes.end());
    std::sort(sortedTeams.begin(), sortedTeams.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (const auto& [teamName, size] : sortedTeams) {
        std::string tName = teamName;
        std::string stats = "Players: " + std::to_string(size) +
                            " | Max Lvl: " + std::to_string(teamMaxLevel[teamName]);

        ::DrawText(tName.c_str(), _bounds.x + 15, yOffset, 18, SKYBLUE);
        yOffset += 20;
        ::DrawText(stats.c_str(), _bounds.x + 25, yOffset, 14, LIGHTGRAY);
        yOffset += 25;
    }
}

} // namespace zappy