#pragma once
#ifndef __ANIMATEDSPRITE_H
#define __ANIMATEDSPRITE_H

#include "Sprite.h"

class Renderer;
class VertexArray;

class AnimatedSprite : public Sprite
{
	// MEMBER METHODS
public:
	AnimatedSprite();
	~AnimatedSprite();

	bool Initialise(Texture& texture);
	void SetupFrames(int fixedFrameWidth, int fixedFrameHeight);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);

	void setLooping(bool loop);
	void Animate();
	bool isAnimating() const;
	void Restart();
	void setFrameDuration(float seconds);

	int getWidth() const;
	int getHeight() const;
	void DebugDraw();

	void SetFrame(int frame);

protected:

	

private:
	AnimatedSprite(const AnimatedSprite& animatedSprite);
	AnimatedSprite& operator=(const AnimatedSprite& animatedSprite);

	// MEMBER DATA
public:

protected:
	VertexArray* m_pVertexData;
	int m_iFrameWidth;
	int m_iFrameHeight;
	int m_iCurrentFrame;
	int m_iTotalFrames;
	float m_fTimeElapsed;
	float m_frameDuration;
	float totalTime;
	bool m_bAnimating;
	bool m_bLooping;
private:
};

#endif // __ANIMATEDSPRITE_H