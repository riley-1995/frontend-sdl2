#pragma once

#include <Poco/Util/AbstractConfiguration.h>

class ConfigurationFacade
{
public:
    class WindowConfigFacade
    {
    public:
        WindowConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                           Poco::Util::AbstractConfiguration& userConfig);

        bool displayPresetNameInTitle() const;

        void setDisplayPresetNameInTitle(bool enabled);

        void toggleDisplayPresetNameInTitle();

    private:
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    class ProjectMConfigFacade
    {
    public:
        ProjectMConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                             Poco::Util::AbstractConfiguration& userConfig);

    private:
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    class AudioConfigFacade
    {
    public:
        AudioConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                          Poco::Util::AbstractConfiguration& userConfig);

    private:
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    ConfigurationFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                        Poco::Util::AbstractConfiguration& userConfig);

    WindowConfigFacade& window();

    ProjectMConfigFacade& projectM();

    AudioConfigFacade& audio();

private:
    WindowConfigFacade _windowConfig;
    ProjectMConfigFacade _projectMConfig;
    AudioConfigFacade _audioConfig;
};
