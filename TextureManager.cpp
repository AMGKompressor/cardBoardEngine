// COMP710 GP Framework 2025

#include "texturemanager.h"

#include "texture.h"
#include "logmanager.h"

#include <SDL.h>

#include <cstdio>
#include <string>

namespace
{
	bool fileReadable(const char* path)
	{
		if (path == nullptr || path[0] == '\0')
		{
			return false;
		}
		FILE* file = std::fopen(path, "rb");
		if (file == nullptr)
		{
			return false;
		}
		std::fclose(file);
		return true;
	}

	std::string fileNameFromPath(const char* path)
	{
		if (path == nullptr)
		{
			return std::string();
		}
		std::string p(path);
		const std::size_t slash = p.find_last_of("/\\");
		if (slash == std::string::npos)
		{
			return p;
		}
		return p.substr(slash + 1);
	}

	// VS runs the exe from game/; CMake copies assets → build/textures/.
	// Try several roots so teammates are not broken by mixed path styles in code.
	std::string resolveTexturePath(const char* path)
	{
		if (fileReadable(path))
		{
			return path;
		}

		const std::string name = fileNameFromPath(path);
		if (name.empty())
		{
			return path;
		}

		const std::string candidates[] = {
			"textures/" + name,
			"assets/textures/" + name,
			"../assets/textures/" + name,
			"game/textures/" + name,
		};

		for (const std::string& candidate : candidates)
		{
			if (fileReadable(candidate.c_str()))
			{
				return candidate;
			}
		}

		return path;
	}
}

TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{
	std::map<std::string, Texture*>::iterator iter = mLoadedTextures.begin();

	while (iter != mLoadedTextures.end())
	{
		Texture* texture = iter->second;
		delete texture;

		++iter;
	}

	mLoadedTextures.clear();
}

bool TextureManager::initialize()
{
	LogManager::getInstance().log("TextureManager starting...");

	return true;
}

Texture* TextureManager::getTexture(const char* filename)
{
	Texture* texture = 0;

	if (mLoadedTextures.find(filename) == mLoadedTextures.end())
	{
		const std::string resolved = resolveTexturePath(filename);
		texture = new Texture();
		if (!texture->initialize(resolved.c_str()))
		{
			LogManager::getInstance().log(
				("Texture failed (requested: " + std::string(filename)
					+ ", tried: " + resolved + ")").c_str());
			LogManager::getInstance().log("Texture failed to initialize!");
			delete texture;
			return 0;
		}

		mLoadedTextures[filename] = texture;
	}
	else
	{
		texture = mLoadedTextures[filename];
	}

	return texture;
}
