#pragma once

#include <miniaudio.h>

#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>
#include <expected>
#include <glm/glm.hpp>

namespace core::audio
{
	enum class AudioError
	{
		InitFailed,
		LoadFailed,
		PlayFailed,
		InvalidHandle,
		NotInitialized
	};

	/**
	* @brief Хэндл для управляемого звука (RAII wrapper)
	*/
	class SoundHandle
	{
	public:
		SoundHandle() = default;
		explicit SoundHandle(ma_sound* sound) noexcept : m_sound(sound) {}

		~SoundHandle()
		{
			if (m_sound)
			{
				ma_sound_stop(m_sound);
				ma_sound_uninit(m_sound);
				delete m_sound;
			}
		}

		SoundHandle(const SoundHandle&) = delete;
		SoundHandle(SoundHandle&& other) noexcept : m_sound(std::exchange(other.m_sound, nullptr)) {}

		auto operator=(const SoundHandle&) -> SoundHandle& = delete;
		auto operator=(SoundHandle&& other) noexcept -> SoundHandle&
		{
			if (this != &other)
			{
				if (m_sound)
				{
					ma_sound_stop(m_sound);
					ma_sound_uninit(m_sound);
					delete m_sound;
				}
				m_sound = std::exchange(other.m_sound, nullptr);
			}
			return *this;
		}

		[[nodiscard]] auto isValid() const noexcept -> bool { return m_sound != nullptr; }

		void stop()    noexcept { if (m_sound) ma_sound_stop(m_sound); }
		void setVolume(float volume) noexcept { if (m_sound) ma_sound_set_volume(m_sound, volume); }
		void setPitch(float pitch)   noexcept { if (m_sound) ma_sound_set_pitch(m_sound, pitch); }
		void setLooping(bool loop)   noexcept { if (m_sound) ma_sound_set_looping(m_sound, loop ? MA_TRUE : MA_FALSE); }

	private:
		ma_sound* m_sound{ nullptr };
	};

	/**
	* @brief Главный аудио движок с поддержкой 3D звука
	* @details Обертка над miniaudio с RAII, автоматическим управлением памятью
	*          и современным C++ интерфейсом
	*/
	class AudioEngine final
	{
	public:
		AudioEngine() = default;
		~AudioEngine() noexcept { shutdown(); }

		AudioEngine(const AudioEngine&) = delete;
		AudioEngine(AudioEngine&& other) noexcept;
		auto operator=(const AudioEngine&) -> AudioEngine& = delete;
		auto operator=(AudioEngine&& other) noexcept -> AudioEngine&;

		[[nodiscard]] auto init() -> bool;

		void shutdown() noexcept;
		void update() noexcept;

		[[nodiscard]] auto loadSound(std::string_view filePath)
			-> std::expected<SoundHandle, AudioError>;

		auto playSound(std::string_view soundId, float volume = 1.0f, float pitch = 1.0f)
			-> std::expected<SoundHandle, AudioError>;

		auto playSound3D(std::string_view soundId, const glm::vec3& position,
			float volume = 1.0f, float minDistance = 1.0f, float maxDistance = 50.0f)
			-> std::expected<SoundHandle, AudioError>;

		// === Фоновая музыка ===

		void playBackgroundMusic(std::string_view filepath, bool loop = true);
		void stopBackgroundMusic() noexcept;
		void setBackgroundMusicVolume(float volume) noexcept;
		[[nodiscard]] auto isBackgroundMusicPlaying() const noexcept -> bool;

		// === Настройки слушателя ===

		void setListenerPosition(const glm::vec3& position) noexcept;
		void setListenerOrientation(const glm::vec3& forward, const glm::vec3& up) noexcept;
		void setListenerGain(float gain) noexcept;

		// === Глобальные настройки ===

		void setMasterVolume(float volume) noexcept;
		void setSoundEffectsVolume(float volume) noexcept;
		[[nodiscard]] auto getMasterVolume() const noexcept -> float { return m_masterVolume; }

	private:
		struct LoadedSound
		{
			ma_sound sound{};
			bool isValid{ false };

			LoadedSound() { memset(&sound, 0, sizeof(sound)); }
			~LoadedSound() { if (isValid) ma_sound_uninit(&sound); }

			LoadedSound(const LoadedSound&) = delete;
			LoadedSound(LoadedSound&& other) noexcept;
			auto operator=(LoadedSound&& other) noexcept -> LoadedSound&;
		};

		// FIX: исправлена сигнатура — ma_uint32 (не ma_unit32)
		static void logCallback(void* userData, ma_uint32 logLevel, const char* message);

		// === Данные ===
		ma_engine m_engine{};
		ma_sound  m_backgroundMusic{};

		std::unordered_map<std::string, LoadedSound> m_sounds;

		float m_masterVolume{ 1.0f };
		float m_sfxVolume   { 1.0f };
		float m_musicVolume { 0.7f };

		bool m_isInitialized{ false };
	};

} // namespace core::audio
