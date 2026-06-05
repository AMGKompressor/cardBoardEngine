#pragma once

#ifndef __SOUNDSYSTEM_H_
#define __SOUNDSYSTEM_H_

#include <string>
#include <map>

#if !defined(CARDBOARD_NO_FMOD)
#include <fmod.hpp>
#endif

class SoundSystem
{
public:
	SoundSystem();
	~SoundSystem();
	static SoundSystem& GetInstance();
	static void DestroyInstance();

	bool Initialise();
	void Update();
	void Shutdown();

	bool LoadSound(const char* filename, const char* key, bool loop = false, bool isMusic = false);

	void PlaySound(const char* key);

	void PauseAll();
	void ResumeAll();

	void PauseMusic();
	void ResumeMusic();

	void SetMasterVolume(float volume);

private:
	

	SoundSystem(const SoundSystem&);
	SoundSystem& operator=(const SoundSystem&);

	static SoundSystem* sm_pInstance;

#if !defined(CARDBOARD_NO_FMOD)
	FMOD::System* m_pSystem;
	std::map<std::string, FMOD::Sound*> m_sounds;

	FMOD::ChannelGroup* m_pMusicGroup;
	FMOD::ChannelGroup* m_pSFXGroup;
#endif

	std::map<std::string, bool> m_isMusicTrack;
	// Used when FMOD is unavailable (e.g. Mac dev builds).
	std::map<std::string, std::string> m_fallbackPaths;
};

#endif // __SOUNDSYSTEM_H_
