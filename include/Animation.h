#pragma once
#include <cstdint>

#include <DXUT.h>
#include <D3DUtilities.h>
#include <d3dx11effect.h>
#include <DDSTextureLoader.h>

#include "T3d.h"
#include "AbsAnimation.h"

class Animation : public AbsAnimation
{
public:

	Animation(std::string meshName, Type type, DirectX::XMFLOAT3 animation, float duration);

	~Animation(void);

	DirectX::XMMATRIX getAnimationMat(float fElapsedTime);

	std::string getMeshName() const { return m_meshName; }
	
	bool isPlaying() const { return isAnimationRunning; }
	
	Type getAnimationType() const { return m_type; }
	
	void pause();

	void reset();

	void play();

private:

	struct TimeStep 
	{ 
		float p_delT, c_delT;
		void update(float fElapsedTime)
		{
			p_delT  = c_delT;
			c_delT += fElapsedTime;
		}
		void reset()
		{
			c_delT = 0;
			p_delT = 0;
		}
		float step()
		{
			return c_delT - p_delT;
		}
	};

	void calculateAnimation(float fElapsedTime);

	void calculateTranslation(float stepRatio);

	void calculateRotation(float stepRatio);
	
private:
	Type m_type;
	float m_duration;
	TimeStep m_deltaT;
	std::string m_meshName;
	
	bool   isAnimationRunning;

	DirectX::XMFLOAT3 m_animation;
	DirectX::XMMATRIX lastAnimationState;
};