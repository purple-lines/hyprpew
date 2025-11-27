#include "globals.hpp"

static SP<HOOK_CALLBACK_FN> g_pCloseWindowHook;
static SP<HOOK_CALLBACK_FN> g_pTickHook;

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
        closingData->closeAmount = easedProgress;
        
        float shrinkRatio = 1.0f - easedProgress;
        float newWidth = closingData->originalSize.x * shrinkRatio;
        float newX = closingData->originalPos.x + (closingData->originalSize.x - newWidth) / 2.0f;
        
        window->m_realPosition->setValueAndWarp(Vector2D(newX, closingData->originalPos.y));
        window->m_realSize->setValueAndWarp(Vector2D(newWidth, closingData->originalSize.y));
        
        window->m_alpha->setValueAndWarp(easedProgress);
        
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
            
            if (window) {
                window->m_fadingOut = false;
                g_pCompositor->closeWindow(window);
            }
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
    
    closingData->originalPos = window->m_position;
    closingData->originalSize = window->m_size;
    
    if (closingData->originalSize.x <= 0 || closingData->originalSize.y <= 0) {
        closingData->originalPos = window->m_realPosition->value();
        closingData->originalSize = window->m_realSize->value();
    }
    
    window->m_originalClosedPos = closingData->originalPos;
    window->m_originalClosedSize = closingData->originalSize;
    
    g_pGlobalState->closingWindows[windowId] = closingData;
    
    window->m_fadingOut = true;
    info.cancelled = true;
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
