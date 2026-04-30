#pragma once

#include <Poco/Logger.h>
#include <Poco/Util/PropertyFileConfiguration.h>

namespace ConfigMigration
{
/**
 * @brief Applies startup migrations from legacy user-config keys to canonical keys.
 *
 * The migration table is append-only: add new key pairs as configuration naming
 * evolves in follow-up issues, while keeping migration behavior centralized.
 *
 * @param userConfig Loaded user configuration to migrate in-place.
 * @param logger Logger used to report applied migrations.
 */
void ApplyUserConfigKeyMigrations(Poco::Util::PropertyFileConfiguration& userConfig, Poco::Logger& logger);

} // namespace ConfigMigration
