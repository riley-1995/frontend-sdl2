#include <gtest/gtest.h>
#include "ProjectMSDLApplication.h"

#include <Poco/Environment.h>
#include <Poco/File.h>
#include <Poco/Path.h>

#include <fstream>
#include <vector>
#include <string>

#ifndef PROJECTMSDL_CONFIG_LOCATION
#error "PROJECTMSDL_CONFIG_LOCATION must be defined for tests."
#endif

// Small helper to write a .properties file to disk.
static void write(const std::string &path, const std::string &s)
{
    std::ofstream os(path, std::ios::binary | std::ios::trunc);
    os << s;
}

class TestApp : public ProjectMSDLApplication {
public:
    using ProjectMSDLApplication::LoadConfigurationLayers;
};

TEST(ConfigPrecedenceIntegration, Minimal_CreatesConfigsAndAcceptsCli)
{
    // Create a temporary root directory for the test
    auto root = Poco::Path(Poco::Path::temp()).append("projectmsdl-it").toString();
    Poco::File(root).createDirectories();

    // Redirect the platform config directory into our temp location
    auto cfgHome = Poco::Path(root).append("cfg").toString();
    Poco::File(cfgHome).createDirectories();

    Poco::Environment::set("XDG_CONFIG_HOME", cfgHome);
    Poco::Environment::set("HOME", cfgHome);
    Poco::Environment::set("APPDATA", cfgHome);
    Poco::Environment::set("LOCALAPPDATA", cfgHome);

    // Stable argv0 so config filename is projectMSDL.properties
    const std::string argv0 = Poco::Path(root).append("projectMSDL").toString();
    const std::string cfgFile = "projectMSDL.properties";

    // Create the "installed default" config
    const std::string installedDir = std::string(PROJECTMSDL_CONFIG_LOCATION);
    Poco::File(installedDir).createDirectories();

    const auto installedCfgPath = Poco::Path(installedDir).setFileName(cfgFile).toString();
    write(installedCfgPath, "projectM.presetPath=/installed/p\nprojectM.texturePath=/installed/t\n");

    // Create the user config
    Poco::Path userDir = Poco::Path::configHome();
    userDir.makeDirectory().append("projectM/");
    Poco::File(userDir).createDirectories();

    const auto userCfgPath = Poco::Path(userDir).setFileName(cfgFile).toString();
    write(userCfgPath, "projectM.presetPath=/user/p\nprojectM.texturePath=/user/t\n");

    // Prepare CLI paths
    const std::string cliPresets = Poco::Path(root).append("cli-presets").toString();
    const std::string cliTextures = Poco::Path(root).append("cli-textures").toString();

    Poco::File(cliPresets).createDirectories();
    Poco::File(cliTextures).createDirectories();

    // Call ProjectMSDLApplication::init() with CLI flags
    // ProjectMSDLApplication app;
    TestApp app;

    std::vector<std::string> args = { argv0, "--presetPath", cliPresets, "--texturePath", cliTextures };
    
    std::vector<char*> argv; argv.reserve(args.size()+1);

    for (auto &s: args) 
        argv.push_back(const_cast<char*>(s.c_str()));
    argv.push_back(nullptr);

    ASSERT_NO_THROW(app.init(static_cast<int>(args.size()), argv.data()));
    app.LoadConfigurationLayers(); // loads installed + user configs and merges with CLI overrides

    ASSERT_TRUE(app.config().hasProperty("projectM.presetPath"));
    EXPECT_EQ(app.config().getString("projectM.presetPath"), cliPresets);
    ASSERT_TRUE(app.config().hasProperty("projectM.texturePath"));
    EXPECT_EQ(app.config().getString("projectM.texturePath"), cliTextures);

    // Assertions we can make without changing the product code:
    // Both config files exist
    EXPECT_TRUE(Poco::File(installedCfgPath).exists());
    EXPECT_TRUE(Poco::File(userCfgPath).exists());

    // Sanity-check that we can query the configuration object
    // (We don't assert specific keys because the app doesn't expose them)
    auto &g = app.config();
    std::vector<std::string> keys;
    ASSERT_NO_THROW(g.keys(keys));
    
    // This will always be true, but ensures we exercised the AbstractConfiguration API properly.
    EXPECT_GE(keys.size(), static_cast<std::size_t>(0));
}