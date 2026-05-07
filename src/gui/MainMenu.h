#pragma once

class ProjectMGUI;
class ProjectMWrapper;
class AudioCapture;

namespace Poco {
class NotificationCenter;
}

class MainMenu
{
public:
    MainMenu() = delete;

    explicit MainMenu(ProjectMGUI& gui);

    /**
     * @brief Draws the main menu bar.
     */
    void Draw();

private:
    Poco::NotificationCenter& _notificationCenter; //!< Notification center instance.
    ProjectMGUI& _gui; //!< Reference to the GUI subsystem.
    ProjectMWrapper& _projectMWrapper; //!< Reference to the projectM wrapper subsystem.
    AudioCapture& _audioCapture; //!< Reference to the audio capture subsystem.

    void DrawFileMenu();
    void DrawPlaybackMenu();
    void DrawOptionsMenu();
    void DrawAudioCaptureDeviceMenu();
    void DrawHelpMenu();

    void PostPlaybackAction(int action);
    void OpenExternalUrl(const char* url) const;
};
