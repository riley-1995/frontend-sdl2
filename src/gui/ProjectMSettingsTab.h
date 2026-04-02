#pragma once

class SettingsUIHelpers;

class ProjectMSettingsTab
{
public:
    ProjectMSettingsTab() = delete;

    /**
     * @brief Creates the projectM tab renderer using shared settings UI helpers.
     */
    explicit ProjectMSettingsTab(SettingsUIHelpers& helpers);

    /**
     * @brief Draws the projectM settings tab content.
     */
    void Draw();

private:
    SettingsUIHelpers& _helpers;
};
