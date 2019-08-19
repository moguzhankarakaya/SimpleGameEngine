#include "ComplexAnimation.h"


ComplexAnimation::ComplexAnimation(UINT numberOfAnimations, 
								   std::vector<AbsAnimation*>& animations) :
	m_numberOfAnimations(numberOfAnimations), 
	m_animations(animations)
{
	currentAnimation   = 0;
	isAnimationRunning = false;
	lastAnimationState = DirectX::XMMatrixIdentity();
}

ComplexAnimation::~ComplexAnimation(void)
{
}

void ComplexAnimation::pause()
{
	if (isAnimationRunning)
	{
		isAnimationRunning = false;
	}
}

void ComplexAnimation::play()
{
	if (!isAnimationRunning)
	{
		isAnimationRunning = true;
		m_animations[currentAnimation]->play();
	}
}

void ComplexAnimation::reset()
{
	currentAnimation   = 0;
	isAnimationRunning = false;
	lastAnimationState = DirectX::XMMatrixIdentity();
}

DirectX::XMMATRIX ComplexAnimation::getAnimationMat(float fElapsedTime)
{
	calculateAnimation(fElapsedTime);

	return lastAnimationState;
}

void ComplexAnimation::calculateAnimation(float fElapsedTime)
{
	if (isAnimationRunning)
	{
		if (m_animations[currentAnimation]->isPlaying())
		{
			lastAnimationState *= m_animations[currentAnimation]->getAnimationMat(fElapsedTime);
		}
		else
		{
			m_animations[currentAnimation]->reset();
			currentAnimation++;

			if (currentAnimation < m_numberOfAnimations)
			{
				m_animations[currentAnimation]->play();
			}
			else
			{
				reset();
			}
		}
	}
}