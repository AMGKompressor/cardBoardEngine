// COMP710 GP Framework 2024

#include "texture.h"

#include "logmanager.h"

#include <SDL.h>
#include <SDL_image.h>
#include <cassert>
#include <cstring>
#include <vector>
#include <GL/glew.h>

#include "glew.h"

#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <SDL_ttf.h>


Texture::Texture()
	: mTextureId(0)
	, mHeight(0)
	, mWidth(0)
{

}

Texture::~Texture()
{
	if (mTextureId != 0)
	{
		glDeleteTextures(1, &mTextureId);
		mTextureId = 0;
	}
}

bool Texture::initialize(const char* filename)
{
	SDL_Surface* loaded = IMG_Load(filename);
	if (loaded == nullptr)
	{
		LogManager::getInstance().log(filename);
		const char* sdlErr = SDL_GetError();
		if (sdlErr != nullptr && sdlErr[0] != '\0')
		{
			LogManager::getInstance().log(sdlErr);
		}
		LogManager::getInstance().log(
			"Texture failed to load! (missing file, or SDL2_image.dll not beside the .exe)");
		return false;
	}

	SDL_Surface* surface = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(loaded);
	if (surface == nullptr)
	{
		LogManager::getInstance().log("Texture format conversion failed!");
		return false;
	}

	mWidth = surface->w;
	mHeight = surface->h;
	const int w = mWidth;
	const int h = mHeight;
	const int pitch = surface->pitch;
	const unsigned char* src = static_cast<const unsigned char*>(surface->pixels);

	// OpenGL's first row is the bottom of the texture; SDL's first row is the top of the image.
	std::vector<unsigned char> rgba(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 4u);
	for (int y = 0; y < h; ++y)
	{
		const unsigned char* srcRow = src + static_cast<std::size_t>(y) * static_cast<std::size_t>(pitch);
		unsigned char* dstRow = &rgba[static_cast<std::size_t>(h - 1 - y) * static_cast<std::size_t>(w) * 4u];
		std::memcpy(dstRow, srcRow, static_cast<std::size_t>(w) * 4u);
	}

	SDL_FreeSurface(surface);
	surface = nullptr;

	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool Texture::initializeFromRgba(int width, int height, const unsigned char* rgbaPixels)
{
	if (width <= 0 || height <= 0 || rgbaPixels == nullptr)
	{
		return false;
	}

	if (mTextureId != 0)
	{
		glDeleteTextures(1, &mTextureId);
		mTextureId = 0;
	}

	mWidth = width;
	mHeight = height;

	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		mWidth,
		mHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		rgbaPixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void Texture::setActive()
{
	glBindTexture(GL_TEXTURE_2D, mTextureId);
}

int Texture::getWidth() const
{
	assert(mWidth);
	return (mWidth);
}

int Texture::getHeight() const
{
	assert(mHeight);
	return (mHeight);
}
void
Texture::LoadTextTexture(const char* text, const char* fontname, int pointsize)
{
	TTF_Font* pFont = 0;
	TTF_Init();
	if (pFont == 0)
	{
		pFont = TTF_OpenFont(fontname, pointsize);
	}
	SDL_Color color;
	color.r = 255;
	color.g = 255;
	color.b = 255;
	color.a = 100;
	SDL_Surface* pSurface = TTF_RenderText_Blended(pFont, text, color);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	LoadSurfaceIntoTexture(pSurface);
	TTF_CloseFont(pFont);
	pFont = 0;
}

void Texture::LoadSurfaceIntoTexture(SDL_Surface* pSurface)
{
	if (pSurface)
	{
		mWidth = pSurface->w;
		mHeight = pSurface->h;

		int bytesPerPixel = pSurface->format->BytesPerPixel;
		unsigned int format = (bytesPerPixel == 4) ? GL_RGBA : GL_RGB;

		// DELETE old texture before generating a new one
		if (mTextureId != 0)
		{
			glDeleteTextures(1, &mTextureId);
			mTextureId = 0;
		}

		glGenTextures(1, &mTextureId);
		glBindTexture(GL_TEXTURE_2D, mTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0,
			format, GL_UNSIGNED_BYTE, pSurface->pixels);

		SDL_FreeSurface(pSurface);
		pSurface = nullptr;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}