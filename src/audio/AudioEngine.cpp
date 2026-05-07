#include "AudioEngine.h"

#include <format>
#include <iostream> // временно, пока нет логера
#include <algorithm>

namespace core::audio
{
	// === LoadedSound ===

	AudioEngine::LoadedSound::LoadedSound(LoadedSound&& other) noexcept
		: isValid(std::exchange(other.isValid, false))
	{
		memcpy(&sound, &other.sound, sizeof(ma_sound));
		memset(&other.sound, 0, sizeof(ma_sound));
	}

	auto AudioEngine::LoadedSound::operator=(LoadedSound&& other) noexcept -> LoadedSound&
	{
		if (this != &other)
		{
			if (isValid) ma_sound_uninit(&sound);
			memcpy(&sound, &other.sound, sizeof(ma_sound));
			isValid = std::exchange(other.isValid, false);
			memset(&other.sound, 0, sizeof(ma_sound));
		}
		return *this;
	}

	// === AudioEngine ===

	AudioEngine::AudioEngine(AudioEngine&& other) noexcept
		: m_sounds(std::move(other.m_sounds))
		, m_masterVolume(other.m_masterVolume)
		, m_sfxVolume(other.m_sfxVolume)
		, m_musicVolume(other.m_musicVolume)
		, m_isInitialized(std::exchange(other.m_isInitialized, false))
	{
		memcpy(&m_engine, &other.m_engine, sizeof(ma_engine));
		memcpy(&m_backgroundMusic, &other.m_backgroundMusic, sizeof(ma_sound));
		memset(&other.m_engine, 0, sizeof(ma_engine));
		memset(&other.m_backgroundMusic, 0, sizeof(ma_sound));
	}

	auto AudioEngine::operator=(AudioEngine&& other) noexcept -> AudioEngine&
	{
		if (this != &other)
		{
			shutdown();

			m_sounds        = std::move(other.m_sounds);
			m_masterVolume  = other.m_masterVolume;
			m_sfxVolume     = other.m_sfxVolume;
			m_musicVolume   = other.m_musicVolume;
			m_isInitialized = std::exchange(other.m_isInitialized, false);

			memcpy(&m_engine,          &other.m_engine,          sizeof(ma_engine));
			memcpy(&m_backgroundMusic, &other.m_backgroundMusic, sizeof(ma_sound));
			memset(&other.m_engine,          0, sizeof(ma_engine));
			memset(&other.m_backgroundMusic, 0, sizeof(ma_sound));
		}
		return *this;
	}

	void AudioEngine::logCallback(void* /*userdata*/, ma_uint32 /*logLevel*/, const char* message)
	{
		std::cout << "[miniaudio] " << message << std::endl;
	}

	auto AudioEngine::init() -> bool
	{
		if (m_isInitialized)
		{
			return true;
		}

		ma_engine_config config = ma_engine_config_init();
		config.pLog = nullptr;

		if (ma_engine_init(&config, &m_engine) != MA_SUCCESS)
		{
			std::cerr << "[AudioEngine] Failed to initialize miniaudio engine\n";
			return false;
		}

		// Инициализируем заглушку для фоновой музыки
		memset(&m_backgroundMusic, 0, sizeof(ma_sound));

		m_isInitialized = true;
		setMasterVolume(m_masterVolume);

		std::cout << "[AudioEngine] Initialized successfully\n";
		return true;
	}

	void AudioEngine::shutdown() noexcept
	{
		if (!m_isInitialized)
		{
			return;
		}

		if (ma_sound_is_playing(&m_backgroundMusic))
		{
			ma_sound_stop(&m_backgroundMusic);
		}
		ma_sound_uninit(&m_backgroundMusic);

		m_sounds.clear();
		ma_engine_uninit(&m_engine);

		m_isInitialized = false;

		std::cout << "[AudioEngine] Shutdown complete\n";
	}

	void AudioEngine::update() noexcept
	{
		// miniaudio всё делает в фоне.
		// Здесь можно добавить логику для динамических 3D-звуков.
	}

