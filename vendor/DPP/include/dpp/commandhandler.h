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
#include <dpp/discord.h>
#include <dpp/json_fwd.hpp>
#include <unordered_map>
#include <vector>
#include <functional>
#include <variant>

namespace dpp {

/**
 * @brief Represents a received parameter.
 * We use variant so that multiple non-related types can be contained within.
 */
typedef std::variant<std::string, dpp::role, dpp::channel, dpp::user, int32_t, bool> command_parameter;

/**
 * @brief Parameter types when registering a command.
 * We don't pass these in when triggering the command in the handler, because it is
 * expected the developer added the command so they know what types to expect for each named
 * parameter.
 */
enum parameter_type {
	pt_string,	//!< String value
	pt_role,	//!< Role object
	pt_channel,	//!< Channel object
	pt_user,	//!< User object
	pt_integer,	//!< 32 bit signed integer
	pt_boolean	//!< boolean
};

/**
 * @brief Details of a command parameter used in registration.
 * Note that for non-slash commands optional parameters can only be at the end of
 * the list of parameters.
 */
struct CoreExport param_info {

	/**
	 * @brief Type of parameter
	 */
	parameter_type type;

	/**
	 * @brief True if the parameter is optional.
	 * For non-slash commands optional parameters may only be on the end of the list.
	 */
	bool optional;

	/**
	 * @brief Description of command. Displayed only for slash commands
	 */
	std::string description;

	/**
	 * @brief Allowed multiple choice options.
	 * The key name is the string passed to the command handler
	 * and the key value is its description displayed to the user.
	 */
	std::map<std::string, std::string> choices;

	/**
	 * @brief Construct a new param_info object
	 * 
	 * @param t Type of parameter
	 * @param o True if parameter is optional
	 * @param description The parameter description
	 * @param opts The options for a multiple choice parameter
	 */
	param_info(parameter_type t, bool o, const std::string &description, const std::map<std::string, std::string> &opts = {});
};

/**
 * @brief Parameter list used during registration.
 * Note that use of vector/pair is important here to preserve parameter order,
 * as opposed to unordered_map (which doesnt guarantee any order at all) and 
 * std::map, which reorders keys alphabetically.
 */
typedef std::vector<std::pair<std::string, param_info>> parameter_registration_t;

/**
 * @brief Parameter list for a called command.
 * See dpp::parameter_registration_t for an explaination as to why vector is used.
 */
typedef std::vector<std::pair<std::string, command_parameter>> parameter_list_t;

/**
 * @brief Represents the sending source of a command.
 * This is passed to any command handler and should be passed back to
 * commandhandler::reply(), allowing the reply method to route any replies back
 * to the origin, which may be a slash command or a message. Both require different
 * response facilities but we want this to be transparent if you use the command
 * handler class.
 */
struct CoreExport command_source {
	/**
	 * @brief Sending guild id
	 */
	snowflake guild_id = 0;
	/**
	 * @brief Source channel id
	 */
	snowflake channel_id = 0;
	/**
	 * @brief Command ID of a slash command
	 */
	snowflake command_id = 0;
	/**
	 * @brief Token for sending a slash command reply
	 */
	std::string command_token;
	/**
	 * @brief The user who issued the command
	 */
	user* issuer;
};

/**
 * @brief The function definition for a command handler. Expects a command name string,
 * and a list of command parameters.
 */
typedef std::function<void(const std::string&, const parameter_list_t&, command_source)> command_handler;

/**
 * @brief Represents the details of a command added to the command handler class.
 */
struct CoreExport command_info_t {
	/**
	 * @brief Function reference for the handler. This is std::function so it can represent
	 * a class member, a lambda or a raw C function pointer.
	 */
	command_handler func;
	/**
	 * @brief Parameters requested for the command, with their types
	 */
	parameter_registration_t parameters;
	/**
	 * @brief Guild ID the command exists on, or 0 to be present on all guilds
	 */
	snowflake guild_id;
};


/**
 * @brief The commandhandler class represents a group of commands, prefixed or slash commands with handling functions.
 * 
 */
class CoreExport commandhandler {
	/**
	 * @brief Commands in the handler
	 */
	std::unordered_map<std::string, command_info_t> commands;

	/**
	 * @brief Valid prefixes
	 */
	std::vector<std::string> prefixes;

	/**
	 * @brief Set to true automatically if one of the prefixes added is "/"
	 */
	bool slash_commands_enabled;

	/**
	 * @brief Cluster we are attached to for issuing REST calls
	 */
	class cluster* owner;

	/**
	 * @brief Application ID
	 */
	snowflake app_id;

	/**
	 * @brief Returns true if the string has a known prefix on the start.
	 * Modifies string to remove prefix if it returns true.
	 * 
	 * @param str String to check and modify
	 * @return true string contained a prefix, prefix removed from string
	 * @return false string did not contain a prefix
	 */
	bool string_has_prefix(std::string &str);

public:

	/**
	 * @brief Construct a new commandhandler object
	 * 
	 * @param o Owning cluster to attach to
	 * @param auto_hook_events Set to true to automatically hook the on_interaction_create
	 * and on_message events. Only do this if you have no other use for these events than
	 * commands that are handled by the command handler (this is usually the case).
	 * @param application_id The application id of the bot. If not specified, the class will
	 * look within the cluster object and use cluster::me::id instead.
	 */
	commandhandler(class cluster* o, bool auto_hook_events = true, snowflake application_id = 0);

	/**
	 * @brief Destroy the commandhandler object
	 */
	~commandhandler();

	/**
	 * @brief Set the application id after construction
	 * 
	 * @param o Owning cluster to attach to
	 */
	commandhandler& set_owner(class cluster* o);

	/**
	 * @brief Add a prefix to the command handler
	 * 
	 * @param prefix Prefix to be handled by the command handler
	 * @return commandhandler& reference to self
	 */
	commandhandler& add_prefix(const std::string &prefix);

	/**
	 * @brief Add a command to the command handler
	 * 
	 * @param command Command to be handled.
	 * Note that if any one of your prefixes is "/" this will attempt to register
	 * a global command using the API and you will receive notification of this command
	 * via an interaction event.
	 * 
	 * @param handler Handler function
	 * @param parameters Parameters to use for the command
	 * @return commandhandler& reference to self
	 */
	commandhandler& add_command(const std::string &command, const parameter_registration_t &parameters, command_handler handler, const std::string &description = "", snowflake guild_id = 0);

	/**
	 * @brief Route a command from the on_message_create function.
	 * Call this method from within your on_message_create with the received
	 * dpp::message object.
	 * 
	 * @param msg message to parse
	 */
	void route(const class dpp::message& msg);

	/**
	 * @brief Route a command from the on_interaction_create function.
	 * Call this method from your on_interaction_create with the received
	 * dpp::interaction_create_t object.
	 * 
	 * @param event command interaction event to parse
	 */
	void route(const class interaction_create_t & event);

	/**
	 * @brief Reply to a command.
	 * You should use this method rather than cluster::message_create as
	 * the way you reply varies between slash commands and message commands.
	 * Note you should ALWAYS reply. Slash commands will emit an ugly error
	 * to the user if you do not emit some form of reply within 3 seconds.
	 * 
	 * @param m message to reply with.
	 * @param interaction true if the reply is generated by an interaction
	 */
	void reply(const dpp::message &m, command_source source);
};

};