// COMP710 GP Framework 2024
#ifndef __TEXTURE_H_
#define __TEXTURE_H_

#include <SDL_ttf.h>
#include <SDL.h>

class Texture
{
public:
	Texture();
	~Texture();

	bool initialize(const char* filename);
	bool initializeFromRgba(int width, int height, const unsigned char* rgbaPixels);

	void setActive();

	int getWidth() const;
	int getHeight() const;

	void LoadTextTexture(const char* text, const char* fontname, int pointsize);

protected:
	void LoadSurfaceIntoTexture(SDL_Surface* pSurface);
	unsigned int mTextureId;
	int mWidth;
	int mHeight;
private:
	Texture(const Texture& texture);
	Texture& operator=(const Texture& texture);

	
};

#endif // __TEXTURE_H_
