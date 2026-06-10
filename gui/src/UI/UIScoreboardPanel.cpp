/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIScoreboardPanel.cpp
*/

#include "UI/UIScoreboardPanel.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Graphics/AssetManager.hpp"
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace zappy {

UIScoreboardPanel::UIScoreboardPanel(raylib::Rectangle bounds, World& world, int zIndex)
    : AUIComponent(bounds, zIndex), _world(world) {}

void UIScoreboardPanel::render() {
    if (!raylib::Keyboard::IsKeyDown(KEY_TAB)) {
        return;
    }

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float width = 800;
    float height = 600;
    raylib::Rectangle panelBounds = {(sw - width) / 2.0f, (sh - height) / 2.0f, width, height};

    panelBounds.Draw(raylib::Color(15, 20, 40, 210));
    panelBounds.DrawLines(raylib::Color(0, 100, 255, 255), 3.0f);

    raylib::Text title("SCOREBOARD", 30, raylib::Color(0, 150, 255, 255),
                       AssetManager::getInstance().getFont("BoldPixels"), 1.5f);
    float titleWidth = title.MeasureEx().x;
    title.Draw(panelBounds.x + (panelBounds.width - titleWidth) / 2.0f, panelBounds.y + 20);

    struct TeamInfo {
        int maxLevel = 1;
        std::vector<std::string> players;
    };
    std::map<std::string, TeamInfo> teamData;

    auto teamStorage = _world.get_storage<TeamName>();
    if (teamStorage) {
        for (auto const& [entity, team] : *teamStorage) {
            int lvl = 1;
            auto levelComp = _world.get_component<Level>(entity);
            if (levelComp) {
                lvl = levelComp->level;
            }

            teamData[team->team_name].maxLevel = std::max(teamData[team->team_name].maxLevel, lvl);
            auto serverId = _world.get_component<ServerId>(entity);
            std::string idStr =
                serverId ? std::to_string(serverId->id) : std::to_string(entity.id());
            teamData[team->team_name].players.push_back("P" + idStr + "(Lvl " +
                                                        std::to_string(lvl) + ")");
        }
    }

    float yOffset = panelBounds.y + 80;
    float col1 = panelBounds.x + 30;
    float col2 = panelBounds.x + 250;
    float col3 = panelBounds.x + 350;
    float col4 = panelBounds.x + 450;

    if (teamData.empty()) {
        raylib::Text("No players connected.", 20, GRAY,
                     AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
            .Draw(panelBounds.x + 30, yOffset);
        return;
    }

    // Headers
    raylib::Text("Team Name", 20, raylib::Color(0, 150, 255, 255),
                 AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
        .Draw(col1, yOffset);
    raylib::Text("Count", 20, raylib::Color(0, 150, 255, 255),
                 AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
        .Draw(col2, yOffset);
    raylib::Text("Max Lvl", 20, raylib::Color(0, 150, 255, 255),
                 AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
        .Draw(col3, yOffset);

    yOffset += 30;
    ::DrawLineEx(raylib::Vector2{panelBounds.x + 20, yOffset},
                 raylib::Vector2{panelBounds.x + panelBounds.width - 20, yOffset}, 2.0f,
                 raylib::Color(0, 100, 255, 255));
    yOffset += 15;

    // Sort teams by size
    std::vector<std::pair<std::string, TeamInfo>> sortedTeams(teamData.begin(), teamData.end());
    std::sort(sortedTeams.begin(), sortedTeams.end(), [](const auto& a, const auto& b) {
        return a.second.players.size() > b.second.players.size();
    });

    std::vector<raylib::Color> rowColors = {
        raylib::Color(230, 60, 60, 60),  // Soft Red
        raylib::Color(60, 230, 60, 60),  // Soft Green
        raylib::Color(60, 100, 230, 60), // Soft Blue
        raylib::Color(230, 230, 60, 60), // Soft Yellow
        raylib::Color(230, 60, 230, 60), // Soft Magenta
        raylib::Color(60, 230, 230, 60)  // Soft Cyan
    };

    int colorIndex = 0;
    for (const auto& [teamName, info] : sortedTeams) {
        float blockHeight = 30 + (info.players.size() * 25);

        if (yOffset > panelBounds.y + panelBounds.height - 40) {
            raylib::Text("... (more teams)", 16, GRAY,
                         AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
                .Draw(col1, yOffset);
            break;
        }

        raylib::Color rowBg = rowColors[colorIndex % rowColors.size()];
        raylib::Rectangle rowRect = {panelBounds.x + 10, yOffset - 5, panelBounds.width - 20,
                                     blockHeight};
        rowRect.Draw(rowBg);

        raylib::Text(teamName, 18, raylib::Color::RayWhite(),
                     AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
            .Draw(col1, yOffset);
        raylib::Text(std::to_string(info.players.size()), 18, raylib::Color::RayWhite(),
                     AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
            .Draw(col2, yOffset);
        raylib::Text(std::to_string(info.maxLevel), 18, raylib::Color::RayWhite(),
                     AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
            .Draw(col3, yOffset);

        yOffset += 30;

        for (const auto& p : info.players) {
            if (yOffset > panelBounds.y + panelBounds.height - 40) {
                raylib::Text("  ... (more players)", 16, GRAY,
                             AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
                    .Draw(col1 + 30, yOffset);
                yOffset += 25;
                break;
            }
            raylib::Text("  - " + p, 16, raylib::Color::LightGray(),
                         AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
                .Draw(col1 + 30, yOffset);
            yOffset += 25;
        }

        colorIndex++;
    }
}

} // namespace zappy
