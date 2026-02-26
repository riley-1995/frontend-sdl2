#include <gtest/gtest.h>

#include "RenderLoop.h"
#include "ProjectMSDLApplication.h"
#include "ProjectMWrapper.h"
#include "SDLRenderingWindow.h"
#include "AudioCapture.h"
#include "gui/ProjectMGUI.h"

#include <SDL2/SDL.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <Poco/Util/Application.h>

#include <cstdlib>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

namespace {

    // In CI it’s common to have no display server on Linux.
    // Setting a dummy video driver allows SDL_Init to succeed.
    // Note: this may not support real GL contexts, but it’s helpful for event-path tests.
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
    }

    // Inject a key press (down+up).
    void PushKey(SDL_Keycode key)
    {
        SDL_Event e{};
        e.type = SDL_KEYDOWN;
        e.key.keysym.sym = key;
        SDL_PushEvent(&e);

        e = SDL_Event{};
        e.type = SDL_KEYUP;
        e.key.keysym.sym = key;
        SDL_PushEvent(&e);
    }

    // Warp the OS cursor into the target window. This improves reliability vs. only pushing motion events.
    void WarpMouse(SDL_Window* w, int x, int y)
    {
        if (w)
            SDL_WarpMouseInWindow(w, x, y);
    }

    // For SDL -> ImGui input paths, setting windowID is important on Windows.
    // Many backends ignore mouse events that don’t match the active window.
    void PushMouseMove(SDL_Window* w, int x, int y)
    {
        SDL_Event e{};
        e.type = SDL_MOUSEMOTION;
        e.motion.windowID = w ? SDL_GetWindowID(w) : 0;
        e.motion.which = 0;
        e.motion.x = x;
        e.motion.y = y;
        e.motion.xrel = 0;
        e.motion.yrel = 0;
        SDL_PushEvent(&e);
    }

    void PushMouseDown(SDL_Window* w, int x, int y)
    {
        SDL_Event e{};
        e.type = SDL_MOUSEBUTTONDOWN;
        e.button.windowID = w ? SDL_GetWindowID(w) : 0;
        e.button.which = 0;
        e.button.button = SDL_BUTTON_LEFT;
        e.button.state = SDL_PRESSED;
        e.button.clicks = 1;
        e.button.x = x;
        e.button.y = y;
        SDL_PushEvent(&e);
    }

    void PushMouseUp(SDL_Window* w, int x, int y)
    {
        SDL_Event e{};
        e.type = SDL_MOUSEBUTTONUP;
        e.button.windowID = w ? SDL_GetWindowID(w) : 0;
        e.button.which = 0;
        e.button.button = SDL_BUTTON_LEFT;
        e.button.state = SDL_RELEASED;
        e.button.clicks = 1;
        e.button.x = x;
        e.button.y = y;
        SDL_PushEvent(&e);
    }

    // Test-visible state (read by assertions after app exits).
    struct UITestState
    {
        bool overlayVisibleObserved{false};
        bool anyPopupOpened{false};
        bool settingsWindowVisible{false};
        bool finished{false};
        int attempts{0};
    };

    static UITestState g_state;

    // Strong “user perspective” signal that a menu dropdown opened:
    // any ImGui popup at all (avoids brittle label/ID assumptions).
    bool IsAnyPopupOpen()
    {
        return ImGui::IsPopupOpen((ImGuiID)0, ImGuiPopupFlags_AnyPopupId);
    }

    // Optional stronger signal: known window created by a menu action.
    bool IsSettingsWindowVisible()
    {
        // Window name used by this app’s Settings UI.
        ImGuiWindow* w = ImGui::FindWindowByName("Settings###Settings");
        return w != nullptr && !w->Hidden;
    }

} // namespace

// ============================================================
// Test-only RenderLoop implementation
//
// This file is compiled into the UI test binary and linked against the
// *_norenderloop test core lib, so the test provides RenderLoop::Run().
// ============================================================

RenderLoop::RenderLoop()
    : _audioCapture(Poco::Util::Application::instance().getSubsystem<AudioCapture>())
    , _projectMWrapper(Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>())
    , _sdlRenderingWindow(Poco::Util::Application::instance().getSubsystem<SDLRenderingWindow>())
    , _projectMHandle(_projectMWrapper.ProjectM())
    , _playlistHandle(_projectMWrapper.Playlist())
    , _projectMGui(Poco::Util::Application::instance().getSubsystem<ProjectMGUI>())
    , _userConfig(ProjectMSDLApplication::instance().UserConfiguration())
{
}

