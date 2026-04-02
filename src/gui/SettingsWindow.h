#pragma once

#include "AudioSettingsTab.h"
#include "FileChooser.h"
#include "ProjectMSettingsTab.h"
#include "SettingsUIHelpers.h"
#include "WindowSettingsTab.h"

#include <Poco/Util/MapConfiguration.h>
#include <Poco/Util/PropertyFileConfiguration.h>

class AudioCapture;
class ProjectMGUI;

class SettingsWindow
{
public:
    SettingsWindow() = delete;

    /**
     * @brief Creates the settings window coordinator.
     */
    explicit SettingsWindow(ProjectMGUI& gui);

    /**
     * @brief Shows the settings window and prepares tab state.
     */
    void Show();

    /**
     * @brief Draws the settings window and delegates tab rendering.
     */
    void Draw();

private:
    /**
     * @brief Draws the static help tab content.
     */
    void DrawHelpTab() const;

    /**
     * @brief Draws and handles the save button action.
     */
    void SaveButton();

    ProjectMGUI& _gui; //!< The GUI subsystem.
    AudioCapture& _audioCapture; //!< The audio capture subsystem.

    bool _visible{false}; //!< Window visibility flag.
    bool _changed{false}; //!< true if the user changed any setting since the last save.

    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _userConfiguration;
    Poco::AutoPtr<Poco::Util::MapConfiguration> _commandLineConfiguration;

    FileChooser _pathChooser{FileChooser::Mode::Directory}; //!< The file chooser dialog to select preset and texture paths.

    SettingsUIHelpers _helpers;
    ProjectMSettingsTab _projectMTab;
    WindowSettingsTab _windowTab;
    AudioSettingsTab _audioTab;
};
