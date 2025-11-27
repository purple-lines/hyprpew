#pragma once

#define WLR_USE_UNSTABLE

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/config/ConfigManager.hpp>

#include <unordered_map>
#include <chrono>
#include <memory>

inline HANDLE PHANDLE = nullptr;

struct SClosingWindow {
    PHLWINDOWREF                          window;
    std::chrono::steady_clock::time_point startTime;
    float                                 progress = 0.0f;
    float                                 duration = 0.3f;
    Vector2D                              originalPos;
    Vector2D                              originalSize;
    bool                                  finished = false;
    bool                                  renderingDisabled = false;
    float                                 closeAmount = 0.0f;
};

struct SGlobalState {
    std::unordered_map<uint64_t, std::shared_ptr<SClosingWindow>> closingWindows;

    float animationDuration = 0.3f;
};

inline std::unique_ptr<SGlobalState> g_pGlobalState;
