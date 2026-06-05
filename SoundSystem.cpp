#include "SoundSystem.h"

#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>

SoundSystem* SoundSystem::sm_pInstance = nullptr;

SoundSystem& SoundSystem::GetInstance()
{
	if (sm_pInstance == nullptr)
	{
		sm_pInstance = new SoundSystem();
	}
	return *sm_pInstance;
}

void SoundSystem::DestroyInstance()
{
	delete sm_pInstance;
	sm_pInstance = nullptr;
}

namespace
{
	bool fileExists(const char* path)
	{
		if (path == nullptr || path[0] == '\0')
		{
			return false;
		}

		std::ifstream file(path, std::ios::binary);
		return file.good();
	}

	void playFallbackSound(const std::string& path)
	{
#if defined(__APPLE__)
		std::string cmd = "afplay -v 1 \"";
		for (char c : path)
		{
			if (c == '"')
			{
				cmd += "\\\"";
			}
			else
			{
				cmd += c;
			}
		}
		cmd += "\" >/dev/null 2>&1 &";
		std::system(cmd.c_str());
#else
		(void)path;
#endif
	}
}

#if defined(CARDBOARD_NO_FMOD)

SoundSystem::SoundSystem() = default;

SoundSystem::~SoundSystem()
{
	Shutdown();
}

bool SoundSystem::Initialise()
{
#if defined(__APPLE__)
	std::cout << "SoundSystem: using macOS afplay fallback (FMOD not linked).\n";
#endif
	return true;
}

void SoundSystem::Update()
{
}

void SoundSystem::Shutdown()
{
	m_isMusicTrack.clear();
	m_fallbackPaths.clear();
}

bool SoundSystem::LoadSound(const char* filename, const char* key, bool loop, bool isMusic)
{
	(void)loop;

	const char* paths[] = {
		filename,
	};

	for (const char* path : paths)
	{
		if (fileExists(path))
		{
			m_fallbackPaths[key] = path;
			m_isMusicTrack[key] = isMusic;
			std::cout << "SoundSystem: loaded '" << key << "' from " << path << "\n";
			return true;
		}
	}

	std::cout << "SoundSystem: Failed to load " << filename << "\n";
	return false;
}

void SoundSystem::PlaySound(const char* key)
{
	const auto it = m_fallbackPaths.find(key);
	if (it == m_fallbackPaths.end())
	{
		return;
	}

	playFallbackSound(it->second);
}

void SoundSystem::SetMasterVolume(float volume)
{
	(void)volume;
}

void SoundSystem::PauseAll()
{
}

void SoundSystem::ResumeAll()
{
}

void SoundSystem::PauseMusic()
{
}

void SoundSystem::ResumeMusic()
{
}

#else

SoundSystem::SoundSystem() : m_pSystem(nullptr), m_pMusicGroup(nullptr), m_pSFXGroup(nullptr)
{

}

SoundSystem::~SoundSystem()
{
	Shutdown();
}

bool SoundSystem::Initialise()
{
	FMOD_RESULT result = FMOD::System_Create(&m_pSystem);

	if (result != FMOD_OK) return false;

	result = m_pSystem->init(512, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) return false;

	m_pSystem->createChannelGroup("music", &m_pMusicGroup);
	m_pSystem->createChannelGroup("sfx", &m_pSFXGroup);

	return true;
}

void SoundSystem::Update()
{
	if (m_pSystem) m_pSystem->update();
}

void SoundSystem::Shutdown()
{
	for (auto& pair : m_sounds)
	{
		pair.second->release();
	}
	m_sounds.clear();
	m_fallbackPaths.clear();

	if (m_pSystem)
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}

bool SoundSystem::LoadSound(const char* filename, const char* key, bool loop, bool isMusic)
{
	if (!m_pSystem) return false;

	FMOD_MODE mode = loop ? (FMOD_LOOP_NORMAL | FMOD_CREATESTREAM) : FMOD_DEFAULT;
	FMOD::Sound* pSound = nullptr;

	FMOD_RESULT result = m_pSystem->createSound(filename, mode, nullptr, &pSound);
	if (result != FMOD_OK)
	{
		std::cout << "SoundSystem: Failed to load " << filename << "\n";
		return false;
	}

	m_sounds[key] = pSound;
	m_fallbackPaths[key] = filename;

	m_isMusicTrack[key] = isMusic;

	return true;
}

void SoundSystem::PlaySound(const char* key)
{
	if (!m_pSystem) return;

	auto it = m_sounds.find(key);
	if (it == m_sounds.end()) return;

	bool isMusic = m_isMusicTrack.count(key) && m_isMusicTrack[key];
	FMOD::ChannelGroup* group = isMusic ? m_pMusicGroup : m_pSFXGroup;

	m_pSystem->playSound(it->second, group, false, nullptr);
}

void SoundSystem::SetMasterVolume(float volume)
{
	if (!m_pSystem) return;

	FMOD::ChannelGroup* master = nullptr;
	m_pSystem->getMasterChannelGroup(&master);
	if (master) master->setVolume(volume);
}

void SoundSystem::PauseAll()
{
	if (!m_pSystem) return;

	FMOD::ChannelGroup* master = nullptr;
	m_pSystem->getMasterChannelGroup(&master);
	if (master) master->setPaused(true);
}

void SoundSystem::ResumeAll()
{
	if (!m_pSystem) return;

	FMOD::ChannelGroup* master = nullptr;
	m_pSystem->getMasterChannelGroup(&master);
	if (master) master->setPaused(false);
}

void SoundSystem::PauseMusic()
{
	if (m_pMusicGroup) m_pMusicGroup->setPaused(true);
}

void SoundSystem::ResumeMusic()
{
	if (m_pMusicGroup) m_pMusicGroup->setPaused(false);
}

#endif
