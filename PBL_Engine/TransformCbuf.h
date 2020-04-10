#pragma once
#include "ConstantBuffers.h"
#include "Renderable.h"
#include <DirectXMath.h>

class TransformCbuf : public Bindable
{
public:
	TransformCbuf(Graphics& gfx, const Renderable& parent);
	void Bind(Graphics& gfx) noexcept override;
private:
	static std::unique_ptr<VertexConstantBuffer<DirectX::XMMATRIX>> pVcbuf; // dynamically allocated pointer to ConstantBuffer
	const Renderable& parent;
};