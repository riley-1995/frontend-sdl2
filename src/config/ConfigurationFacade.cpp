#include "config/ConfigurationFacade.h"

#include "config/SettingsConfigKeys.h"

namespace
{
constexpr bool kDisplayPresetNameInTitleDefault = true;
} // namespace

ConfigurationFacade::WindowConfigFacade::WindowConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                                                            Poco::Util::AbstractConfiguration& userConfig)
    : _effectiveConfig(effectiveConfig)
    , _userConfig(userConfig)
{
}

bool ConfigurationFacade::WindowConfigFacade::displayPresetNameInTitle() const
{
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowDisplayPresetNameInTitle,
                                    kDisplayPresetNameInTitleDefault);
}

void ConfigurationFacade::WindowConfigFacade::setDisplayPresetNameInTitle(bool enabled)
{
    _userConfig.setBool(SettingsConfigKeys::kConfigWindowDisplayPresetNameInTitle, enabled);
}

void ConfigurationFacade::WindowConfigFacade::toggleDisplayPresetNameInTitle()
{
    setDisplayPresetNameInTitle(!displayPresetNameInTitle());
}

ConfigurationFacade::ProjectMConfigFacade::ProjectMConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                                                                Poco::Util::AbstractConfiguration& userConfig)
    : _effectiveConfig(effectiveConfig)
    , _userConfig(userConfig)
{
}

ConfigurationFacade::AudioConfigFacade::AudioConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                                                          Poco::Util::AbstractConfiguration& userConfig)
    : _effectiveConfig(effectiveConfig)
    , _userConfig(userConfig)
{
}

ConfigurationFacade::ConfigurationFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                                         Poco::Util::AbstractConfiguration& userConfig)
    : _windowConfig(effectiveConfig, userConfig)
    , _projectMConfig(effectiveConfig, userConfig)
    , _audioConfig(effectiveConfig, userConfig)
{
}

ConfigurationFacade::WindowConfigFacade& ConfigurationFacade::window()
{
    return _windowConfig;
}

ConfigurationFacade::ProjectMConfigFacade& ConfigurationFacade::projectM()
{
    return _projectMConfig;
}

ConfigurationFacade::AudioConfigFacade& ConfigurationFacade::audio()
{
    return _audioConfig;
}
