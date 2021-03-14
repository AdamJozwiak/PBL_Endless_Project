#include "Renderable.h"

#include "assert.hpp"

#include "Blender.h"
#include "GraphicsThrowMacros.h"
#include "IndexBuffer.h"

void Renderable::Draw(Graphics& gfx, PassType passType) const
    noexcept(!IS_DEBUG) {
    for (auto& b : binds) {
        switch (passType) {
            case PassType::normal:
                if (b->GetStatus()) {
                    b->Bind(gfx);
                }
                break;
            case PassType::refractive:
                if (b->GetStatus()) {
                    b->Bind(gfx);
                }
                break;
            case PassType::shadowPass:
                if (b->GetStatus()) {
                    b->Bind(gfx);
                }
                break;
        }
    }
    for (auto& b : GetStaticBinds()) {
        switch (passType) {
            case PassType::normal:
                if (b->GetStatus()) {
                    b->Bind(gfx);
                }
                break;
            case PassType::refractive:
                if (b->GetStatus()) {
                    b->Bind(gfx);
                }
                break;
            case PassType::shadowPass:
                if (b->GetStatus()) {
                    b->Bind(gfx);
                }
                break;
        }
    }
    gfx.DrawIndexed(pIndexBuffer->GetCount());

    // Setting blending state back to normal
    static Blender blending = Blender(gfx, false);
    blending.Bind(gfx);
}

void Renderable::AddBind(std::shared_ptr<Bindable> bind) noexcept(!IS_DEBUG) {
    assert("*Must* use AddIndexBuffer to bind index buffer" &&
           typeid(*bind) != typeid(IndexBuffer));
    binds.push_back(bind);
}

void Renderable::AddIndexBuffer(std::shared_ptr<IndexBuffer> ibuf) noexcept(
    !IS_DEBUG) {
    assert("Attempting to add index buffer a second time" &&
           pIndexBuffer == nullptr);
    pIndexBuffer = ibuf.get();
    binds.push_back(std::move(ibuf));
}
