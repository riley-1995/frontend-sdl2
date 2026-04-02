#pragma once

#include <Poco/Util/MapConfiguration.h>
#include <Poco/Util/PropertyFileConfiguration.h>

#include <string>

class FileChooser;

class SettingsUIHelpers
{
public:
    SettingsUIHelpers() = delete;

    /**
     * @brief Creates shared helpers used by all settings tabs.
     */
    SettingsUIHelpers(
        Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& userConfiguration,
        Poco::AutoPtr<Poco::Util::MapConfiguration>& commandLineConfiguration,
        bool& changed,
        FileChooser& pathChooser);

    /**
     * @brief Returns the mutable user configuration map.
     */
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& userConfiguration();

    /**
     * @brief Marks the settings dialog as dirty.
     */
    void markChanged();

    /**
     * @brief Displays a setting name label and shows a tooltip if hovered.
     * @param label The label text.
     * @param tooltipText The tooltip displayed when hovering the setting name.
     */
    void LabelWithTooltip(const std::string& label, const std::string& tooltipText);

    /**
     * @brief Displays an editable path field, with a directory chooser button.
     * @param property The property name in the config.
     */
    void PathSetting(const std::string& property);

    /**
     * @brief Displays a checkbox.
     * @param property The property name in the config.
     * @param defaultValue Default value for the property if not set.
     */
    void BooleanSetting(const std::string& property, bool defaultValue);

    /**
     * @brief Displays a slider to set an integer min/max value.
     * @param property The property name in the config.
     * @param defaultValue Default value for the property if not set.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     */
    void IntegerSetting(const std::string& property, int defaultValue, int min, int max);

    /**
     * @brief Displays a slider to set two integer min/max values.
     * @param property1 The first property name in the config.
     * @param property2 The second property name in the config.
     * @param defaultValue1 Default value for the first property if not set.
     * @param defaultValue2 Default value for the second property if not set.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     */
    void IntegerSettingVec(const std::string& property1, const std::string& property2,
                           int defaultValue1, int defaultValue2, int min, int max);

    /**
     * @brief Displays a slider to set a double min/max value.
     * @param property The property name in the config.
     * @param defaultValue Default value for the property if not set.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     */
    void DoubleSetting(const std::string& property, double defaultValue, double min, double max);

    /**
     * @brief Displays a slider to select a double min/max value and an "Apply" button to set the value.
     * Useful if the slider affects UI rendering (e.g. scaling).
     * @param property The property name in the config.
     * @param defaultValue Default value for the property if not set.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     * @param tempValue The storage location for the displayed slider value.
     */
    void DoubleSettingWithApply(const std::string& property, double defaultValue, double min, double max,
                                float& tempValue);

    /**
     * @brief Displays a reset button and removes the property from the UI map if clicked.
     * @param property1 First property to reset.
     * @param property2 Optional second property to reset.
     * @return true if the button was pressed, false otherwise.
     */
    bool ResetButton(const std::string& property1, const std::string& property2 = "");

    /**
     * @brief Displays a red note and a tooltip when hovered explaining the setting can't be changed now.
     */
    void OverriddenSettingMarker();

    /**
     * @brief Displays a reset button and override marker for one or two properties.
     * @param resetKey1 Primary property to reset.
     * @param resetKey2 Optional second property to reset.
     * @param overrideKey1 Optional property to check for override marker (defaults to resetKey1 if empty).
     * @param overrideKey2 Optional second property to check for override marker.
     * @return true if reset button was pressed, false otherwise.
     */
    bool DrawResetAndOverrideMarker(const std::string& resetKey1, const std::string& resetKey2 = "",
                                    const std::string& overrideKey1 = "", const std::string& overrideKey2 = "");

private:
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration>& _userConfiguration;
    Poco::AutoPtr<Poco::Util::MapConfiguration>& _commandLineConfiguration;
    bool& _changed;
    FileChooser& _pathChooser;
};
