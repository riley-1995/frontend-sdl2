#pragma once

class AudioCapture;
class SettingsUIHelpers;

class AudioSettingsTab
{
public:
    AudioSettingsTab() = delete;

    /**
     * @brief Creates the audio tab renderer.
     */
    AudioSettingsTab(SettingsUIHelpers& helpers, AudioCapture& audioCapture);

    /**
     * @brief Draws the audio settings tab content.
     */
    void Draw();

private:
    /**
     * @brief Displays a combobox with available audio devices.
     */
    void AudioDeviceSetting();

    SettingsUIHelpers& _helpers;
    AudioCapture& _audioCapture;
};
