#include "Animation.h"

Animation::Animation(std::string meshName,    Type type, DirectX::XMFLOAT3 animation,       float duration) :
	                  m_meshName(meshName), m_type(type),      m_animation(animation), m_duration(duration)
{
	lastAnimationState = DirectX::XMMatrixIdentity();
	isAnimationRunning = false;
	m_deltaT.reset();
}

Animation::~Animation(void)
{
}

void Animation::play()
{
	if (!isAnimationRunning)
	{
		isAnimationRunning = true;
	}
}

void Animation::pause()
{
	if (isAnimationRunning)
	{
		isAnimationRunning = false;
	}
}

void Animation::reset()
{
	lastAnimationState = DirectX::XMMatrixIdentity();
	isAnimationRunning = false;
	m_deltaT.reset();
}

DirectX::XMMATRIX Animation::getAnimationMat(float fElapsedTime)
{
	calculateAnimation(fElapsedTime);

	return lastAnimationState;
}

void Animation::calculateAnimation(float fElapsedTime)
{
	if (isAnimationRunning)
	{
		if (m_deltaT.c_delT <= m_duration)
		{
			// Calculate Animation
			m_deltaT.update(fElapsedTime);
			float animationStepRatio = m_deltaT.step() / m_duration;
			
			if (m_type == Type::ROTATION)
			{
				calculateRotation(animationStepRatio);
			}
			else if (m_type == Type::TRANSLATION)
			{
				calculateTranslation(animationStepRatio);
			}
		}
		else
		{
			m_deltaT.reset();
			isAnimationRunning = false;
		}
	}
}

void Animation::calculateRotation(float stepRatio)
{
	DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(m_animation.x * stepRatio * DirectX::XM_PI / 180);
	DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(m_animation.y * stepRatio * DirectX::XM_PI / 180);
	DirectX::XMMATRIX rotZ = DirectX::XMMatrixRotationZ(m_animation.z * stepRatio * DirectX::XM_PI / 180);
	
	lastAnimationState *= (rotZ * rotY * rotX);
}

void Animation::calculateTranslation(float stepRatio)
{
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(m_animation.x * stepRatio,
																 m_animation.y * stepRatio,
																 m_animation.z * stepRatio);
	lastAnimationState *= (translation);
}