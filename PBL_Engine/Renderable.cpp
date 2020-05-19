#include "Renderable.h"

#include <cassert>

#include "GraphicsThrowMacros.h"
#include "IndexBuffer.h"

void Renderable::Draw(Graphics& gfx) const noexcept(!IS_DEBUG) {
    for (auto& b : binds) {
        b->Bind(gfx);
    }
    for (auto& b : GetStaticBinds()) {
        b->Bind(gfx);
    }
    gfx.DrawIndexed(pIndexBuffer->GetCount());
}

void Renderable::AddBind(std::shared_ptr<Bindable> bind) noexcept(!IS_DEBUG) {
    assert("*Must* use AddIndexBuffer to bind index buffer" &&
           typeid(*bind) != typeid(IndexBuffer));
    binds.push_back(bind);
}

void Renderable::AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept(
    !IS_DEBUG) {
    assert("Attempting to add index buffer a second time" &&
           pIndexBuffer == nullptr);
    pIndexBuffer = ibuf.get();
    binds.push_back(std::move(ibuf));
}
