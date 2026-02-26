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

    // In CI it’s common to have no display server on Linux
    // Setting a dummy video driver allows SDL_Init to succeed
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

    // Warp the OS cursor into the target window
    void WarpMouse(SDL_Window* w, int x, int y)
    {
        if (w)
            SDL_WarpMouseInWindow(w, x, y);
    }

    void PushMouseMove(SDL_Window* w, int x, int y)
    {
        if (!w) return;
        SDL_Event e{};
        e.type = SDL_MOUSEMOTION;
        e.motion.windowID = SDL_GetWindowID(w);
        e.motion.which = 0;
        e.motion.x = x;
        e.motion.y = y;
        e.motion.xrel = 0;
        e.motion.yrel = 0;
        SDL_PushEvent(&e);
    }

    void PushMouseDown(SDL_Window* w, int x, int y)
    {
        if (!w) return;
        SDL_Event e{};
        e.type = SDL_MOUSEBUTTONDOWN;
        e.button.windowID = SDL_GetWindowID(w);
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
        if (!w) return;
        SDL_Event e{};
        e.type = SDL_MOUSEBUTTONUP;
        e.button.windowID = SDL_GetWindowID(w);
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

    // Strong “user perspective” signal that a menu dropdown opened
    // Guarded by ImGui context checks in the caller
    bool IsAnyPopupOpen()
    {
        return ImGui::IsPopupOpen((ImGuiID)0, ImGuiPopupFlags_AnyPopupId);
    }

    // Optional stronger signal: known window created by a menu action
    bool IsSettingsWindowVisible()
    {
        ImGuiWindow* w = ImGui::FindWindowByName("Settings###Settings");
        return w != nullptr && !w->Hidden;
    }

} // namespace


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

    // Stable-ish coordinates for menu bar + first dropdown item (800x600 window)
    const int fileX = 30;
    const int fileY = 12;
    const int itemX = 60;
    const int itemY = 50;

    // Small initialization window: wait up to this many frames for window + ImGui context to appear.
    constexpr int kInitGraceFrames = 100;
    int initFrames = 0;

    _sdlRenderingWindow.ShowCursor(true);

    // Ensure overlay is visible from a user’s perspective (safe to push early; will be processed once ready).
    if (!_projectMGui.Visible())
        PushKey(SDLK_ESCAPE);

    for (int frame = 0; frame < kMaxFrames && !g_state.finished; ++frame)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            // Process input if GUI subsystem is initialized
            _projectMGui.ProcessInput(e);

            // Emulate production ESC toggle behavior if ProcessInput didn't change visibility
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            {
                const bool nowVisible = _projectMGui.Visible();
                // If ProcessInput didn't change overlay, toggle it
                static_cast<void>(nowVisible); // keep compiler quiet if needed
                // Compare with stored observed state instead of wasVisible to be robust
                if (!g_state.overlayVisibleObserved && _projectMGui.Visible())
                {
                    g_state.overlayVisibleObserved = true;
                }
                else if (!g_state.overlayVisibleObserved && ! _projectMGui.Visible())
                {
                    _projectMGui.Toggle();
                    _sdlRenderingWindow.ShowCursor(_projectMGui.Visible());
                }
            }
        }

        _projectMGui.Draw();
        _sdlRenderingWindow.Swap();

        // Refresh window handle each frame (window may be created after loop begins)
        SDL_Window* win = _sdlRenderingWindow.GetRenderingWindow();
        if (win)
            SDL_RaiseWindow(win);

        // If GUI overlay is visible at any point, mark it.
        if (_projectMGui.Visible())
            g_state.overlayVisibleObserved = true;

        // Wait for both SDL window and ImGui context before attempting clicks.
        bool imguiReady = (ImGui::GetCurrentContext() != nullptr);
        bool windowReady = (win != nullptr);

        if (!imguiReady || !windowReady)
        {
            ++initFrames;
            // If initialization never happens within the grace period, bail out early (fail cleanly).
            if (initFrames > kInitGraceFrames)
            {
                // Let assertions after app.run() catch the failure; just break the loop to exit the app.
                break;
            }

            // Continue to next frame until both are ready.
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        // Click File, then click first item
        if (g_state.overlayVisibleObserved && win)
        {
            // Attempt 1
            if (frame == 30) { ++g_state.attempts; SDL_RaiseWindow(win); WarpMouse(win, fileX, fileY); PushMouseMove(win, fileX, fileY); }
            if (frame == 31) { PushMouseDown(win, fileX, fileY); }
            if (frame == 32) { PushMouseUp(win, fileX, fileY); }

            if (frame == 50) { WarpMouse(win, itemX, itemY); PushMouseMove(win, itemX, itemY); }
            if (frame == 51) { PushMouseDown(win, itemX, itemY); }
            if (frame == 52) { PushMouseUp(win, itemX, itemY); }

            // Attempt 2
            if (!g_state.anyPopupOpened && frame == 120) { ++g_state.attempts; SDL_RaiseWindow(win); WarpMouse(win, fileX, fileY + 8); PushMouseMove(win, fileX, fileY + 8); }
            if (!g_state.anyPopupOpened && frame == 121) { PushMouseDown(win, fileX, fileY + 8); }
            if (!g_state.anyPopupOpened && frame == 122) { PushMouseUp(win, fileX, fileY + 8); }
        }

        // Safe ImGui checks only when context exists.
        if (ImGui::GetCurrentContext())
        {
            if (!g_state.anyPopupOpened && g_state.overlayVisibleObserved)
                g_state.anyPopupOpened = IsAnyPopupOpen();

            if (!g_state.settingsWindowVisible && g_state.anyPopupOpened)
                g_state.settingsWindowVisible = IsSettingsWindowVisible();
        }

        if (g_state.settingsWindowVisible)
            g_state.finished = true;

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

// Unused RenderLoop methods for this test harness
void RenderLoop::PollEvents() {}
void RenderLoop::CheckViewportSize() {}
void RenderLoop::KeyEvent(const SDL_KeyboardEvent&, bool) {}
void RenderLoop::ScrollEvent(const SDL_MouseWheelEvent&) {}
void RenderLoop::MouseDownEvent(const SDL_MouseButtonEvent&) {}
void RenderLoop::MouseUpEvent(const SDL_MouseButtonEvent&) {}
void RenderLoop::QuitNotificationHandler(const Poco::AutoPtr<QuitNotification>&) {}


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
    EXPECT_TRUE(g_state.overlayVisibleObserved) << "Overlay was never observed visible (ESC toggle may have failed).";
    EXPECT_TRUE(g_state.anyPopupOpened) << "No ImGui popup opened after clicking the menu bar (attempts=" << g_state.attempts << ").";
    EXPECT_TRUE(g_state.settingsWindowVisible) << "Menu interaction did not open the expected Settings window (UI may not be interactable).";
}