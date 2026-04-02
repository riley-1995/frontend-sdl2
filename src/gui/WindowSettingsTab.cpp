#include "WindowSettingsTab.h"

#include "SDLRenderingWindow.h"
#include "SettingsConfigKeys.h"
#include "SettingsUIHelpers.h"

#include <imgui.h>

#include <Poco/Util/Application.h>

using namespace SettingsConfigKeys;

namespace
{
constexpr int kSettingsTableColumnCount = 5;
constexpr float kChooseColumnWidth = 100.0F;
constexpr float kResetColumnWidth = 50.0F;

constexpr int kDefaultWindowWidth = 1920;
constexpr int kDefaultWindowHeight = 1080;
constexpr int kWindowSizeMin = 32;
// 8192 mirrors SDL/UI safety bounds for extreme resolutions and large desktops.
constexpr int kWindowSizeMax = 8192;
constexpr int kDefaultMonitor = 0;
constexpr int kMonitorMin = 0;
constexpr int kMonitorMax = 10;
constexpr int kFullscreenMin = 240;
constexpr int kDefaultFps = 60;
constexpr int kFpsMin = 0;
constexpr int kFpsMax = 300;

constexpr double kUiScaleDefault = 1.0;
constexpr double kUiScaleMin = 0.1;
constexpr double kUiScaleMax = 3.0;

constexpr int kWindowPositionLimit = 8192;
} // namespace

WindowSettingsTab::WindowSettingsTab(SettingsUIHelpers& helpers)
    : _helpers(helpers)
{
}

void WindowSettingsTab::OnShow()
{
    _userScale = static_cast<float>(_helpers.userConfiguration()->getDouble(kConfigWindowUiScale, kUiScaleDefault));
}

