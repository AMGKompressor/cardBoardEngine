#include "animatedsprite.h"

// Local includes: 
#include "Renderer.h" 
#include "vertexarray.h" 
#include "texture.h" 
#include "imgui.h"

// Library includes: 
#include <cassert>

AnimatedSprite::AnimatedSprite()
	: m_pVertexData(0)
	, m_iFrameWidth(0)
	, m_iFrameHeight(0)
	, m_iCurrentFrame(0)
	, m_iTotalFrames(0)
	, m_fTimeElapsed(0.0f)
	, m_bAnimating(false)
	, m_bLooping(false)
	, m_frameDuration(1.0f)
	, totalTime(0.0f)
{

}

AnimatedSprite::~AnimatedSprite()
{
	delete m_pVertexData; m_pVertexData = 0;
}

bool
AnimatedSprite::Initialise(Texture& texture)
{
	return Sprite::initialize(texture);
}

int
AnimatedSprite::getWidth() const
{
	return static_cast<int>(ceilf(m_iFrameWidth * mScale));
}

int
AnimatedSprite::getHeight() const
{
	return static_cast<int>(ceilf(m_iFrameHeight * mScale));
}

bool
AnimatedSprite::isAnimating() const
{
	return m_bAnimating;
}

void
AnimatedSprite::SetupFrames(int fixedFrameWidth, int fixedFrameHeight)
{
	m_iFrameWidth = fixedFrameWidth; 
	m_iFrameHeight = fixedFrameHeight;

	const int textureWidth = mTexture->getWidth();
	const int textureHeight = mTexture->getHeight();
	const int totalFramesWide = textureWidth / fixedFrameWidth; 
	const int totalFramesHigh = textureHeight / fixedFrameHeight;

	const int stride = 5;
	const float uFrameWidth = 1.0f / totalFramesWide; 
	const float vFrameHeight = 1.0f / totalFramesHigh; m_iTotalFrames = totalFramesWide * totalFramesHigh;

	const int vertsPerSprite = 4;
	const int numVertices = vertsPerSprite * (m_iTotalFrames);

	float* vertices = new float[numVertices * stride];

	for (int h = 0; h < totalFramesHigh; ++h)
	{
		for (int w = 0; w < totalFramesWide; ++w)
		{
			float uOffset = (w * uFrameWidth); float vOffset = (h * vFrameHeight);

			float quad[] =
			{
				-0.5f,  0.5f, 0.0f,  0.0f + uOffset,          vOffset + vFrameHeight,
				 0.5f,  0.5f, 0.0f,  uFrameWidth + uOffset,   vOffset + vFrameHeight,
				 0.5f, -0.5f, 0.0f,  uFrameWidth + uOffset,   vOffset,
				-0.5f, -0.5f, 0.0f,  0.0f + uOffset,          vOffset
			};
			const int floatsPerSprite = stride * vertsPerSprite; 
			for (int j = 0; j < floatsPerSprite; ++j)
			{
				int index = ((w * floatsPerSprite) + j) + (h * (floatsPerSprite * totalFramesWide));

				vertices[index] = quad[j];
			}
		}
	}

	const int totalIndices = 6 * m_iTotalFrames;
	unsigned int* allIndices = new unsigned int[totalIndices];

	unsigned int i = 0;

	for (int k = 0; k < m_iTotalFrames; ++k)
	{
		unsigned int indices[] = { 0 + i, 1 + i, 2 + i, 2 + i, 3 + i, 0 + i };

		for (int j = 0; j < 6; ++j)
		{
			int index = (k * 6) + j;

			allIndices[index] = indices[j];
		}

		i += 4;
	}
	m_pVertexData = new VertexArray(vertices, numVertices, allIndices, totalIndices); delete vertices;
	vertices = 0;

	delete allIndices; 
	allIndices = 0;
}

void
AnimatedSprite::Process(float deltaTime)
{
	totalTime += deltaTime;
	if (m_bAnimating)
	{
		m_fTimeElapsed += deltaTime;
		if (m_fTimeElapsed > m_frameDuration)
		{
			++m_iCurrentFrame;
			if (m_iCurrentFrame >= m_iTotalFrames)
			{
				if (m_bLooping)
				{
					Restart();
				}
				else
				{
					m_iCurrentFrame = m_iTotalFrames - 1; m_bAnimating = false;
				}
				
			}
			m_fTimeElapsed = 0.0f;
		}
	}
}

void
AnimatedSprite::Draw(Renderer& renderer)
{
	assert(m_pVertexData); 
	mTexture->setActive(); 
	m_pVertexData->setActive();
	//renderer.DrawAnimatedSprite(*this, m_iCurrentFrame);
}

void AnimatedSprite::Animate()
{
	m_bAnimating = true;
}

void
AnimatedSprite::setFrameDuration(float seconds)
{
	m_frameDuration = seconds;
}

void
AnimatedSprite::setLooping(bool loop)
{
	m_bLooping = loop;
}
void AnimatedSprite::Restart()
{
	m_iCurrentFrame = 0; m_fTimeElapsed = 0.0f;
}

void AnimatedSprite::DebugDraw()
{
	ImGui::SliderInt("Frame ", &m_iCurrentFrame, 0, m_iTotalFrames - 1);
}

void AnimatedSprite::SetFrame(int frame)
{
	if (frame >= 0 && frame < m_iTotalFrames)
		m_iCurrentFrame = frame;
}
