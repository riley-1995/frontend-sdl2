#include "SettingsUIHelpers.h"

#include "FileChooser.h"

#include <imgui.h>

#include <algorithm>

namespace
{
constexpr std::size_t kPathBufferSize = 2048;
constexpr std::size_t kPathBufferCopyLimit = kPathBufferSize - 1;
} // namespace

SettingsUIHelpers::SettingsUIHelpers(
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& userConfiguration,
    Poco::AutoPtr<Poco::Util::MapConfiguration>& commandLineConfiguration,
    bool& changed,
    FileChooser& pathChooser)
    : _userConfiguration(userConfiguration)
    , _commandLineConfiguration(commandLineConfiguration)
    , _changed(changed)
    , _pathChooser(pathChooser)
{
}

Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& SettingsUIHelpers::userConfiguration()
{
    return _userConfiguration;
}

void SettingsUIHelpers::markChanged()
{
    _changed = true;
}

void SettingsUIHelpers::LabelWithTooltip(const std::string& label, const std::string& tooltipText)
{
    ImGui::TableSetColumnIndex(0);

    ImGui::TextUnformatted(label.c_str());
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltipText.c_str());
        ImGui::EndTooltip();
    }
}

void SettingsUIHelpers::PathSetting(const std::string& property)
{
    ImGui::TableSetColumnIndex(1);

    auto path = _userConfiguration->getString(property, "");
    char pathBuffer[kPathBufferSize]{};
    strncpy(pathBuffer, path.c_str(), std::min<std::size_t>(kPathBufferCopyLimit, path.size()));

    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText(std::string("##path_" + property).c_str(), &pathBuffer[0], IM_ARRAYSIZE(pathBuffer),
                         ImGuiInputTextFlags_EnterReturnsTrue))
    {
        _userConfiguration->setString(property, std::string(pathBuffer));
        _changed = true;
    }

    ImGui::TableSetColumnIndex(2);

    ImGui::PushID(std::string(property + "_ChoosePathButton").c_str());
    if (ImGui::Button("..."))
    {
        _pathChooser.CurrentDirectory(path);
        _pathChooser.Title("Select directory");
        _pathChooser.Context(property);
        _pathChooser.Show();
    }
    ImGui::PopID();

    DrawResetAndOverrideMarker(property);
}

void SettingsUIHelpers::BooleanSetting(const std::string& property, bool defaultValue)
{
    ImGui::TableSetColumnIndex(1);

    auto value = _userConfiguration->getBool(property, defaultValue);

    if (ImGui::Checkbox(std::string("##boolean_" + property).c_str(), &value))
    {
        _userConfiguration->setBool(property, value);
        _changed = true;
    }

    DrawResetAndOverrideMarker(property);
}

void SettingsUIHelpers::IntegerSetting(const std::string& property, int defaultValue, int min, int max)
{
    ImGui::TableSetColumnIndex(1);

    auto value = _userConfiguration->getInt(property, defaultValue);

    if (ImGui::SliderInt(std::string("##integer_" + property).c_str(), &value, min, max))
    {
        _userConfiguration->setInt(property, value);
        _changed = true;
    }

    DrawResetAndOverrideMarker(property);
}

void SettingsUIHelpers::IntegerSettingVec(const std::string& property1, const std::string& property2,
                                          int defaultValue1, int defaultValue2, int min, int max)
{
    ImGui::TableSetColumnIndex(1);

    int values[2] = {
        _userConfiguration->getInt(property1, defaultValue1),
        _userConfiguration->getInt(property2, defaultValue2)};

    if (ImGui::SliderInt2(std::string("##integer_" + property1 + property2).c_str(), values, min, max))
    {
        _userConfiguration->setInt(property1, values[0]);
        _userConfiguration->setInt(property2, values[1]);
        _changed = true;
    }

    DrawResetAndOverrideMarker(property1, property2);
}

void SettingsUIHelpers::DoubleSetting(const std::string& property, double defaultValue, double min, double max)
{
    ImGui::TableSetColumnIndex(1);

    auto value = static_cast<float>(_userConfiguration->getDouble(property, defaultValue));

    if (ImGui::SliderFloat(std::string("##double_" + property).c_str(), &value, static_cast<float>(min),
                           static_cast<float>(max)))
    {
        _userConfiguration->setDouble(property, value);
        _changed = true;
    }

    DrawResetAndOverrideMarker(property);
}

void SettingsUIHelpers::DoubleSettingWithApply(const std::string& property, double defaultValue, double min,
                                               double max, float& tempValue)
{
    ImGui::TableSetColumnIndex(1);

    ImGui::SliderFloat(std::string("##double_" + property).c_str(), &tempValue, static_cast<float>(min),
                       static_cast<float>(max));

    ImGui::SameLine();

    ImGui::PushID(std::string(property + "_ApplyButton").c_str());
    if (ImGui::Button("Apply"))
    {
        _userConfiguration->setDouble(property, tempValue);
        _changed = true;
    }
    ImGui::PopID();

    if (DrawResetAndOverrideMarker(property))
    {
        tempValue = static_cast<float>(_userConfiguration->getDouble(property, defaultValue));
    }
}

bool SettingsUIHelpers::ResetButton(const std::string& property1, const std::string& property2)
{
    if (!_userConfiguration->has(property1) && (property2.empty() || !_userConfiguration->has(property2)))
    {
        return false;
    }

    ImGui::TableSetColumnIndex(3);

    bool pressed{false};

    ImGui::PushID(std::string(property1 + property2 + "_ResetButton").c_str());
    if (ImGui::Button("Reset"))
    {
        _userConfiguration->remove(property1);
        if (!property2.empty())
        {
            _userConfiguration->remove(property2);
        }
        _changed = true;
        pressed = true;
    }
    ImGui::PopID();

    return pressed;
}

void SettingsUIHelpers::OverriddenSettingMarker()
{
    ImGui::TableSetColumnIndex(4);

    ImGui::TextColored(ImVec4(1, 0, 0, 1), "[!]");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Value passed as command line argument.");
        ImGui::TextUnformatted("The value configured here will only be used if NOT overridden via the command line.");
        ImGui::EndTooltip();
    }
}

bool SettingsUIHelpers::DrawResetAndOverrideMarker(const std::string& resetKey1, const std::string& resetKey2,
                                                   const std::string& overrideKey1, const std::string& overrideKey2)
{
    bool resetPressed = ResetButton(resetKey1, resetKey2);

    std::string checkKey1 = overrideKey1.empty() ? resetKey1 : overrideKey1;
    std::string checkKey2 = overrideKey2.empty() ? resetKey2 : overrideKey2;

    bool isOverridden = _commandLineConfiguration->has(checkKey1);
    if (!checkKey2.empty())
    {
        isOverridden = isOverridden || _commandLineConfiguration->has(checkKey2);
    }

    if (isOverridden)
    {
        OverriddenSettingMarker();
    }

    return resetPressed;
}
