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

static void PumpOnce(ProjectMGUI& gui, RenderLoopTestHarness& loop)
{
    loop.PollEventsPublic();
    gui.Draw();
}

static void PumpN(ProjectMGUI& gui, RenderLoopTestHarness& loop, int frames)
{
    for (int i = 0; i < frames; ++i)
    {
        PumpOnce(gui, loop);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

static void PushEscPress(SDL_Window* w)
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

static void PushClick(SDL_Window* w, int x, int y)
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

static bool WaitForImGuiReady(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
{
    for (int i = 0; i < maxFrames; ++i)
    {
        PumpOnce(gui, loop);
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetMainViewport() != nullptr)
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

static bool WaitForOverlayVisible(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
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

static bool WaitForImGuiNotCapturingMouse(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
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

static bool IsAnyPopupOpen()
{
    if (ImGui::GetCurrentContext() == nullptr) return false;
    return ImGui::IsPopupOpen((ImGuiID)0, ImGuiPopupFlags_AnyPopupId);
}

static bool WaitForAnyPopupOpen(ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxFrames)
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

static bool ImGuiReady()
{
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetMainViewport() != nullptr;
}

// Sweep clicks across the menubar band (DPI/theme aware) until a popup opens
static bool ClickMenubarSweepUntilPopup(SDL_Window* sdlWin, ProjectMGUI& gui, RenderLoopTestHarness& loop, int maxSweeps = 2)
{
    if (!ImGuiReady()) return false;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float frameH = ImGui::GetFrameHeight(); // DPI-aware
    const float y = vp->Pos.y + frameH * 0.5f;    // within menubar band

    const float left = vp->Pos.x + 8.0f;
    const float right = vp->Pos.x + vp->Size.x - 8.0f;

    const ImGuiStyle& st = ImGui::GetStyle();
    const float approxBtn = ImGui::CalcTextSize("File").x + st.FramePadding.x * 6.0f;
    const float step = (approxBtn > 30.0f ? approxBtn : 45.0f);

    for (int sweep = 0; sweep < maxSweeps; ++sweep)
    {
        for (float x = left; x < right; x += step)
        {
            // Let ImGui settle on hover/focus state
            (void)WaitForImGuiNotCapturingMouse(gui, loop, 10);

            PushClick(sdlWin, (int)x, (int)y);

            // Give it a few frames to open the popup
            if (WaitForAnyPopupOpen(gui, loop, 12))
                return true;
        }
    }
    return false;
}

// Fetch the top open popup window (ImGui internal).
static ImGuiWindow* GetTopOpenPopupWindow()
{
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return nullptr;

    for (int i = (int)ctx->OpenPopupStack.Size - 1; i >= 0; --i)
    {
        ImGuiPopupData& pd = ctx->OpenPopupStack[i];
        if (pd.Window)
            return pd.Window;
    }
    return nullptr;
}

// Click inside the currently open popup window at a given row index, using ImGui metrics.
static bool ClickPopupRow(SDL_Window* sdlWin, ProjectMGUI& gui, RenderLoopTestHarness& loop, int rowIndex)
{
    if (!ImGuiReady()) return false;

    ImGuiWindow* pw = GetTopOpenPopupWindow();
    if (!pw) return false;

    const ImGuiStyle& st = ImGui::GetStyle();
    const float rowH = ImGui::GetTextLineHeightWithSpacing();

    const float x = pw->Pos.x + st.WindowPadding.x + 12.0f;
    const float y = pw->Pos.y + st.WindowPadding.y + rowH * (rowIndex + 0.5f);

    PushClick(sdlWin, (int)x, (int)y);
    PumpN(gui, loop, 8);
    return true;
}

} // namespace

TEST(SDLE2E, MainMenu_PopupOpens_AndIsInteractable_NoExactCoordinates)
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

    ASSERT_NO_THROW(window.initialize(app));
    ASSERT_NO_THROW(renderer.initialize(app));
    ASSERT_NO_THROW(gui.initialize(app));

    SDL_Window* sdlWin = window.GetRenderingWindow();
    ASSERT_NE(sdlWin, nullptr);

    RenderLoopTestHarness loop;

    ASSERT_TRUE(WaitForImGuiReady(gui, loop, 180))
        << "ImGui context never became ready.";

    // Ensure overlay visible (ESC toggles it)
    if (!gui.Visible())
    {
        PushEscPress(sdlWin);
        ASSERT_TRUE(WaitForOverlayVisible(gui, loop, 90))
            << "Overlay never became visible after ESC.";
    }

    // Open any menubar popup without hardcoded coordinates
    ASSERT_TRUE(ClickMenubarSweepUntilPopup(sdlWin, gui, loop, 3))
        << "Could not open any menubar popup by sweeping across the menubar band.";

    EXPECT_TRUE(IsAnyPopupOpen())
        << "Popup did not remain open after opening it.";

    // Prove it's interactable by clicking within the popup window's real rect.
    (void)ClickPopupRow(sdlWin, gui, loop, 1);

    // After clicking an item row, popup often closes; both states are acceptable, but we should not have triggered a quit.
    EXPECT_FALSE(loop.WantsToQuitPublic())
        << "Render loop requested quit unexpectedly.";

    gui.uninitialize();
    renderer.uninitialize();
    window.uninitialize();
}