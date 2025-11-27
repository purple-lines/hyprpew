#pragma once

#include "globals.hpp"
#include <hyprland/src/render/pass/PassElement.hpp>

class CCurtainPassElement : public IPassElement {
  public:
    struct SCurtainData {
        SClosingWindow* closingWindow = nullptr;
        PHLMONITOR      pMonitor;
    };

    CCurtainPassElement(const SCurtainData& data_);
    virtual ~CCurtainPassElement() = default;

    virtual void        draw(const CRegion& damage);
    virtual bool        needsLiveBlur();
    virtual bool        needsPrecomputeBlur();

    virtual const char* passName() {
        return "CCurtainPassElement";
    }

  private:
    SCurtainData data;
    
    void renderCurtainEffect();
};
