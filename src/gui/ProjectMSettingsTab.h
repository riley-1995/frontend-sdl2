#pragma once

class SettingsUIHelpers;

class ProjectMSettingsTab
{
public:
    ProjectMSettingsTab() = delete;
    explicit ProjectMSettingsTab(SettingsUIHelpers& helpers);

    void Draw();

private:
    SettingsUIHelpers& _helpers;
};
