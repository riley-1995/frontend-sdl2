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
    explicit SettingsWindow(ProjectMGUI& gui);

    void Show();
    void Draw();

private:
    void DrawHelpTab() const;
    void SaveButton();

    ProjectMGUI& _gui;
    AudioCapture& _audioCapture;

    bool _visible{false};
    bool _changed{false};

    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _userConfiguration;
    Poco::AutoPtr<Poco::Util::MapConfiguration> _commandLineConfiguration;

    FileChooser _pathChooser{FileChooser::Mode::Directory};

    SettingsUIHelpers _helpers;
    ProjectMSettingsTab _projectMTab;
    WindowSettingsTab _windowTab;
    AudioSettingsTab _audioTab;
};
