#pragma once
#include <vector>
#include <cstdint>

#include <DXUT.h>
#include <d3dx11effect.h>
#include <D3DUtilities.h>
#include <DDSTextureLoader.h>

#include "T3d.h"
#include "Animation.h"
#include "AbsAnimation.h"

class ComplexAnimation : public AbsAnimation
{
public:
	ComplexAnimation(UINT numberOfAnimations, std::vector<AbsAnimation*>& animations);

	~ComplexAnimation(void);

	DirectX::XMMATRIX getAnimationMat(float fElapsedTime);

	bool isPlaying() const { return isAnimationRunning; }

	std::string getMeshName() const { return m_animations[currentAnimation]->getMeshName(); }

	Type getAnimationType() const { return m_animations[currentAnimation]->getAnimationType(); }

	void pause();

	void reset();

	void play();
	
private:

	void calculateAnimation(float fElapsedTime);

private:
	UINT m_numberOfAnimations;
	std::vector<AbsAnimation*> m_animations;

	bool isAnimationRunning;
	UINT currentAnimation;
	
	DirectX::XMMATRIX lastAnimationState;
};