#include "SettingsWindow.h"

#include "AudioCapture.h"
#include "ProjectMSDLApplication.h"

#include "notifications/DisplayToastNotification.h"

#include <imgui.h>

#include <Poco/NotificationCenter.h>
#include <Poco/Path.h>

namespace
{
constexpr float kSettingsWindowWidth = 1050.0F;
constexpr float kSettingsWindowHeight = 550.0F;

constexpr char kConfigAppUserConfigurationFile[] = "app.UserConfigurationFile";
} // namespace

SettingsWindow::SettingsWindow(ProjectMGUI& gui)
    : _gui(gui)
    , _audioCapture(ProjectMSDLApplication::instance().getSubsystem<AudioCapture>())
    , _userConfiguration(ProjectMSDLApplication::instance().UserConfiguration())
    , _commandLineConfiguration(ProjectMSDLApplication::instance().CommandLineConfiguration())
    , _helpers(_userConfiguration, _commandLineConfiguration, _changed, _pathChooser)
    , _projectMTab(_helpers)
    , _windowTab(_helpers)
    , _audioTab(_helpers, _audioCapture)
{
}

void SettingsWindow::Show()
{
    _windowTab.OnShow();
    _visible = true;
}

void SettingsWindow::Draw()
{
    if (!_visible)
    {
        return;
    }

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    constexpr ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_None;

    std::string windowId = "Settings";
    if (_changed)
    {
        // Surface unsaved state directly in the title bar so users can see it while switching tabs.
        windowId.append(" [CHANGED - NOT SAVED]");
    }
    windowId.append("###Settings");

    ImGui::SetNextWindowSize(ImVec2(kSettingsWindowWidth, kSettingsWindowHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowId.c_str(), &_visible, windowFlags))
    {
        if (ImGui::BeginTabBar("projectM Settings", tabBarFlags))
        {
            _projectMTab.Draw();
            _windowTab.Draw();
            _audioTab.Draw();
            DrawHelpTab();

            ImGui::EndTabBar();
        }

        ImGui::Separator();
        SaveButton();
    }
    ImGui::End();

    if (_pathChooser.Draw())
    {
        auto& selectedDirectory = _pathChooser.SelectedFiles();
        if (!selectedDirectory.empty())
        {
            // Persist the selected directory immediately; Save will write the full config to disk.
            _userConfiguration->setString(_pathChooser.Context(),
                                          Poco::Path(selectedDirectory.at(0).path()).makeDirectory().toString());
            _changed = true;
        }
    }
}

void SettingsWindow::DrawHelpTab() const
{
    if (ImGui::BeginTabItem("Help"))
    {
        Poco::Path userConfigurationDir = Poco::Path::configHome();
        userConfigurationDir.makeDirectory().append("projectM/");

        ImGui::TextUnformatted("General:");
        ImGui::Bullet();
        ImGui::TextWrapped("%s", "Hover over the setting name to see a short description.");
        ImGui::Bullet();
        ImGui::TextWrapped("%s", "Settings overridden by command-line parameters cannot be changed.");
        ImGui::Bullet();
        ImGui::TextWrapped(
            "All values changed/set in this window are stored in the configuration file, projectMSDL.properties, in the user configuration directory \"%s\"",
            userConfigurationDir.toString().c_str());

        ImGui::Separator();

        ImGui::TextUnformatted("Changing values:");
        ImGui::Bullet();
        ImGui::TextWrapped(
            "%s",
            "Hold down control/command key when clicking on sliders to enter a custom value. The manually entered value can be outside the slider's value range.");
        ImGui::Bullet();
        ImGui::TextWrapped(
            "%s",
            "Click on \"Reset\" to unset a value and use the default value from the application's factory configuration file.");
        ImGui::EndTabItem();
    }
}

void SettingsWindow::SaveButton()
{
    if (ImGui::Button("Save Settings"))
    {
        try
        {
            // The user config file path can be injected by command-line options.
            auto configFile = _commandLineConfiguration->getString(kConfigAppUserConfigurationFile, "");
            if (!configFile.empty())
            {
                _userConfiguration->save(configFile);
                Poco::NotificationCenter::defaultCenter().postNotification(
                    new DisplayToastNotification("Settings saved!"));
            }
            else
            {
                Poco::NotificationCenter::defaultCenter().postNotification(
                    new DisplayToastNotification("Error saving settings"));
            }
        }
        catch (...)
        {
            Poco::NotificationCenter::defaultCenter().postNotification(
                new DisplayToastNotification("Error saving settings"));
        }

        _changed = false;
    }
}