	auto AudioEngine::loadSound(std::string_view filepath) -> std::expected<SoundHandle, AudioError>
	{
		if (!m_isInitialized)
		{
			return std::unexpected(AudioError::NotInitialized);
		}

		const std::string path(filepath);

		if (auto it = m_sounds.find(path); it != m_sounds.end())
		{
			ma_sound* sound = new ma_sound();

			if (ma_sound_init_copy(&m_engine, &it->second.sound, 0, nullptr, sound) != MA_SUCCESS)
			{
				delete sound;
				return std::unexpected(AudioError::LoadFailed);
			}

			return SoundHandle(sound);
		}

		LoadedSound loaded;
		if (ma_sound_init_from_file(&m_engine, path.c_str(),
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			nullptr, nullptr, &loaded.sound) != MA_SUCCESS)
		{
			return std::unexpected(AudioError::LoadFailed);
		}

		loaded.isValid = true;

		ma_sound* sound = new ma_sound();

		if (ma_sound_init_copy(&m_engine, &loaded.sound, 0, nullptr, sound) != MA_SUCCESS)
		{
			ma_sound_uninit(&loaded.sound);
			delete sound;
			return std::unexpected(AudioError::LoadFailed);
		}

		m_sounds.emplace(path, std::move(loaded));

		return SoundHandle(sound);
	}

	auto AudioEngine::playSound(std::string_view soundId, float volume, float pitch)
		-> std::expected<SoundHandle, AudioError>
	{
		auto sound = loadSound(soundId); // FIX: LoadSound -> loadSound
		if (!sound)
		{
			return std::unexpected(sound.error());
		}

		sound->setVolume(volume * m_sfxVolume * m_masterVolume);
		sound->setPitch(pitch);
		// Запускаем воспроизведение
		/*if (sound->isValid())
		{
			ma_sound_start( sound.);
		}*/

		return sound;
	}

	auto AudioEngine::playSound3D(std::string_view soundId, const glm::vec3& position,
		float volume, float minDistance, float maxDistance)
		-> std::expected<SoundHandle, AudioError>
	{
		auto sound = loadSound(soundId);
		if (!sound)
		{
			return std::unexpected(sound.error());
		}

		sound->setVolume(volume * m_sfxVolume * m_masterVolume);
		// TODO: установить позицию источника звука через ma_sound_set_position

		return sound;
	}

	void AudioEngine::playBackgroundMusic(std::string_view filepath, bool loop)
	{
		if (!m_isInitialized) return;

		stopBackgroundMusic();

		ma_sound_uninit(&m_backgroundMusic);
		memset(&m_backgroundMusic, 0, sizeof(ma_sound));

		if (ma_sound_init_from_file(&m_engine, filepath.data(), 0,
			nullptr, nullptr, &m_backgroundMusic) != MA_SUCCESS)
		{
			std::cerr << std::format("[AudioEngine] Failed to load background music: {}", filepath)
				<< std::endl;
			return;
		}

		ma_sound_set_looping(&m_backgroundMusic, loop ? MA_TRUE : MA_FALSE);
		ma_sound_set_volume(&m_backgroundMusic, m_musicVolume * m_masterVolume);
		ma_sound_start(&m_backgroundMusic);
	}

	void AudioEngine::stopBackgroundMusic() noexcept
	{
		if (ma_sound_is_playing(&m_backgroundMusic))
		{
			ma_sound_stop(&m_backgroundMusic);
		}
	}

	void AudioEngine::setBackgroundMusicVolume(float volume) noexcept
	{
		m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
		ma_sound_set_volume(&m_backgroundMusic, m_musicVolume * m_masterVolume);
	}

	auto AudioEngine::isBackgroundMusicPlaying() const noexcept -> bool
	{
		return ma_sound_is_playing(&m_backgroundMusic) != 0;
	}

	void AudioEngine::setListenerPosition(const glm::vec3& position) noexcept
	{
		if (m_isInitialized)
		{
			ma_engine_listener_set_position(&m_engine, 0, position.x, position.y, position.z);
		}
	}

	void AudioEngine::setListenerOrientation(const glm::vec3& forward, const glm::vec3& up) noexcept
	{
		if (m_isInitialized)
		{
			ma_engine_listener_set_direction(&m_engine, 0, forward.x, forward.y, forward.z);
			ma_engine_listener_set_world_up(&m_engine, 0, up.x, up.y, up.z);
		}
	}

	void AudioEngine::setListenerGain(float gain) noexcept
	{
		if (m_isInitialized)
		{
			ma_engine_listener_set_cone(&m_engine, 0,
				glm::radians(360.0f), 
				glm::radians(360.0f),
				gain);
		}
	}

	void AudioEngine::setMasterVolume(float volume) noexcept
	{
		m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
		if (m_isInitialized)
		{
			ma_engine_set_volume(&m_engine, m_masterVolume);
		}
	}

	void AudioEngine::setSoundEffectsVolume(float volume) noexcept
	{
		m_sfxVolume = std::clamp(volume, 0.0f, 1.0f);
	}

} // namespace core::audio
