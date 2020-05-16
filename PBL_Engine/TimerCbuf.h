#pragma once
#include <DirectXMath.h>

#include "ConstantBuffers.h"
#include "Renderable.h"
#include "Timer.h"

class TimerCbuf : public Bindable {
  private:
    struct Transforms {
        float time;
        float emptyFloat1;
        float emptyFloat2;
        float emptyFloat3;
    };

  public:
    TimerCbuf(Graphics& gfx, const Renderable& parent, UINT slot = 0u);
    void Bind(Graphics& gfx) noexcept override;

  private:
    static std::unique_ptr<PixelConstantBuffer<Transforms>> pPcbuf;
    const Renderable& parent;
    Timer timer;
};