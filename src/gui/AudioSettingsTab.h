#pragma once

class AudioCapture;
class SettingsUIHelpers;

class AudioSettingsTab
{
public:
    AudioSettingsTab() = delete;
    AudioSettingsTab(SettingsUIHelpers& helpers, AudioCapture& audioCapture);

    void Draw();

private:
    void AudioDeviceSetting();

    SettingsUIHelpers& _helpers;
    AudioCapture& _audioCapture;
};
