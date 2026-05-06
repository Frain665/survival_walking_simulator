#pragma once

#include <format>
#include <source_location>
#include <string_view>
#include <iostream>
#include <chrono>
#include <thread>

namespace utils
{
	class Logger
	{
	public:
		enum class Level
		{
			Debug,
			Info,
			Warning,
			Error,
			Fatal
		};

	private:
		static void log(Level level, std::string_veiw message)
		{
			if (level < s_level) return;

			const auto now = std::chrono::system_clock::now();
			const auto time = std::chrono::system_clock::to_time_t(now);

			std::cout << std::format("[{:%H:%M:%S}] [{}] {}\n",
										*std::localtime(&time),
										levelToString(level),
										message);
		}

		static constexpr std::string_view levelToString(Level level)
		{
			switch (level)
			{
			case utils::Logger::Level::Debug:
				return "DEBUG";
				break;
			case utils::Logger::Level::Info:
				return "INFO";
				break;
			case utils::Logger::Level::Warning:
				return "WARNING";
				break;
			case utils::Logger::Level::Error:
				return "ERROR";
				break;
			case utils::Logger::Level::Fatal:
				return "FATAL";
				break;
			default:
				return "UNKNOWN";
				break;
			}
		}

		static inline Level s_level{ Level::Info };
	};


} // namespace utils