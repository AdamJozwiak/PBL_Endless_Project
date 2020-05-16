#include "TimerCbuf.h"

TimerCbuf::TimerCbuf(Graphics& gfx, const Renderable& parent, UINT slot)
    : parent(parent) {
    if (!pPcbuf) {
        pPcbuf = std::make_unique<PixelConstantBuffer<Transforms>>(gfx, slot);
    }
    timer.Mark();
}

void TimerCbuf::Bind(Graphics& gfx) noexcept {
    float time = timer.Peek();
    const Transforms tf = {time, 0.0f, 0.0f, 0.0f};
    pPcbuf->Update(gfx, tf);
    pPcbuf->Bind(gfx);
}

std::unique_ptr<PixelConstantBuffer<TimerCbuf::Transforms>> TimerCbuf::pPcbuf;
