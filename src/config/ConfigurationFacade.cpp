#include "config/ConfigurationFacade.h"

#include "config/SettingsConfigKeys.h"

namespace
{
// Keep behavior aligned with existing UI defaults when key is absent.
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
    // Read from effective config so defaults and user overrides are both respected.
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowDisplayPresetNameInTitle,
                                    kDisplayPresetNameInTitleDefault);
}

void ConfigurationFacade::WindowConfigFacade::setDisplayPresetNameInTitle(bool enabled)
{
    // Persist explicit choice in the user configuration layer.
    _userConfig.setBool(SettingsConfigKeys::kConfigWindowDisplayPresetNameInTitle, enabled);
}

void ConfigurationFacade::WindowConfigFacade::toggleDisplayPresetNameInTitle()
{
    // Toggle based on the resolved value that the UI currently sees.
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
    // Each scoped facade shares the same read/write configuration sources.
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
