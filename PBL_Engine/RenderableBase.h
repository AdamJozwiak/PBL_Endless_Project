#pragma once
#include "IndexBuffer.h"
#include "Renderable.h"

template <class T>
class RenderableBase : public Renderable {
  protected:
    bool IsStaticInitialized() noexcept { return !staticBinds.empty(); }
    static void AddStaticBind(std::unique_ptr<Bindable> bind) noexcept(
        !IS_DEBUG) {
        assert("*Must* use AddStaticIndexBuffer to bind index buffer" &&
               typeid(*bind) != typeid(IndexBuffer));
        staticBinds.push_back(std::move(bind));
    }
    void AddStaticIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept(
        !IS_DEBUG) {
        assert("Attempting to add index buffer a second time" &&
               pIndexBuffer == nullptr);
        // Every instance of the drawable class has its own pointer to the index
        // buffer, so unlike any other Bindables we have to ensure every
        // additional object has itd own IndexBufferPointer, otherwise only the
        // first one will have one, ande the others will have nullptr, thus
        // crashing the program. SetIndexFromStatic() is responsible for that
        pIndexBuffer = ibuf.get();
        staticBinds.push_back(std::move(ibuf));
    }
    void SetIndexFromStatic() noexcept(!IS_DEBUG) {
        assert("Attempting to add index buffer a second time" &&
               pIndexBuffer == nullptr);
        for (const auto& b : staticBinds) {
            if (const auto p = dynamic_cast<IndexBuffer*>(
                    b.get()))  // Looks for IndexBuffer in staticBinds
            {
                pIndexBuffer = p;
                return;
            }
        }
        assert("Failed to find index buffer in static binds" &&
               pIndexBuffer != nullptr);
    }

  private:
    const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds()
        const noexcept override {
        return staticBinds;
    }

  private:
    inline static std::vector<std::unique_ptr<Bindable>> staticBinds;
};
