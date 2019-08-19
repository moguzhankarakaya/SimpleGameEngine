#pragma once
#include <cstdint>

#include <DXUT.h>
#include <d3dx11effect.h>
#include <D3DUtilities.h>
#include <DDSTextureLoader.h>


enum class Type { ROTATION, TRANSLATION };

class AbsAnimation
{
public:
	virtual ~AbsAnimation() {}
	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void reset() = 0;
	virtual bool isPlaying() const = 0;
	virtual std::string getMeshName() const = 0;
	virtual Type getAnimationType() const = 0;
	virtual DirectX::XMMATRIX getAnimationMat(float) = 0;
};