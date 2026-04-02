#pragma once

#include <Poco/Util/MapConfiguration.h>
#include <Poco/Util/PropertyFileConfiguration.h>

#include <string>

class FileChooser;

class SettingsUIHelpers
{
public:
    SettingsUIHelpers() = delete;
    SettingsUIHelpers(
        Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& userConfiguration,
        Poco::AutoPtr<Poco::Util::MapConfiguration>& commandLineConfiguration,
        bool& changed,
        FileChooser& pathChooser);

    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& userConfiguration();
    void markChanged();

    void LabelWithTooltip(const std::string& label, const std::string& tooltipText);
    void PathSetting(const std::string& property);
    void BooleanSetting(const std::string& property, bool defaultValue);
    void IntegerSetting(const std::string& property, int defaultValue, int min, int max);
    void IntegerSettingVec(const std::string& property1, const std::string& property2,
                           int defaultValue1, int defaultValue2, int min, int max);
    void DoubleSetting(const std::string& property, double defaultValue, double min, double max);
    void DoubleSettingWithApply(const std::string& property, double defaultValue, double min, double max,
                                float& tempValue);
    bool ResetButton(const std::string& property1, const std::string& property2 = "");
    void OverriddenSettingMarker();
    bool DrawResetAndOverrideMarker(const std::string& resetKey1, const std::string& resetKey2 = "",
                                    const std::string& overrideKey1 = "", const std::string& overrideKey2 = "");

private:
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& _userConfiguration;
    Poco::AutoPtr<Poco::Util::MapConfiguration>& _commandLineConfiguration;
    bool& _changed;
    FileChooser& _pathChooser;
};
