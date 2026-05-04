#include "ProjectMSettingsTab.h"

#include "config/SettingsConfigKeys.h"
#include "SettingsUIHelpers.h"

#include <imgui.h>

using namespace SettingsConfigKeys;

namespace
{
constexpr int kSettingsTableColumnCount = 5;
constexpr float kChooseColumnWidth = 100.0F;
constexpr float kResetColumnWidth = 50.0F;

constexpr double kPresetDisplayDurationDefault = 30.0;
constexpr double kPresetDisplayDurationMin = 1.0;
constexpr double kPresetDisplayDurationMax = 240.0;
constexpr double kPresetTransitionDurationDefault = 3.0;
constexpr double kPresetTransitionDurationMin = 0.0;
constexpr double kPresetTransitionDurationMax = 10.0;
constexpr double kHardCutDurationDefault = 20.0;
constexpr double kHardCutDurationMin = 1.0;
constexpr double kHardCutDurationMax = 240.0;
constexpr double kHardCutSensitivityDefault = 1.0;
constexpr double kHardCutSensitivityMin = 0.0;
constexpr double kHardCutSensitivityMax = 10.0;

constexpr int kMeshDefaultX = 64;
constexpr int kMeshDefaultY = 48;
constexpr int kMeshSizeMin = 8;
// Upper mesh bound keeps CPU-heavy settings available for advanced presets.
constexpr int kMeshSizeMax = 300;
} // namespace

ProjectMSettingsTab::ProjectMSettingsTab(SettingsUIHelpers& helpers)
    : _helpers(helpers)
{
}

void ProjectMSettingsTab::Draw()
{
    if (ImGui::BeginTabItem("projectM"))
    {
        if (ImGui::BeginTable("projectM", kSettingsTableColumnCount, ImGuiTableFlags_None))
        {
            ImGui::TableSetupColumn("##desc", ImGuiTableColumnFlags_WidthFixed, .0f);
            ImGui::TableSetupColumn("##setting", ImGuiTableColumnFlags_WidthStretch, .0f);
            ImGui::TableSetupColumn("##choose", ImGuiTableColumnFlags_WidthFixed, kChooseColumnWidth);
            ImGui::TableSetupColumn("##reset", ImGuiTableColumnFlags_WidthFixed, kResetColumnWidth);
            ImGui::TableSetupColumn("##override", ImGuiTableColumnFlags_WidthFixed, .0f);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Preset Path", "Path to search for preset files if no playlist is loaded.");
            _helpers.PathSetting(kConfigProjectMPresetPath);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Texture Path", "Path to search for texture/image files requested by presets.");
            _helpers.PathSetting(kConfigProjectMTexturePath);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Display projectM Logo Preset",
                                      "If enabled, the projectM logo preset is shown on startup.");
            _helpers.BooleanSetting(kConfigProjectMEnableSplash, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Lock Preset", "If enabled, presets will not be switched automatically.");
            _helpers.BooleanSetting(kConfigProjectMPresetLocked, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Shuffle Presets", "Selects presets randomly from the current playlist.");
            _helpers.BooleanSetting(kConfigProjectMShuffleEnabled, true);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Skip To Dropped Presets",
                                      "If enabled, will skip to the new presets when preset(s) are dropped onto the window and added to the playlist");
            _helpers.BooleanSetting(kConfigProjectMSkipToDropped, true);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Dropped Folder Overrides Playlist",
                                      "When dropping a folder, clear the playlist and add all presets from the folder.");
            _helpers.BooleanSetting(kConfigProjectMDroppedFolderOverride, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Preset Display Duration",
                                      "Time in seconds a preset will be displayed before it's switched.");
            _helpers.DoubleSetting(kConfigProjectMDisplayDuration, kPresetDisplayDurationDefault,
                                   kPresetDisplayDurationMin, kPresetDisplayDurationMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Preset Transition Duration",
                                      "Time in seconds it takes to transition softly from one preset to another.");
            _helpers.DoubleSetting(kConfigProjectMTransitionDuration, kPresetTransitionDurationDefault,
                                   kPresetTransitionDurationMin, kPresetTransitionDurationMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Enable Hard Cuts",
                                      "Enables beat-driven, fast preset changes.\nSensitivity and earliest switch time can be configured separately.");
            _helpers.BooleanSetting(kConfigProjectMHardCutsEnabled, false);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Hard Cut Duration",
                                      "Time in seconds before a preset will be switched at\nthe earliest on hard cuts. If larger than display duration,\nhard cuts won't happen at all.");
            _helpers.DoubleSetting(kConfigProjectMHardCutDuration, kHardCutDurationDefault,
                                   kHardCutDurationMin, kHardCutDurationMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("  Hard Cut Threshold",
                                      "Volume difference between measurements required to trigger a hard cut.\nHigher values mean fewer hard cuts.");
            _helpers.DoubleSetting(kConfigProjectMHardCutSensitivity, kHardCutSensitivityDefault,
                                   kHardCutSensitivityMin, kHardCutSensitivityMax);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Aspect Correction",
                                      "Enables aspect ration correction in presets.\nOnly affects presets using the aspect ratio variables.");
            _helpers.BooleanSetting(kConfigProjectMAspectCorrectionEnabled, true);

            ImGui::TableNextRow();
            _helpers.LabelWithTooltip("Per-Point Mesh Size X/Y",
                                      "Size of the per-point transformation grid.\nHigher values produce better quality, but require exponentially more CPU time to calculate.\nMilkdrop's default is 48x32.");
            _helpers.IntegerSettingVec(kConfigProjectMMeshX, kConfigProjectMMeshY, kMeshDefaultX, kMeshDefaultY,
                                       kMeshSizeMin, kMeshSizeMax);

            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
}
