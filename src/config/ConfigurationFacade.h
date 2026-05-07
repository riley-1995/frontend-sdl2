#pragma once

#include <Poco/Util/AbstractConfiguration.h>

class ConfigurationFacade
{
public:
    /**
     * @brief Facade for window-scoped configuration access.
     *
     * Reads values from the effective (merged) configuration and persists writable values
     * to the user configuration layer.
     */
    class WindowConfigFacade
    {
    public:
        /**
         * @brief Creates a facade for window configuration values.
         * @param effectiveConfig The merged read configuration source.
         * @param userConfig The writable user configuration source.
         */
        WindowConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                           Poco::Util::AbstractConfiguration& userConfig);

        /**
         * @brief Returns the configured startup window width.
         * @return Startup width in pixels.
         */
        int width() const;

        /**
         * @brief Returns the configured startup window height.
         * @return Startup height in pixels.
         */
        int height() const;

        /**
         * @brief Returns the configured startup X position.
         * @return Startup left position.
         */
        int left() const;

        /**
         * @brief Returns the configured startup Y position.
         * @return Startup top position.
         */
        int top() const;

        /**
         * @brief Returns whether startup position override is enabled.
         * @return True if startup position values should be applied.
         */
        bool overridePosition() const;

        /**
         * @brief Returns the configured startup monitor.
         * @return Monitor index where 0 means OS-selected display.
         */
        int monitor() const;

        /**
         * @brief Returns whether the window should be borderless.
         * @return True if borderless startup is enabled.
         */
        bool borderless() const;

        /**
         * @brief Returns whether startup fullscreen mode is enabled.
         * @return True if window should start in fullscreen.
         */
        bool fullscreen() const;

        /**
         * @brief Returns whether exclusive fullscreen mode is enabled.
         * @return True if exclusive fullscreen should be used.
         */
        bool fullscreenExclusiveMode() const;

        /**
         * @brief Returns the configured exclusive fullscreen width.
         * @return Fullscreen width in pixels.
         */
        int fullscreenWidth() const;

        /**
         * @brief Returns the configured exclusive fullscreen height.
         * @return Fullscreen height in pixels.
         */
        int fullscreenHeight() const;

        /**
         * @brief Returns whether waiting for vertical sync is enabled.
         * @return True if VSync wait is enabled.
         */
        bool waitForVerticalSync() const;

        /**
         * @brief Returns whether adaptive vertical sync is enabled.
         * @return True if adaptive VSync is enabled.
         */
        bool adaptiveVerticalSync() const;

        /**
         * @brief Returns whether preset names are shown in the window title.
         * @return True if preset names should be appended to the title.
         */
        bool displayPresetNameInTitle() const;

        /**
         * @brief Sets whether preset names are shown in the window title.
         * @param enabled True to append preset names to the title.
         */
        void setDisplayPresetNameInTitle(bool enabled);

        /**
         * @brief Toggles preset-name display in the window title.
         */
        void toggleDisplayPresetNameInTitle();

    private:
        // Effective config includes defaults plus persisted overrides.
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        // User config is the writable layer persisted to disk.
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    /**
     * @brief Placeholder facade for projectM-scoped settings.
     */
    class ProjectMConfigFacade
    {
    public:
        /**
         * @brief Creates a facade for projectM-scoped configuration values.
         * @param effectiveConfig The merged read configuration source.
         * @param userConfig The writable user configuration source.
         */
        ProjectMConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                             Poco::Util::AbstractConfiguration& userConfig);

        /**
         * @brief Returns whether the current preset is locked.
         * @return True if preset locking is enabled.
         */
        bool presetLocked() const;

        /**
         * @brief Sets preset lock state.
         * @param enabled True to lock the current preset.
         */
        void setPresetLocked(bool enabled);

        /**
         * @brief Toggles preset lock state.
         */
        void togglePresetLocked();

        /**
         * @brief Returns whether shuffle is enabled for preset playback.
         * @return True if shuffle mode is enabled.
         */
        bool shuffleEnabled() const;

        /**
         * @brief Sets shuffle state.
         * @param enabled True to enable shuffle mode.
         */
        void setShuffleEnabled(bool enabled);

        /**
         * @brief Toggles shuffle state.
         */
        void toggleShuffleEnabled();

        /**
         * @brief Returns whether toast messages are displayed.
         * @return True if toast display is enabled.
         */
        bool displayToasts() const;

        /**
         * @brief Sets whether toast messages are displayed.
         * @param enabled True to enable toast display.
         */
        void setDisplayToasts(bool enabled);

        /**
         * @brief Toggles toast message display.
         */
        void toggleDisplayToasts();

        /**
         * @brief Returns the configured beat sensitivity value.
         * @return Beat sensitivity scalar.
         */
        double beatSensitivity() const;

        /**
         * @brief Sets beat sensitivity.
         * @param value Beat sensitivity scalar.
         */
        void setBeatSensitivity(double value);

    private:
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    /**
     * @brief Facade for audio-scoped configuration access.
     *
     * Reads values from the effective (merged) configuration and persists writable values
     * to the user configuration layer. Provides a single typed API for all audio settings
     * used by capture, settings UI, and CLI, replacing scattered raw key lookups.
     */
    class AudioConfigFacade
    {
    public:
        /**
         * @brief Creates a facade for audio-scoped configuration values.
         * @param effectiveConfig The merged read configuration source.
         * @param userConfig The writable user configuration source.
         */
        AudioConfigFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                          Poco::Util::AbstractConfiguration& userConfig);

        /**
         * @brief Returns whether available audio devices should be listed at startup.
         * @return True if the device list should be logged on startup.
         */
        bool listDevices() const;

        /**
         * @brief Returns the configured audio device as a numeric index.
         *
         * Reads audio.device as an integer. Throws Poco::SyntaxException if the
         * configured value is a non-numeric string — callers should catch and fall
         * back to deviceName() in that case.
         *
         * @return Device index, or -1 if unset (default capture device).
         */
        int deviceIndex() const;

        /**
         * @brief Returns the configured audio device as a string name.
         *
         * Used as a fallback when the audio.device value is a non-numeric name.
         *
         * @return Device name string, or empty string if unset.
         */
        std::string deviceName() const;

        /**
         * @brief Persists an audio device selection by numeric index.
         * @param index Device index as reported by the capture API, or -1 for default.
         */
        void setDevice(int index);

        /**
         * @brief Persists an audio device selection by name.
         * @param name Full device name string as reported by the capture API.
         */
        void setDevice(const std::string& name);

    private:
        // Effective config includes defaults plus persisted overrides.
        Poco::Util::AbstractConfiguration& _effectiveConfig;
        // User config is the writable layer persisted to disk.
        Poco::Util::AbstractConfiguration& _userConfig;
    };

    /**
     * @brief Creates the top-level configuration facade.
     * @param effectiveConfig The merged read configuration source.
     * @param userConfig The writable user configuration source.
     */
    ConfigurationFacade(Poco::Util::AbstractConfiguration& effectiveConfig,
                        Poco::Util::AbstractConfiguration& userConfig);

    /**
     * @brief Returns the window-scoped configuration facade.
     * @return The window facade.
     */
    WindowConfigFacade& window();

    /**
     * @brief Returns the projectM-scoped configuration facade.
     * @return The projectM facade.
     */
    ProjectMConfigFacade& projectM();

    /**
     * @brief Returns the audio-scoped configuration facade.
     * @return The audio facade.
     */
    AudioConfigFacade& audio();

private:
    WindowConfigFacade _windowConfig;
    ProjectMConfigFacade _projectMConfig;
    AudioConfigFacade _audioConfig;
};