void WindowSettingsTab::Draw()
{
    if (ImGui::BeginTabItem("Window / Rendering"))
    {
        if (ImGui::BeginTable("projectM", kSettingsTableColumnCount, ImGuiTableFlags_None))
        {
            ImGui::TableSetupColumn("##desc", ImGuiTableColumnFlags_WidthFixed, .0f);
            ImGui::TableSetupColumn("##setting", ImGuiTableColumnFlags_WidthStretch, .0f);
            ImGui::TableSetupColumn("##choose", ImGuiTableColumnFlags_WidthFixed, kChooseColumnWidth);
            ImGui::TableSetupColumn("##reset", ImGuiTableColumnFlags_WidthFixed, kResetColumnWidth);
            ImGui::TableSetupColumn("##override", ImGuiTableColumnFlags_WidthFixed, .0f);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Startup Window Placement",
                                      "Initial window placement options when starting projectM.\nClick the button to copy the current window size, position and display to the settings.");
            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button("Use Current Size and Position"))
            {
                auto& renderingWindow = Poco::Util::Application::instance().getSubsystem<SDLRenderingWindow>();
                int x;
                int y;

                renderingWindow.GetWindowSize(x, y);
                _helpers.userConfiguration()->setInt(kConfigWindowWidth, x);
                _helpers.userConfiguration()->setInt(kConfigWindowHeight, y);

                renderingWindow.GetWindowPosition(x, y, true);
                _helpers.userConfiguration()->setInt(kConfigWindowLeft, x);
                _helpers.userConfiguration()->setInt(kConfigWindowTop, y);

                _helpers.userConfiguration()->setBool(kConfigWindowOverridePosition, true);
                _helpers.userConfiguration()->setInt(kConfigWindowMonitor, renderingWindow.GetCurrentDisplay() + 1);
            }

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Window Size",
                                      "Initial window size when starting projectM.\nThis might or might not include the window decoration, depending on the OS.");
            _helpers.IntegerSettingVec(kConfigWindowWidth, kConfigWindowHeight, kDefaultWindowWidth, kDefaultWindowHeight,
                                       kWindowSizeMin, kWindowSizeMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Window Position",
                                      "Initial window position when starting projectM.\nThe coordinates are relative to the current monitor.");
            WindowPositionSetting();

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Monitor",
                                      "Use 0 to let the OS select the monitor, or any positive number to select a specific monitor.\nIf the number is larger than the number of connected monitors, the last available one will be used.");
            _helpers.IntegerSetting(kConfigWindowMonitor, kDefaultMonitor, kMonitorMin, kMonitorMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Borderless Window",
                                      "Don't display the window border and title bar if the OS supports it.\nCan make the window immovable.");
            _helpers.BooleanSetting(kConfigWindowBorderless, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Start Fullscreen", "Start projectM in fullscreen mode");
            _helpers.BooleanSetting(kConfigWindowFullscreen, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Exclusive Fullscreen Mode",
                                      "Use exclusive mode if fullscreen, e.g. not as a borderless window.\nThis can improve performance, but may switch the desktop resolution!");
            _helpers.BooleanSetting(kConfigWindowFullscreenExclusive, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Exclusive Mode Resolution",
                                      "Resolution to change to in exclusive fullscreen mode.\nNot all graphics driver support arbitrary resolution and will use the next-best supported one.");
            _helpers.IntegerSettingVec(kConfigFullscreenWidth, kConfigFullscreenHeight, kDefaultWindowWidth, kDefaultWindowHeight,
                                       kFullscreenMin, kWindowSizeMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Target FPS",
                                      "Limit frames rendered per second to the given FPS value.\nNOTE: A value of 0 will NOT limit FPS and render at either VSync or unlimited pace, possibly using all CPU/GPU resources.");
            _helpers.IntegerSetting(kConfigProjectMFps, kDefaultFps, kFpsMin, kFpsMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Wait for Vertical Sync",
                                      "Wait for vertical sync interval before displaying the next frame.\nThis will limit max FPS to the vertical sync frequency but prevents tearing.");
            _helpers.BooleanSetting(kConfigWindowWaitForVerticalSync, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Use Adaptive Sync",
                                      "Tries to use adaptive vertical sync if vertical sync is enabled.\nWhen using a monitor capable of adaptive sync, setting FPS to 0 gives the best results.");
            _helpers.BooleanSetting(kConfigWindowAdaptiveVerticalSync, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Preset Name in Title",
                                      "Controls displaying the current preset name after the application name in the window title.");
            _helpers.BooleanSetting(kConfigWindowDisplayPresetNameInTitle, true);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Display Toast Messages",
                                      "Controls displaying toast messages/notifications, e.g. when changing the audio device.");
            _helpers.BooleanSetting(kConfigProjectMDisplayToasts, true);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("UI Scaling Factor", "Multiplies the default UI/font size with the given factor.");
            _helpers.DoubleSettingWithApply(kConfigWindowUiScale, kUiScaleDefault, kUiScaleMin, kUiScaleMax, _userScale);

            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
}

void WindowSettingsTab::WindowPositionSetting()
{
    ImGui::TableSetColumnIndex(1);

    bool positionIsOverridden = _helpers.userConfiguration()->getBool(kConfigWindowOverridePosition, false);

    if (ImGui::Checkbox("##window_set_pos", &positionIsOverridden))
    {
        _helpers.userConfiguration()->setBool(kConfigWindowOverridePosition, positionIsOverridden);
    }

    if (positionIsOverridden)
    {
        ImGui::SameLine();

        int values[2] = {
            _helpers.userConfiguration()->getInt(kConfigWindowLeft, 0),
            _helpers.userConfiguration()->getInt(kConfigWindowTop, 0)};

        if (ImGui::SliderInt2("##window_pos", values, -kWindowPositionLimit, kWindowPositionLimit))
        {
            _helpers.userConfiguration()->setInt(kConfigWindowLeft, values[0]);
            _helpers.userConfiguration()->setInt(kConfigWindowTop, values[1]);
            _helpers.markChanged();
        }
    }

    _helpers.DrawResetAndOverrideMarker(kConfigWindowOverridePosition, "", kConfigWindowLeft, kConfigWindowTop);
}
