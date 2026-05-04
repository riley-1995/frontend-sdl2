#include "gui/MainMenu.h"

#include "AudioCapture.h"
#include "ProjectMSDLApplication.h"
#include "ProjectMWrapper.h"

#include "config/ConfigurationFacade.h"

#include "gui/ProjectMGUI.h"
#include "gui/SystemBrowser.h"

#include "notifications/PlaybackControlNotification.h"
#include "notifications/QuitNotification.h"
#include "notifications/UpdateWindowTitleNotification.h"

#include "imgui.h"

#include <Poco/NotificationCenter.h>


MainMenu::MainMenu(ProjectMGUI& gui)
    : _notificationCenter(Poco::NotificationCenter::defaultCenter())
    , _gui(gui)
    , _projectMWrapper(Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>())
    , _audioCapture(Poco::Util::Application::instance().getSubsystem<AudioCapture>())
{
}

void MainMenu::Draw() 
{
    if (!ImGui::BeginMainMenuBar()) return;

    DrawFileMenu();
    DrawPlaybackMenu();
    DrawOptionsMenu();
    DrawHelpMenu();

    ImGui::EndMainMenuBar();
}

void MainMenu::DrawFileMenu() 
{
    if (!ImGui::BeginMenu("File")) return;

    if (ImGui::MenuItem("Settings...", "Ctrl+s")) 
    {
        _gui.ShowSettingsWindow();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Quit projectM", "Ctrl+q")) 
    {
        _notificationCenter.postNotification(new QuitNotification);
    }

    ImGui::EndMenu();
}

void MainMenu::DrawPlaybackMenu() 
{
    if (!ImGui::BeginMenu("Playback")) return;

    auto& app = ProjectMSDLApplication::instance();

    if (ImGui::MenuItem("Play Next Preset", "n")) 
    {
        PostPlaybackAction(static_cast<int>(PlaybackControlNotification::Action::LastPreset));
    }
    if (ImGui::MenuItem("Play Previous Preset", "p")) 
    {
        PostPlaybackAction(static_cast<int>(PlaybackControlNotification::Action::PreviousPreset));
    }
    if (ImGui::MenuItem("Go Back One Preset", "Backspace"))
    {
        PostPlaybackAction(static_cast<int>(PlaybackControlNotification::Action::LastPreset));
    }
    if (ImGui::MenuItem("Random Preset", "r"))
    {
        PostPlaybackAction(static_cast<int>(PlaybackControlNotification::Action::RandomPreset));
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Lock Preset", "Spacebar", app.config().getBool("projectM.presetLocked", false)))
    {
        PostPlaybackAction(static_cast<int>(PlaybackControlNotification::Action::TogglePresetLocked));
    }
    if (ImGui::MenuItem("Enable Shuffle", "y", app.config().getBool("projectM.shuffleEnabled", true)))
    {
        PostPlaybackAction(static_cast<int>(PlaybackControlNotification::Action::ToggleShuffle));
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Copy Current Preset Filename", "Ctrl+c"))
    {
        _projectMWrapper.PresetFileNameToClipboard();
    }

    ImGui::EndMenu();
}

void MainMenu::DrawOptionsMenu() 
{
    if (!ImGui::BeginMenu("Options"))
    {
        return;
    }

    auto& app = ProjectMSDLApplication::instance();
    // Facade keeps menu code focused on intent instead of raw config keys.
    ConfigurationFacade configurationFacade(app.config(), *app.UserConfiguration());

    DrawAudioCaptureDeviceMenu();

    ImGui::Separator();

    if (ImGui::MenuItem("Display Toast Messages", "", app.config().getBool("projectM.displayToasts", true)))
    {
        ToggleUserConfig("projectM.displayToasts", true);
    }
    if (ImGui::MenuItem("Display Preset Name in Window Title", "", configurationFacade.window().displayPresetNameInTitle()))
    {
        // Writes to user config while reading current state from effective config.
        configurationFacade.window().toggleDisplayPresetNameInTitle();
        _notificationCenter.postNotification(new UpdateWindowTitleNotification);
    }

    ImGui::Separator();

    float beatSensitivity = projectm_get_beat_sensitivity(_projectMWrapper.ProjectM());
    if (ImGui::SliderFloat("Beat Sensitivity", &beatSensitivity, 0.0f, 2.0f))
    {
        projectm_set_beat_sensitivity(_projectMWrapper.ProjectM(), beatSensitivity);
        app.UserConfiguration()->setDouble("projectM.beatSensitivity", beatSensitivity);
    }

    ImGui::EndMenu();
}

void MainMenu::DrawAudioCaptureDeviceMenu() 
{
    if (!ImGui::BeginMenu("Audio Capture Device"))
    {
        return;
    }

    const auto devices = _audioCapture.AudioDeviceList();
    const auto currentIndex = _audioCapture.AudioDeviceIndex();

    for (const auto& device : devices)
    {
        if (ImGui::MenuItem(device.second.c_str(), "", device.first == currentIndex))
        {
            _audioCapture.AudioDeviceIndex(device.first);
        }
    }

    ImGui::EndMenu();
}

void MainMenu::DrawHelpMenu() 
{
    if (!ImGui::BeginMenu("Help"))
    {
        return;
    }

    if (ImGui::MenuItem("Quick Help..."))
    {
        _gui.ShowHelpWindow();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("About projectM..."))
    {
        _gui.ShowAboutWindow();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Visit the projectM Wiki on GitHub"))
    {
        OpenExternalUrl("https://github.com/projectM-visualizer/projectm/wiki");
    }
    if (ImGui::MenuItem("Report a Bug or Request a Feature"))
    {
        OpenExternalUrl("https://github.com/projectM-visualizer/projectm/issues/new/choose");
    }
    if (ImGui::MenuItem("Sponsor projectM on OpenCollective"))
    {
        OpenExternalUrl("https://opencollective.com/projectm");
    }

    ImGui::EndMenu();
}

void MainMenu::PostPlaybackAction(int action) 
{
    _notificationCenter.postNotification(
        new PlaybackControlNotification(static_cast<PlaybackControlNotification::Action>(action)));
}

void MainMenu::ToggleUserConfig(const std::string& key, bool defaultValue) const 
{
    auto& app = ProjectMSDLApplication::instance();
    app.UserConfiguration()->setBool(key, !app.config().getBool(key, defaultValue));
}

void MainMenu::OpenExternalUrl(const char* url) const 
{
    SystemBrowser::OpenURL(url);
}
