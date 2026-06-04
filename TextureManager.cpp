// COMP710 GP Framework 2025

#include "texturemanager.h"

#include "texture.h"
#include "logmanager.h"

#include <SDL.h>
#include <SDL_filesystem.h>

#include <cstdio>
#include <string>
#include <vector>

namespace
{
	std::string gExeDirectory;

	void ensureExeDirectoryCached()
	{
		if (!gExeDirectory.empty())
		{
			return;
		}

		char* basePath = SDL_GetBasePath();
		if (basePath != nullptr)
		{
			gExeDirectory = basePath;
			SDL_free(basePath);
		}
	}

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

	std::string joinPath(const std::string& root, const std::string& relative)
	{
		if (root.empty())
		{
			return relative;
		}
		if (relative.empty())
		{
			return root;
		}
		return root + relative;
	}

	void pushCandidate(std::vector<std::string>& candidates, const std::string& path)
	{
		if (path.empty())
		{
			return;
		}
		for (const std::string& existing : candidates)
		{
			if (existing == path)
			{
				return;
			}
		}
		candidates.push_back(path);
	}

	std::string resolveTexturePath(const char* path)
	{
		ensureExeDirectoryCached();

		const std::string name = fileNameFromPath(path);
		std::vector<std::string> candidates;
		pushCandidate(candidates, path);

		if (!name.empty())
		{
			const std::string relativePaths[] = {
				"textures/" + name,
				"assets/textures/" + name,
				"../assets/textures/" + name,
				"game/textures/" + name,
			};

			for (const std::string& relative : relativePaths)
			{
				pushCandidate(candidates, relative);
				if (!gExeDirectory.empty())
				{
					pushCandidate(candidates, joinPath(gExeDirectory, relative));
				}
			}
		}

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
	ensureExeDirectoryCached();
	if (!gExeDirectory.empty())
	{
		LogManager::getInstance().log(
			("TextureManager exe dir: " + gExeDirectory).c_str());
	}
	else
	{
		LogManager::getInstance().log("TextureManager: SDL_GetBasePath unavailable.");
	}

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
