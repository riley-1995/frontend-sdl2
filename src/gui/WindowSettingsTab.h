#pragma once

class SettingsUIHelpers;

class WindowSettingsTab
{
public:
    WindowSettingsTab() = delete;
    explicit WindowSettingsTab(SettingsUIHelpers& helpers);

    void OnShow();
    void Draw();

private:
    void WindowPositionSetting();

    SettingsUIHelpers& _helpers;
    float _userScale{1.0F};
};
