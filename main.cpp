#include "globals.hpp"
#include "CurtainPassElement.hpp"

#include <hyprland/src/render/pass/PassElement.hpp>

static SP<HOOK_CALLBACK_FN> g_pCloseWindowHook;
static SP<HOOK_CALLBACK_FN> g_pRenderHook;
static SP<HOOK_CALLBACK_FN> g_pTickHook;

inline float easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

inline float easeOutExpo(float t) {
    return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
}

void onTick(void* self, SCallbackInfo& info, std::any data) {
    if (!g_pGlobalState || g_pGlobalState->closingWindows.empty())
        return;
    
    auto now = std::chrono::steady_clock::now();
    std::vector<uint64_t> toRemove;
    
    for (auto& [windowId, closingData] : g_pGlobalState->closingWindows) {
        if (!closingData || closingData->finished)
            continue;
        
        auto window = closingData->window.lock();
        if (!window) {
            closingData->finished = true;
            toRemove.push_back(windowId);
            continue;
        }
            
        auto elapsed = std::chrono::duration<float>(now - closingData->startTime).count();
        closingData->progress = std::min(1.0f, elapsed / closingData->duration);
        
            float easedProgress = easeOutExpo(closingData->progress);
        
            closingData->closeAmount = easedProgress * 0.5f;
        
        if (closingData->progress >= 1.0f) {
            closingData->finished = true;
            toRemove.push_back(windowId);
        }
        
        for (auto& m : g_pCompositor->m_monitors) {
            if (m)
                g_pHyprRenderer->damageMonitor(m);
        }
    }
    
    for (auto& id : toRemove) {
        auto it = g_pGlobalState->closingWindows.find(id);
        if (it != g_pGlobalState->closingWindows.end()) {
            auto window = it->second->window.lock();
            g_pGlobalState->closingWindows.erase(it);
            
        }
    }
}

void onCloseWindow(void* self, SCallbackInfo& info, std::any data) {
    if (!g_pGlobalState)
        return;
        
    auto window = std::any_cast<PHLWINDOW>(data);
    if (!window)
        return;
    
    uint64_t windowId = (uint64_t)window.get();
    
    if (g_pGlobalState->closingWindows.count(windowId))
        return;
    
    static auto* const PDURATION = (Hyprlang::FLOAT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprpew:duration")->getDataStaticPtr();
    float duration = **PDURATION;
    
    auto closingData = std::make_shared<SClosingWindow>();
    closingData->window = window;
    closingData->startTime = std::chrono::steady_clock::now();
    closingData->duration = duration;
    closingData->progress = 0.0f;
    closingData->closeAmount = 0.0f;
    
    closingData->originalPos = window->m_realPosition->value();
    closingData->originalSize = window->m_realSize->value();
    
    g_pGlobalState->closingWindows[windowId] = closingData;
    
}

void onRenderStage(void* self, SCallbackInfo& info, std::any data) {
    if (!g_pGlobalState || g_pGlobalState->closingWindows.empty())
        return;
    
    auto stage = std::any_cast<eRenderStage>(data);
    
    if (stage != RENDER_POST_WINDOWS)
        return;
    
    auto monitor = g_pHyprOpenGL->m_renderData.pMonitor.lock();
    if (!monitor)
        return;
    
    for (auto& [windowId, closingData] : g_pGlobalState->closingWindows) {
        if (!closingData || closingData->finished)
            continue;
        
        auto window = closingData->window.lock();
        if (!window)
            continue;
        
        if (window->m_monitor.lock() != monitor)
            continue;
        
        CCurtainPassElement::SCurtainData curtainData;
        curtainData.closingWindow = closingData.get();
        curtainData.pMonitor = monitor;
        
        g_pHyprRenderer->m_renderPass.add(makeUnique<CCurtainPassElement>(curtainData));
    }
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprpew] Mismatched headers! Can't proceed.",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprpew] Version mismatch");
    }

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprpew:duration", Hyprlang::FLOAT{0.4f});

    g_pGlobalState = std::make_unique<SGlobalState>();

    g_pCloseWindowHook = HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "closeWindow",
        [](void* self, SCallbackInfo& info, std::any data) { onCloseWindow(self, info, data); }
    );
    
    g_pTickHook = HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "tick",
        [](void* self, SCallbackInfo& info, std::any data) { onTick(self, info, data); }
    );
    
    g_pRenderHook = HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "render",
        [](void* self, SCallbackInfo& info, std::any data) { onRenderStage(self, info, data); }
    );

    HyprlandAPI::reloadConfig();

    HyprlandAPI::addNotification(PHANDLE, "[hyprpew] Initialized! Curtain close animations enabled.",
                                 CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);

    return {"hyprpew", "Smooth curtain-style window close animations", "purplelines", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (g_pGlobalState) {
        g_pGlobalState->closingWindows.clear();
    }
    g_pGlobalState.reset();
}