void RenderLoop::Run()
{
    // Bounded loop ensures the test cannot hang indefinitely.
    constexpr int kMaxFrames = 360;

    // Stable-ish coordinates for menu bar + first dropdown item (800x600 window).
    // If DPI/layout changes, adjust these or add a small y-sweep.
    const int fileX = 30;
    const int fileY = 12;
    const int itemX = 60;
    const int itemY = 50;

    SDL_Window* win = _sdlRenderingWindow.GetRenderingWindow();
    if (win)
        SDL_RaiseWindow(win);

    _sdlRenderingWindow.ShowCursor(true);

    // Ensure overlay is visible from a user’s perspective (press ESC if needed).
    if (!_projectMGui.Visible())
        PushKey(SDLK_ESCAPE);

    for (int frame = 0; frame < kMaxFrames && !g_state.finished; ++frame)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            // Some overlay toggling is handled in production RenderLoop::KeyEvent.
            // Our test harness must emulate that behavior for ESC (but avoid double toggles).
            const bool wasVisible = _projectMGui.Visible();

            _projectMGui.ProcessInput(e);

            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            {
                const bool nowVisible = _projectMGui.Visible();
                if (nowVisible == wasVisible)
                {
                    _projectMGui.Toggle();
                    _sdlRenderingWindow.ShowCursor(_projectMGui.Visible());
                }
            }
        }

        _projectMGui.Draw();
        _sdlRenderingWindow.Swap();

        if (_projectMGui.Visible())
            g_state.overlayVisibleObserved = true;

        // Drive a simple “user flow”: click File, then click first item.
        // Mouse down/up are separated across frames for better ImGui reliability.
        if (g_state.overlayVisibleObserved && win)
        {
            // Attempt 1
            if (frame == 30) { ++g_state.attempts; SDL_RaiseWindow(win); WarpMouse(win, fileX, fileY); PushMouseMove(win, fileX, fileY); }
            if (frame == 31) { PushMouseDown(win, fileX, fileY); }
            if (frame == 32) { PushMouseUp(win, fileX, fileY); }

            if (frame == 50) { WarpMouse(win, itemX, itemY); PushMouseMove(win, itemX, itemY); }
            if (frame == 51) { PushMouseDown(win, itemX, itemY); }
            if (frame == 52) { PushMouseUp(win, itemX, itemY); }

            // Attempt 2 (slight Y offset for DPI/menu-bar variance)
            if (!g_state.anyPopupOpened && frame == 120) { ++g_state.attempts; SDL_RaiseWindow(win); WarpMouse(win, fileX, fileY + 8); PushMouseMove(win, fileX, fileY + 8); }
            if (!g_state.anyPopupOpened && frame == 121) { PushMouseDown(win, fileX, fileY + 8); }
            if (!g_state.anyPopupOpened && frame == 122) { PushMouseUp(win, fileX, fileY + 8); }
        }

        // Deterministic check #1: did a menu/popup open at all?
        if (!g_state.anyPopupOpened && g_state.overlayVisibleObserved)
            g_state.anyPopupOpened = IsAnyPopupOpen();

        // Deterministic check #2: did a known window appear as a result of menu interaction?
        if (!g_state.settingsWindowVisible && g_state.anyPopupOpened)
            g_state.settingsWindowVisible = IsSettingsWindowVisible();

        if (g_state.settingsWindowVisible)
            g_state.finished = true;

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

// Unused RenderLoop methods for this test harness.
void RenderLoop::PollEvents() {}
void RenderLoop::CheckViewportSize() {}
void RenderLoop::KeyEvent(const SDL_KeyboardEvent&, bool) {}
void RenderLoop::ScrollEvent(const SDL_MouseWheelEvent&) {}
void RenderLoop::MouseDownEvent(const SDL_MouseButtonEvent&) {}
void RenderLoop::MouseUpEvent(const SDL_MouseButtonEvent&) {}
void RenderLoop::QuitNotificationHandler(const Poco::AutoPtr<QuitNotification>&) {}


// ============================================================
// The actual UI test (end-to-end from the app entrypoint).
// ============================================================

TEST(SDLUiE2E, Overlay_FileMenu_OpensDropdown_AndIsInteractable)
{
    ConfigureHeadlessIfNeeded();

    ProjectMSDLApplication app;

    std::vector<std::string> args{
        "projectMSDL-ui-test",
        "--width=800",
        "--height=600",
        "--left=0",
        "--top=0",
        // "--testMode"  // include only if already supported by the app
    };

    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto& s : args)
        argv.push_back(s.data());

    int rc = EXIT_FAILURE;

    ASSERT_NO_THROW({
        app.init(static_cast<int>(argv.size()), argv.data());
        rc = app.run();
    });

    EXPECT_EQ(rc, EXIT_SUCCESS) << "App exited unexpectedly (crash/abort).";

    EXPECT_TRUE(g_state.overlayVisibleObserved)
        << "Overlay was never observed visible (ESC toggle may have failed).";

    EXPECT_TRUE(g_state.anyPopupOpened)
        << "No ImGui popup opened after clicking the menu bar (attempts=" << g_state.attempts << ").";

    EXPECT_TRUE(g_state.settingsWindowVisible)
        << "Menu interaction did not open the expected Settings window (UI may not be interactable).";
}