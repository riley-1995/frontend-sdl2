#include "config/ConfigMigration.h"

#include <array>
#include <string>

namespace
{
struct ConfigKeyMigration
{
    const char* legacyKey;
    const char* canonicalKey;
};

// Keep startup key migrations centralized so future schema updates are append-only.
constexpr std::array<ConfigKeyMigration, 3> kUserConfigKeyMigrations{{
    {"window.fullscreen.exclusive", "window.fullscreen.exclusiveMode"},
    {"fullscreen.width", "window.fullscreen.width"},
    {"fullscreen.height", "window.fullscreen.height"},
}};

bool ApplyUserConfigKeyMigration(Poco::Util::PropertyFileConfiguration& userConfig,
                                 const ConfigKeyMigration& migration)
{
    if (userConfig.has(migration.canonicalKey) || !userConfig.has(migration.legacyKey))
    {
        return false;
    }

    userConfig.setString(migration.canonicalKey, userConfig.getString(migration.legacyKey));
    return true;
}
} // namespace

void ConfigMigration::ApplyUserConfigKeyMigrations(Poco::Util::PropertyFileConfiguration& userConfig,
                                                   Poco::Logger& logger)
{
    int appliedMigrationCount{0};
    for (const auto& migration : kUserConfigKeyMigrations)
    {
        if (ApplyUserConfigKeyMigration(userConfig, migration))
        {
            ++appliedMigrationCount;
            poco_information_f2(logger,
                                "Migrated legacy user config key '%s' to canonical key '%s'.",
                                std::string(migration.legacyKey),
                                std::string(migration.canonicalKey));
        }
    }

    if (appliedMigrationCount > 0)
    {
        poco_information_f1(logger,
                            "Applied %?d startup user config key migration(s).",
                            appliedMigrationCount);
    }
}
