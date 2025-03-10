/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#pragma once
#include <dpp/export.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <functional>

/**
 * @brief The main namespace for D++ functions. classes and types
 */
namespace dpp {
	/** @brief A 64 bit unsigned value representing many things on discord.
	 * Discord calls the value a 'snowflake' value.
	 */
	typedef uint64_t snowflake;

	/** @brief The managed class is the base class for various types that can
	 * be stored in a cache that are identified by a dpp::snowflake id
	 */
	class CoreExport managed {
	public:
		/** Unique ID of object */
		snowflake id;
		/** Constructor, initialises id to 0 */
		managed(const snowflake = 0);
		/** Default destructor */
		virtual ~managed() = default;
	};

	/** @brief Supported image types for profile pictures */
	enum image_type {
		/// image/png
		i_png,
		/// image/jpeg
		i_jpg,
		/// image/gif
		i_gif
	};

	/** @brief Log levels */
	enum loglevel {
		/// Trace
		ll_trace = 0,
		/// Debug
		ll_debug,
		/// Information
		ll_info,
		/// Warning
		ll_warning,
		/// Error
		ll_error,
		/// Critical
		ll_critical
	};

	/** @brief Utility helper functions, generally for logging */
	namespace utility {

		typedef std::function<void(const std::string& output)> cmd_result_t;

		/**
		 * @brief Run a commandline program asyncronously. The command line program
		 * is spawned in a separate std::thread, and when complete, its output from
		 * stdout is passed to the callback function in its string prameter. For eample
		 * ```
		 * dpp::utility::exec("ls", [](const std::string& output) {
		 *     std::cout << "Output of 'ls': " << output << "\n";
		 * });
		 * ```
		 * 
		 * @param cmd The command to run.
		 * @param parameters Command line parameters. Each will be escaped using std::quoted.
		 * @param callback The callback to call on completion.
		 */
		void CoreExport exec(const std::string& cmd, std::vector<std::string> parameters = {}, cmd_result_t callback = {});

		/**
		 * @brief Returns urrent date and time
		 * 
		 * @return std::string Current date and time
		 */
		std::string CoreExport current_date_time();
		/**
		 * @brief Convert a dpp::loglevel enum value to a string
		 * 
		 * @param in log level to convert
		 * @return std::string string form of log level
		 */
		std::string CoreExport loglevel(dpp::loglevel in);

		/**
		 * @brief Store a 128 bit icon hash (profile picture, server icon etc)
		 * as a 128 bit binary value made of two uint64_t.
		 * Has a constructor to build one from a string, and a method to fetch
		 * the value back in string form.
		 */
		struct CoreExport iconhash {

			uint64_t first;		//!< High 64 bits
			uint64_t second;	//!< Low 64 bits

			/**
			 * @brief Construct a new iconcash object
			 */
			iconhash();

			/**
			 * @brief Construct a new iconhash object
			 * 
			 * @param hash String hash to construct from.
			 * Must contain a 32 character hex string.
			 * 
			 * @throws std::length_error if the provided
			 * string is not exactly 32 characters long.
			 */
			iconhash(const std::string &hash);

			/**
			 * @brief Assign from std::string
			 * 
			 * @param assignment string to assign from.
			 * 
			 * @throws std::length_error if the provided
			 * string is not exactly 32 characters long.
			 */
			iconhash& operator=(const std::string &assignment);

			/**
			 * @brief Change value of iconhash object
			 * 
			 * @param hash String hash to change to.
			 * Must contain a 32 character hex string.
			 * 
			 * @throws std::length_error if the provided
			 * string is not exactly 32 characters long.
			 */
			void set(const std::string &hash);

			/**
			 * @brief Convert iconhash back to 32 character
			 * string value.
			 * 
			 * @return std::string Hash value 
			 */
			std::string to_string() const;
		};

		/**
		 * @brief Return the current time with fractions of seconds.
		 * This is a unix epoch time with the fractional seconds part
		 * after the decimal place.
		 * 
		 * @return double time with fractional seconds
		 */
		double CoreExport time_f();

		/**
		 * @brief Returns true if D++ was built with voice support
		 * 
		 * @return bool True if voice support is compiled in (libsodium/libopus) 
		 */
		bool CoreExport has_voice();

		/**
		 * @brief Convert a byte count to display value
		 * 
		 * @param c number of bytes
		 * @return std::string display value suffixed with M, G, T where neccessary
		 */
		std::string CoreExport bytes(uint64_t c);

		/**
		 * @brief A class used to represent an uptime in hours, minutes,
		 * seconds and days, with helper functions to convert from time_t
		 * and display as a string.
		 */
		struct CoreExport uptime {
			uint16_t days;	//!< Number of days
			uint8_t hours;	//!< Number of hours
			uint8_t mins;	//!< Number of minutes
			uint8_t secs;	//!< Number of seconds

			/**
			 * @brief Construct a new uptime object
			 */
			uptime();

			/**
			 * @brief Construct a new uptime object
			 * 
			 * @param diff A time_t to initialise the object from
			 */
			uptime(time_t diff);

			/**
			 * @brief Get uptime as string
			 * 
			 * @return std::string Uptime as string
			 */
			std::string to_string();

			/**
			 * @brief Get uptime as seconds
			 * 
			 * @return uint64_t Uptime as seconds
			 */
			uint64_t to_secs();

			/**
			 * @brief Get uptime as milliseconds
			 * 
			 * @return uint64_t Uptime as milliseconds
			 */
			uint64_t to_msecs();
		};

		/**
		 * @brief Output hex values of a section of memory for debugging
		 * 
		 * @param data The start of the data to display
		 * @param length The length of data to display
		 */
		void CoreExport debug_dump(uint8_t* data, size_t length);

		/**
		 * @brief Returns the length of a UTF-8 string in codepoints
		 * 
		 * @param str string to count length of
		 * @return size_t length of string (0 for invalid utf8)
		 */
		size_t CoreExport utf8len(const std::string &str);

		/**
		 * @brief Return substring of a UTF-8 encoded string in codepoints
		 * 
		 * @param str string to return substring from
		 * @param start start codepoint offset
		 * @param length length in codepoints
		 * @return std::string Substring in UTF-8 or emtpy string if invalid UTF-8 passed in
		 */
		std::string CoreExport utf8substr(const std::string& str, std::string::size_type start, std::string::size_type length);
	};

};

#include <dpp/voicestate.h>
#include <dpp/role.h>
#include <dpp/user.h>
#include <dpp/channel.h>
#include <dpp/guild.h>
#include <dpp/invite.h>
#include <dpp/dtemplate.h>
#include <dpp/emoji.h>
#include <dpp/ban.h>
#include <dpp/prune.h>
#include <dpp/voiceregion.h>
#include <dpp/integration.h>
#include <dpp/webhook.h>
#include <dpp/presence.h>
#include <dpp/intents.h>
#include <dpp/slashcommand.h>
#include <dpp/auditlog.h>
