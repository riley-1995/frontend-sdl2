#include "AudioSettingsTab.h"

#include "AudioCapture.h"
#include "SettingsConfigKeys.h"
#include "SettingsUIHelpers.h"

#include <imgui.h>

using namespace SettingsConfigKeys;

namespace
{
constexpr int kSettingsTableColumnCount = 5;
constexpr float kAudioChooseColumnWidth = 50.0F;
constexpr float kAudioResetColumnWidth = 100.0F;

constexpr double kBeatSensitivityDefault = 1.0;
constexpr double kBeatSensitivityMin = 0.0;
constexpr double kBeatSensitivityMax = 2.0;
} // namespace

AudioSettingsTab::AudioSettingsTab(SettingsUIHelpers& helpers, AudioCapture& audioCapture)
    : _helpers(helpers)
    , _audioCapture(audioCapture)
{
}

void AudioSettingsTab::Draw()
{
    if (ImGui::BeginTabItem("Audio"))
    {
        if (ImGui::BeginTable("projectM", kSettingsTableColumnCount, ImGuiTableFlags_None))
        {
            ImGui::TableSetupColumn("##desc", ImGuiTableColumnFlags_WidthFixed, .0f);
            ImGui::TableSetupColumn("##setting", ImGuiTableColumnFlags_WidthStretch, .0f);
            ImGui::TableSetupColumn("##choose", ImGuiTableColumnFlags_WidthFixed, kAudioChooseColumnWidth);
            ImGui::TableSetupColumn("##reset", ImGuiTableColumnFlags_WidthFixed, kAudioResetColumnWidth);
            ImGui::TableSetupColumn("##override", ImGuiTableColumnFlags_WidthFixed, .0f);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Audio Capturing Device", "The device to capture audio from.");
            AudioDeviceSetting();

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Beat Sensitivity", "Beat detection multiplier.");
            _helpers.DoubleSetting(kConfigProjectMBeatSensitivity, kBeatSensitivityDefault,
                                   kBeatSensitivityMin, kBeatSensitivityMax);

            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
}

void AudioSettingsTab::AudioDeviceSetting()
{
    ImGui::TableSetColumnIndex(1);

    auto devices = _audioCapture.AudioDeviceList();
    auto currentIndex = _audioCapture.AudioDeviceIndex();

    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##audiodevice", devices.at(currentIndex).c_str(), 0))
    {
        for (const auto& device : devices)
        {
            bool isSelected = device.first == currentIndex;

            if (ImGui::Selectable(device.second.c_str(), isSelected))
            {
                _audioCapture.AudioDeviceIndex(device.first);
                if (device.first == -1)
                {
                    _helpers.userConfiguration()->setInt(kConfigAudioDevice, -1);
                }
                else
                {
                    _helpers.userConfiguration()->setString(kConfigAudioDevice, device.second);
                }
                _helpers.markChanged();
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    _helpers.DrawResetAndOverrideMarker(kConfigAudioDevice);
}
