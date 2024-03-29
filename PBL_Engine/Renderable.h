#pragma once
#include <DirectXMath.h>

#include "Graphics.h"

enum class PassType { normal, shadowPass, refractive };
class Bindable;

class Renderable {
    template <class T>
    friend class RenderableBase;

  public:
    Renderable() = default;
    Renderable(const Renderable&) = delete;
    virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
    void Draw(Graphics& gfx, PassType passType = PassType::normal) const noexcept(!IS_DEBUG);
    virtual void Update(float dt) noexcept {};
    virtual ~Renderable() = default;

  protected:
    template <class T>
    T* QueryBindable() noexcept {
        for (auto& pb : binds) {
            if (auto pt = dynamic_cast<T*>(pb.get())) {
                return pt;
            }
        }
        return nullptr;
    }
    void AddBind(std::shared_ptr<Bindable> bind) noexcept(!IS_DEBUG);
    void AddIndexBuffer(std::shared_ptr<class IndexBuffer> ibuf) noexcept(
        !IS_DEBUG);

  private:
    virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds()
        const noexcept = 0;

  private:
    const IndexBuffer* pIndexBuffer = nullptr;
    std::vector<std::shared_ptr<Bindable>> binds;
};
