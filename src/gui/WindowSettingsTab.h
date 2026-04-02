#pragma once

class SettingsUIHelpers;

class WindowSettingsTab
{
public:
    WindowSettingsTab() = delete;

    /**
     * @brief Creates the window/rendering tab renderer.
     */
    explicit WindowSettingsTab(SettingsUIHelpers& helpers);

    /**
     * @brief Preloads temporary values used by controls in this tab.
     */
    void OnShow();

    /**
     * @brief Draws the window/rendering settings tab content.
     */
    void Draw();

private:
    /**
     * @brief Displays a checkbox to override the window startup position, and if this is selected, displays two sliders.
     */
    void WindowPositionSetting();

    SettingsUIHelpers& _helpers;
    float _userScale{1.0F}; //!< Temporary value for UI scale
};
