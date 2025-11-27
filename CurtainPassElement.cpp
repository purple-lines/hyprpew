#include "CurtainPassElement.hpp"
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/Compositor.hpp>

CCurtainPassElement::CCurtainPassElement(const CCurtainPassElement::SCurtainData& data_) : data(data_) {
    ;
}

void CCurtainPassElement::draw(const CRegion& damage) {
    if (!data.closingWindow || !data.pMonitor)
        return;
    
    renderCurtainEffect();
}

void CCurtainPassElement::renderCurtainEffect() {
    auto closingData = data.closingWindow;
    if (!closingData)
        return;
    
    auto window = closingData->window.lock();
    if (!window)
        return;
    
    auto monitor = data.pMonitor;
    if (!monitor)
        return;
    
    float closeAmount = closingData->closeAmount;
    
    if (closeAmount <= 0.001f || closeAmount >= 0.5f)
        return;
    
    Vector2D windowPos = closingData->originalPos - monitor->m_position;
    Vector2D windowSize = closingData->originalSize;
    
    float scale = monitor->m_scale;
    
    float leftCurtainWidth = closeAmount * windowSize.x;
    float rightCurtainStart = windowSize.x - leftCurtainWidth;
    
    CBox leftCurtainBox = {
        windowPos.x,
        windowPos.y,
        leftCurtainWidth,
        windowSize.y
    };
    
    CBox rightCurtainBox = {
        windowPos.x + rightCurtainStart,
        windowPos.y,
        leftCurtainWidth,
        windowSize.y
    };
    
    leftCurtainBox.scale(scale).round();
    rightCurtainBox.scale(scale).round();
    
    float curtainAlpha = 0.85f * (closeAmount / 0.5f);
    
    CHyprColor curtainColor = CHyprColor{0.0f, 0.0f, 0.0f, curtainAlpha};
    
    g_pHyprOpenGL->renderRect(leftCurtainBox, curtainColor, {});
    
    g_pHyprOpenGL->renderRect(rightCurtainBox, curtainColor, {});
    
    float edgeWidth = 2.0f * scale;
    float edgeAlpha = 0.3f * (1.0f - closeAmount / 0.5f);
    
    if (edgeAlpha > 0.01f) {
        CHyprColor edgeColor = CHyprColor{1.0f, 1.0f, 1.0f, edgeAlpha};
        
        CBox leftEdgeBox = {
            (windowPos.x + leftCurtainWidth - edgeWidth / scale) * scale,
            windowPos.y * scale,
            edgeWidth,
            windowSize.y * scale
        };
        leftEdgeBox.round();
        g_pHyprOpenGL->renderRect(leftEdgeBox, edgeColor, {});
        
        CBox rightEdgeBox = {
            (windowPos.x + rightCurtainStart) * scale,
            windowPos.y * scale,
            edgeWidth,
            windowSize.y * scale
        };
        rightEdgeBox.round();
        g_pHyprOpenGL->renderRect(rightEdgeBox, edgeColor, {});
    }
}

bool CCurtainPassElement::needsLiveBlur() {
    return false;
}

bool CCurtainPassElement::needsPrecomputeBlur() {
    return false;
}
