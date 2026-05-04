#include "config/ConfigurationFacade.h"

#include "config/SettingsConfigKeys.h"

namespace
{
// _effectiveConfig resolves defaults plus runtime overrides (user file, CLI, etc.).
// _userConfig is the persisted write layer used by mutating facade methods.
// Constants below preserve established behavior when a key is absent from effective config.
constexpr int kWindowWidthDefault = 800;
constexpr int kWindowHeightDefault = 600;
constexpr int kWindowLeftDefault = 0;
constexpr int kWindowTopDefault = 0;
constexpr bool kWindowOverridePositionDefault = false;
constexpr int kWindowMonitorDefault = 0;
constexpr bool kWindowBorderlessDefault = false;
constexpr bool kWindowFullscreenDefault = false;
constexpr bool kWindowFullscreenExclusiveModeDefault = false;
constexpr int kWindowFullscreenWidthDefault = 0;
constexpr int kWindowFullscreenHeightDefault = 0;
constexpr bool kWindowWaitForVerticalSyncDefault = true;
constexpr bool kWindowAdaptiveVerticalSyncDefault = true;
constexpr bool kDisplayPresetNameInTitleDefault = true;
} // namespace

ConfigurationFacade::WindowConfigFacade::WindowConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                                                            Poco::Util::AbstractConfiguration& userConfig)
    : _effectiveConfig(effectiveConfig)
    , _userConfig(userConfig)
{
}

int ConfigurationFacade::WindowConfigFacade::width() const
{
    // Read resolved startup window width from effective configuration.
    return _effectiveConfig.getInt(SettingsConfigKeys::kConfigWindowWidth, kWindowWidthDefault);
}

int ConfigurationFacade::WindowConfigFacade::height() const
{
    // Read resolved startup window height from effective configuration.
    return _effectiveConfig.getInt(SettingsConfigKeys::kConfigWindowHeight, kWindowHeightDefault);
}

int ConfigurationFacade::WindowConfigFacade::left() const
{
    // Read startup X position (monitor-relative when override is enabled).
    return _effectiveConfig.getInt(SettingsConfigKeys::kConfigWindowLeft, kWindowLeftDefault);
}

int ConfigurationFacade::WindowConfigFacade::top() const
{
    // Read startup Y position (monitor-relative when override is enabled).
    return _effectiveConfig.getInt(SettingsConfigKeys::kConfigWindowTop, kWindowTopDefault);
}

bool ConfigurationFacade::WindowConfigFacade::overridePosition() const
{
    // Read whether explicit startup position values should be applied.
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowOverridePosition,
                                    kWindowOverridePositionDefault);
}

int ConfigurationFacade::WindowConfigFacade::monitor() const
{
    // Read preferred startup monitor index (0 means OS-selected display).
    return _effectiveConfig.getInt(SettingsConfigKeys::kConfigWindowMonitor, kWindowMonitorDefault);
}

bool ConfigurationFacade::WindowConfigFacade::borderless() const
{
    // Read whether the window should start with OS border/title decorations disabled.
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowBorderless, kWindowBorderlessDefault);
}

bool ConfigurationFacade::WindowConfigFacade::fullscreen() const
{
    // Read whether startup should enter fullscreen mode.
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowFullscreen, kWindowFullscreenDefault);
}

bool ConfigurationFacade::WindowConfigFacade::fullscreenExclusiveMode() const
{
    // Read whether fullscreen should use exclusive display mode.
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowFullscreenExclusive,
                                    kWindowFullscreenExclusiveModeDefault);
}

int ConfigurationFacade::WindowConfigFacade::fullscreenWidth() const
{
    // Read exclusive fullscreen target width.
    return _effectiveConfig.getInt(SettingsConfigKeys::kConfigFullscreenWidth,
                                   kWindowFullscreenWidthDefault);
}

int ConfigurationFacade::WindowConfigFacade::fullscreenHeight() const
{
    // Read exclusive fullscreen target height.
    return _effectiveConfig.getInt(SettingsConfigKeys::kConfigFullscreenHeight,
                                   kWindowFullscreenHeightDefault);
}

bool ConfigurationFacade::WindowConfigFacade::waitForVerticalSync() const
{
    // Read whether frame presentation should block on VSync.
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowWaitForVerticalSync,
                                    kWindowWaitForVerticalSyncDefault);
}

bool ConfigurationFacade::WindowConfigFacade::adaptiveVerticalSync() const
{
    // Read whether adaptive VSync should be attempted when VSync is enabled.
    return _effectiveConfig.getBool(SettingsConfigKeys::kConfigWindowAdaptiveVerticalSync,
                                    kWindowAdaptiveVerticalSyncDefault);
}

bool ConfigurationFacade::WindowConfigFacade::displayPresetNameInTitle() const
{
    // Read whether preset names should be appended to the window title.
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
