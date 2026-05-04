#pragma once

#include <Poco/Util/AbstractConfiguration.h>

class ConfigurationFacade
{
public:
    // Window-scoped settings read from the effective (merged) config and persist into user config.
    class WindowConfigFacade
    {
    public:
        WindowConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                           Poco::Util::AbstractConfiguration& userConfig);

        bool displayPresetNameInTitle() const;

        void setDisplayPresetNameInTitle(bool enabled);

        void toggleDisplayPresetNameInTitle();

    private:
        // Effective config includes defaults plus persisted overrides.
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        // User config is the writable layer persisted to disk.
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    // Placeholder facade for projectM-scoped settings methods.
    class ProjectMConfigFacade
    {
    public:
        ProjectMConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                             Poco::Util::AbstractConfiguration& userConfig);

    private:
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    // Placeholder facade for audio-scoped settings methods.
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

    // Accessors expose scoped config APIs and hide string-key usage.
    WindowConfigFacade& window();

    ProjectMConfigFacade& projectM();

    AudioConfigFacade& audio();

private:
    WindowConfigFacade _windowConfig;
    ProjectMConfigFacade _projectMConfig;
    AudioConfigFacade _audioConfig;
};
