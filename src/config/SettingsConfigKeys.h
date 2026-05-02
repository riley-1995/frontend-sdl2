#pragma once

namespace SettingsConfigKeys
{

// Canonical config keys are fully-qualified and must match projectMSDL.properties.in.

constexpr char kConfigProjectMPresetPath[] = "projectM.presetPath";
constexpr char kConfigProjectMTexturePath[] = "projectM.texturePath";
constexpr char kConfigProjectMEnableSplash[] = "projectM.enableSplash";
constexpr char kConfigProjectMPresetLocked[] = "projectM.presetLocked";
constexpr char kConfigProjectMShuffleEnabled[] = "projectM.shuffleEnabled";
constexpr char kConfigProjectMSkipToDropped[] = "projectM.skipToDropped";
constexpr char kConfigProjectMDroppedFolderOverride[] = "projectM.droppedFolderOverride";
constexpr char kConfigProjectMDisplayDuration[] = "projectM.displayDuration";
constexpr char kConfigProjectMTransitionDuration[] = "projectM.transitionDuration";
constexpr char kConfigProjectMHardCutsEnabled[] = "projectM.hardCutsEnabled";
constexpr char kConfigProjectMHardCutDuration[] = "projectM.hardCutDuration";
constexpr char kConfigProjectMHardCutSensitivity[] = "projectM.hardCutSensitivity";
constexpr char kConfigProjectMAspectCorrectionEnabled[] = "projectM.aspectCorrectionEnabled";
constexpr char kConfigProjectMMeshX[] = "projectM.meshX";
constexpr char kConfigProjectMMeshY[] = "projectM.meshY";
constexpr char kConfigProjectMFps[] = "projectM.fps";
constexpr char kConfigProjectMDisplayToasts[] = "projectM.displayToasts";
constexpr char kConfigProjectMBeatSensitivity[] = "projectM.beatSensitivity";

constexpr char kConfigWindowWidth[] = "window.width";
constexpr char kConfigWindowHeight[] = "window.height";
constexpr char kConfigWindowLeft[] = "window.left";
constexpr char kConfigWindowTop[] = "window.top";
constexpr char kConfigWindowOverridePosition[] = "window.overridePosition";
constexpr char kConfigWindowMonitor[] = "window.monitor";
constexpr char kConfigWindowBorderless[] = "window.borderless";
constexpr char kConfigWindowFullscreen[] = "window.fullscreen";
constexpr char kConfigWindowFullscreenExclusive[] = "window.fullscreen.exclusiveMode";
constexpr char kConfigWindowWaitForVerticalSync[] = "window.waitForVerticalSync";
constexpr char kConfigWindowAdaptiveVerticalSync[] = "window.adaptiveVerticalSync";
constexpr char kConfigWindowDisplayPresetNameInTitle[] = "window.displayPresetNameInTitle";
constexpr char kConfigWindowUiScale[] = "window.uiScale";

constexpr char kConfigFullscreenWidth[] = "window.fullscreen.width";
constexpr char kConfigFullscreenHeight[] = "window.fullscreen.height";

constexpr char kConfigAudioDevice[] = "audio.device";
constexpr char kConfigAppUserConfigurationFile[] = "app.UserConfigurationFile";

} // namespace SettingsConfigKeys