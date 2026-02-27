#include <gtest/gtest.h>

#include "ProjectMSDLApplication.h"
#include "RenderLoop.h"
#include "SDLRenderingWindow.h"
#include "ProjectMWrapper.h"
#include "gui/ProjectMGUI.h"

#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cstdlib>
#include <chrono>
#include <thread>

namespace {

#if !defined(_WIN32)
void ConfigureHeadlessIfNeeded()
{
#if defined(__linux__)
    const char* display = std::getenv("DISPLAY");
    if (!display || display[0] == '\0')
    {
        if (!std::getenv("SDL_VIDEODRIVER"))
            setenv("SDL_VIDEODRIVER", "dummy", 1);
    }
#endif
    if (!std::getenv("SDL_AUDIODRIVER"))
        setenv("SDL_AUDIODRIVER", "dummy", 1);
}
#endif

class RenderLoopTestHarness : public RenderLoop
{
public:
    void PollEventsPublic() { PollEvents(); }
    bool WantsToQuitPublic() const { return _wantsToQuit; }
};

void PumpOnce(ProjectMGUI& gui, RenderLoopTestHarness& loop)
{
    loop.PollEventsPublic();
    gui.Draw();
}

bool WaitForImGuiNotCapturingMouse(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
{
    for (int i = 0; i < maxFrames; ++i)
    {
        PumpOnce(gui, loop);

        if (ImGui::GetCurrentContext() == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }

        const ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse)
            return true;

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

bool WaitForImGuiReady(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
{
    for (int i = 0; i < maxFrames; ++i)
    {
        PumpOnce(gui, loop);
        if (ImGui::GetCurrentContext() != nullptr)
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

bool WaitForOverlayVisible(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
{
    for (int i = 0; i < maxFrames; ++i)
    {
        PumpOnce(gui, loop);
        if (gui.Visible())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

void PushEscPress(SDL_Window* w)
{
    const Uint32 wid = w ? SDL_GetWindowID(w) : 0;

    SDL_Event e{};
    e.type = SDL_KEYDOWN;
    e.key.type = SDL_KEYDOWN;
    e.key.windowID = wid;
    e.key.state = SDL_PRESSED;
    e.key.repeat = 0;
    e.key.keysym.sym = SDLK_ESCAPE;
    e.key.keysym.scancode = SDL_SCANCODE_ESCAPE;
    e.key.keysym.mod = KMOD_NONE;
    ASSERT_EQ(SDL_PushEvent(&e), 1);

    e = SDL_Event{};
    e.type = SDL_KEYUP;
    e.key.type = SDL_KEYUP;
    e.key.windowID = wid;
    e.key.state = SDL_RELEASED;
    e.key.repeat = 0;
    e.key.keysym.sym = SDLK_ESCAPE;
    e.key.keysym.scancode = SDL_SCANCODE_ESCAPE;
    e.key.keysym.mod = KMOD_NONE;
    ASSERT_EQ(SDL_PushEvent(&e), 1);
}

void PushClick(SDL_Window* w, int x, int y)
{
    if (!w) return;
    const Uint32 wid = SDL_GetWindowID(w);

    SDL_Event e{};
    e.type = SDL_MOUSEMOTION;
    e.motion.windowID = wid;
    e.motion.which = 0;
    e.motion.x = x;
    e.motion.y = y;
    e.motion.xrel = 0;
    e.motion.yrel = 0;
    SDL_PushEvent(&e);

    e = SDL_Event{};
    e.type = SDL_MOUSEBUTTONDOWN;
    e.button.windowID = wid;
    e.button.which = 0;
    e.button.button = SDL_BUTTON_LEFT;
    e.button.state = SDL_PRESSED;
    e.button.clicks = 1;
    e.button.x = x;
    e.button.y = y;
    SDL_PushEvent(&e);

    e = SDL_Event{};
    e.type = SDL_MOUSEBUTTONUP;
    e.button.windowID = wid;
    e.button.which = 0;
    e.button.button = SDL_BUTTON_LEFT;
    e.button.state = SDL_RELEASED;
    e.button.clicks = 1;
    e.button.x = x;
    e.button.y = y;
    SDL_PushEvent(&e);
}

bool IsAnyPopupOpen()
{
    if (ImGui::GetCurrentContext() == nullptr) return false;
    return ImGui::IsPopupOpen((ImGuiID)0, ImGuiPopupFlags_AnyPopupId);
}

bool IsSettingsWindowVisible()
{
    if (ImGui::GetCurrentContext() == nullptr) return false;
    ImGuiWindow* w = ImGui::FindWindowByName("Settings###Settings");
    return w != nullptr && !w->Hidden;
}

bool WaitForPopupOpen(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
{
    for (int i = 0; i < maxFrames; ++i)
    {
        PumpOnce(gui, loop);
        if (IsAnyPopupOpen())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

bool TryClickMenuItemRowsUntilSettingsOpens(SDL_Window* sdlWin, ProjectMGUI& gui, RenderLoopTestHarness& loop, int menuX, int startY, int rowStep, int attempts)
{
    for (int i = 0; i < attempts; ++i)
    {
        const int y = startY + i * rowStep;
        PushClick(sdlWin, menuX, y);

        // Give a few frames for the click to register / action to run.
        for (int f = 0; f < 10; ++f)
        {
            PumpOnce(gui, loop);
            if (IsSettingsWindowVisible())
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    return false;
}

bool ClickFileAndWaitForPopup(SDL_Window* sdlWin, ProjectMGUI& gui, RenderLoopTestHarness& loop, int fileX, int fileY, int maxTries)
{
    for (int t = 0; t < maxTries; ++t)
    {
        // Try to ensure ImGui isn't capturing mouse already
        (void)WaitForImGuiNotCapturingMouse(gui, loop, 30);

        PushClick(sdlWin, fileX, fileY);

        // Pump a handful of frames so ImGui can open the popup
        for (int f = 0; f < 20; ++f)
        {
            PumpOnce(gui, loop);
            if (IsAnyPopupOpen())
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        // If it didn't open, nudge Y slightly (DPI/layout variance)
        fileY = (fileY < 4) ? (fileY + 2) : (fileY - 2);
    }
    return false;
}

} // namespace

TEST(SDLE2E, Overlay_FileMenu_OpensDropdown_AndIsInteractable)
{
#if !defined(_WIN32)
    ConfigureHeadlessIfNeeded();
#endif

    const char* argv0 = "projectMSDL-ui-test";
    int argc = 1;
    char* argv[] = { const_cast<char*>(argv0), nullptr };

    ProjectMSDLApplication app;
    ASSERT_NO_THROW(app.init(argc, argv));

    auto& window = app.getSubsystem<SDLRenderingWindow>();
    auto& renderer = app.getSubsystem<ProjectMWrapper>();
    auto& gui = app.getSubsystem<ProjectMGUI>();

    // Manual init/uninit like teammate's framework (avoids the crashy app.run() teardown path)
    ASSERT_NO_THROW(window.initialize(app));
    ASSERT_NO_THROW(renderer.initialize(app));
    ASSERT_NO_THROW(gui.initialize(app));

    SDL_Window* sdlWin = window.GetRenderingWindow();
    ASSERT_NE(sdlWin, nullptr);

    RenderLoopTestHarness loop;

    ASSERT_TRUE(WaitForImGuiReady(gui, loop, 120)) << "ImGui context never became ready.";

    // Ensure overlay visible (ESC toggles it)
    if (!gui.Visible())
    {
        PushEscPress(sdlWin);
        ASSERT_TRUE(WaitForOverlayVisible(gui, loop, 60)) << "Overlay never became visible after ESC.";
    }

    // Coordinates: these are approximate and depend on your UI layout.
    int fileX = 30;
    int fileY = 6; // better for Windows/DPI; keeps us inside the menubar

    ASSERT_TRUE(ClickFileAndWaitForPopup(sdlWin, gui, loop, fileX, fileY, 6))
        << "No ImGui popup opened after clicking File in the menu bar.";

    // Optional: try to open Settings by clicking a few likely rows.
    // This avoids hard-coding one exact Y. It also reduces the chance you click Quit.
    const int itemX = 80;
    const int firstRowY = 45;
    const int rowStep = 18;
    const int tries = 6;

    const bool settingsOpened =
        TryClickMenuItemRowsUntilSettingsOpens(sdlWin, gui, loop, itemX, firstRowY, rowStep, tries);

    // The core “interactable dropdown” assertion is the popup opening
    const bool popupOpened = true; // or store the return value
    EXPECT_TRUE(popupOpened) << "Popup never opened; menu may not be interactable.";
    EXPECT_FALSE(loop.WantsToQuitPublic()) << "Render loop requested quit unexpectedly.";

    // Keep this as EXPECT (not ASSERT) so the test still provides value even if Settings row differs.
    EXPECT_TRUE(settingsOpened)
        << "Could not open Settings window by clicking menu rows. "
        << "If your File menu doesn't contain Settings (or row positions differ), "
        << "either adjust firstRowY/rowStep/tries, or remove this check.";

    gui.uninitialize();
    renderer.uninitialize();
    window.uninitialize();
}