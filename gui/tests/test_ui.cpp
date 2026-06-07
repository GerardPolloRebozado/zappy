#include "Graphics/AssetManager.hpp"
#include "UI/UIButton.hpp"
#include "UI/UIInput.hpp"
#include "UI/UIManager.hpp"
#include "UI/UIPanel.hpp"
#include "UI/UIText.hpp"
#include <criterion/criterion.h>
#include <raylib.h>

using namespace zappy;

// initialize a headless window for testing GPU-dependent logic
void setup_ui_tests() {
    SetTraceLogLevel(LOG_NONE); // Keep output clean
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(100, 100, "Headless UI Test");
}

void teardown_ui_tests() {
    zappy::AssetManager::getInstance().unloadAll();
    CloseWindow();
}

Test(ui_component, basic_properties, .init = setup_ui_tests, .fini = teardown_ui_tests) {
    UIPanel panel({10, 20, 100, 50}, raylib::Color::Red(), 5);

    // Test Bounds
    raylib::Rectangle bounds = panel.getBounds();
    cr_assert_eq(bounds.x, 10);
    cr_assert_eq(bounds.y, 20);
    cr_assert_eq(bounds.width, 100);
    cr_assert_eq(bounds.height, 50);

    panel.setBounds({0, 0, 10, 10});
    bounds = panel.getBounds();
    cr_assert_eq(bounds.width, 10);

    // Test Z-Index
    cr_assert_eq(panel.getZIndex(), 5);
    panel.setZIndex(10);
    cr_assert_eq(panel.getZIndex(), 10);

    // Test Visibility
    cr_assert_eq(panel.isVisible(), true, "Component should be visible by default");
    panel.setVisible(false);
    cr_assert_eq(panel.isVisible(), false);
}

Test(ui_input, default_state, .init = setup_ui_tests, .fini = teardown_ui_tests) {
    UIInput input({0, 0, 100, 20}, "Test", "Placeholder", 256, 0);

    cr_assert_eq(input.getText(), "Test");
}

Test(ui_manager, manage_components, .init = setup_ui_tests, .fini = teardown_ui_tests) {
    UIManager manager;

    auto panel1 =
        std::make_shared<UIPanel>(raylib::Rectangle{0, 0, 10, 10}, raylib::Color::Red(), 1);
    auto panel2 =
        std::make_shared<UIPanel>(raylib::Rectangle{0, 0, 10, 10}, raylib::Color::Blue(), 2);

    manager.addComponent(panel1);
    manager.addComponent(panel2);

    // Not crashing on update/render is a basic health check
    manager.update(0.16f);

    manager.clear();
    // After clear, update should not trigger anything, preventing memory leaks or use-after-free
    manager.update(0.16f);
    cr_assert(true);
}
