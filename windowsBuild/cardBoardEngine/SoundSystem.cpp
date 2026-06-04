#include "SoundSystem.h"

#include <iostream>
#include <cassert>

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

SoundSystem::SoundSystem() : m_pSystem(nullptr)
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

	if (m_pSystem)
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}

bool SoundSystem::LoadSound(const char* filename, const char* key, bool loop, bool isMusic)
{
	FMOD_MODE mode = loop ? (FMOD_LOOP_NORMAL | FMOD_CREATESTREAM) : FMOD_DEFAULT;
	FMOD::Sound* pSound = nullptr;

	FMOD_RESULT result = m_pSystem->createSound(filename, mode, nullptr, &pSound);
	if (result != FMOD_OK)
	{
		std::cout << "SoundSystem: Failed to load " << filename << "\n";
		return false;
	}

	m_sounds[key] = pSound;

	m_isMusicTrack[key] = isMusic;

	return true;
}

void SoundSystem::PlaySound(const char* key)
{
	auto it = m_sounds.find(key);
	if (it == m_sounds.end()) return;

	bool isMusic = m_isMusicTrack.count(key) && m_isMusicTrack[key];
	FMOD::ChannelGroup* group = isMusic ? m_pMusicGroup : m_pSFXGroup;

	m_pSystem->playSound(it->second, group, false, nullptr);
}

void SoundSystem::SetMasterVolume(float volume)
{
	FMOD::ChannelGroup* master = nullptr;
	m_pSystem->getMasterChannelGroup(&master);
	if (master) master->setVolume(volume);
}

void SoundSystem::PauseAll()
{
	FMOD::ChannelGroup* master = nullptr;
	m_pSystem->getMasterChannelGroup(&master);
	if (master) master->setPaused(true);
}

void SoundSystem::ResumeAll()
{
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