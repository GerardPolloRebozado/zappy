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
#include <string>
#include <vector>

namespace zappy {

UIScoreboardPanel::UIScoreboardPanel(raylib::Rectangle bounds, World& world, int zIndex)
    : AUIComponent(bounds, zIndex), _world(world) {}

void UIScoreboardPanel::render() {
    _bounds.Draw(raylib::Color(DARKGRAY).Fade(0.85f));
    _bounds.DrawLines(RAYWHITE, 2.0f);

    raylib::Text("SCOREBOARD", 22, RAYWHITE).Draw(_bounds.x + 15, _bounds.y + 15);

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
        raylib::Text("No players connected.", 16, GRAY).Draw(_bounds.x + 15, (float)yOffset);
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

        raylib::Text(tName, 18, SKYBLUE, GetFontDefault(), 1.5f)
            .Draw(_bounds.x + 15, (float)yOffset);
        yOffset += 20;
        raylib::Text(stats, 14, LIGHTGRAY, GetFontDefault(), 1.5f)
            .Draw(_bounds.x + 25, (float)yOffset);
        yOffset += 25;
    }
}

} // namespace zappy