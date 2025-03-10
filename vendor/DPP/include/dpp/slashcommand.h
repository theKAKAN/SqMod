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
#include <variant>
#include <dpp/discord.h>
#include <dpp/json_fwd.hpp>
#include <dpp/message.h>

namespace dpp {

/**
 * @brief Represents command option types.
 * These are the possible parameter value types.
 */
enum command_option_type : uint8_t {
	/** A sub-command */
	co_sub_command = 1,
	/** A sub-command group */
	co_sub_command_group = 2,
	/** A string value */
	co_string = 3,
	/** An integer value */
	co_integer = 4,
	/** A boolean value */
	co_boolean = 5,
	/** A user snowflake id */
	co_user = 6,
	/** A channel snowflake id */
	co_channel = 7,
	/** A role snowflake id */
	co_role = 8
};

/**
 * @brief This type is a variant that can hold any of the potential
 * native data types represented by the enum above.
 * It is used in interactions.
 */
typedef std::variant<std::string, int32_t, bool, snowflake> command_value;

/**
 * @brief This struct represents choices in a multiple choice option
 * for a command parameter.
 * It has both a string name, and a value parameter which is a variant,
 * meaning it can hold different potential types (see dpp::command_value)
 * that you can retrieve with std::get().
 */
struct CoreExport command_option_choice {
	std::string name;	//!< Option name (1-32 chars)
	command_value value;	//!< Option value

	/**
	 * @brief Construct a new command option choice object
	 */
	command_option_choice() = default;

	/**
	 * @brief Construct a new command option choice object
	 *
	 * @param n name to initialise with
	 * @param v value to initialise with
	 */
	command_option_choice(const std::string &n, command_value v);
};

/**
 * @brief helper function to serialize a command_option_choice to json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param choice command_option_choice to be serialized
 */
void to_json(nlohmann::json& j, const command_option_choice& choice);

/**
 * @brief Each command option is a command line parameter.
 * It can have a type (see dpp::command_option_type), a name,
 * a description, can be required or optional, and can have
 * zero or more choices (for multiple choice), plus options.
 * Adding options acts like sub-commands and can contain more
 * options.
 */
struct CoreExport command_option {
	command_option_type type;                    //!< Option type (what type of value is accepted)
	std::string name;                            //!< Option name (1-32 chars)
	std::string description;                     //!< Option description (1-100 chars)
	bool required;                               //!< True if this is a mandatory parameter
	std::vector<command_option_choice> choices;  //!< List of choices for multiple choice command
	std::vector<command_option> options;         //!< Sub-commands

	/**
	 * @brief Construct a new command option object
	 */
	command_option() = default;

	/**
	 * @brief Construct a new command option object
	 *
	 * @param t Option type
	 * @param name Option name
	 * @param description Option description
	 * @param required True if this is a mandatory parameter
	 */
	command_option(command_option_type t, const std::string &name, const std::string &description, bool required = false);

	/**
	 * @brief Add a multiple choice option
	 *
	 * @param o choice to add
	 * @return command_option& returns a reference to self for chaining of calls
	 */
	command_option& add_choice(const command_option_choice &o);

	/**
	 * @brief Add a sub-command option
	 *
	 * @param o Sub-command option to add
	 * @return command_option& return a reference to self for chaining of calls
	 */
	command_option& add_option(const command_option &o);
};

/**
 * @brief helper function to serialize a command_option to json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param opt command_option to be serialized
 */
void to_json(nlohmann::json& j, const command_option& opt);

/**
 * @brief Response types when responding to an interaction within on_interaction_create.
 * Do not use ir_acknowledge or ir::channel_message, as these are deprecated in the
 * Discord API spec. They are listed in this enum for completeness.
 */
enum interaction_response_type {
	ir_pong = 1,					//!< ACK a Ping
	ir_acknowledge = 2,				//!< DEPRECATED ACK a command without sending a message, eating the user's input
	ir_channel_message = 3,				//!< DEPRECATED respond with a message, eating the user's input
	ir_channel_message_with_source = 4,		//!< respond to an interaction with a message
	ir_deferred_channel_message_with_source = 5,	//!< ACK an interaction and edit a response later, the user sees a loading state
	ir_deferred_update_message = 6,			//!< for components, ACK an interaction and edit the original message later; the user does not see a loading state
	ir_update_message = 7				//!< for components, edit the message the component was attached to
};

/**
 * @brief A response to an interaction, used to reply to a command and initiate
 * a message, which can be hidden from others (ephemeral) or visible to all.
 *
 * The dpp::interaction_response object wraps a dpp::message object. To set the
 * message as 'ephemeral' (e.g. only the command issuer can see it) you should
 * add the dpp::m_ephemeral flag to the dpp::message::flags field. e.g.:
 *
 * `mymessage.flags |= dpp::m_ephemeral;`
 */
struct CoreExport interaction_response {

	/**
	 * @brief Response type from dpp::interaction_response_type.
	 * Should be one of ir_pong, ir_channel_message_with_source,
	 * or ir_deferred_channel_message_with_source.
	 */
	interaction_response_type type;

	/**
	 * @brief A message object. This pointer is always valid
	 * while the containing interaction_response exists.
	 */
	struct message* msg;

	/**
	 * @brief Construct a new interaction response object
	 */
	interaction_response();

	/**
	 * @brief Construct a new interaction response object
	 *
	 * @param t Type of reply
	 * @param m Message to reply with
	 */
	interaction_response(interaction_response_type t, const struct message& m);

	/**
	 * @brief Fill object properties from JSON
	 *
	 * @param j JSON to fill from
	 * @return interaction_response& Reference to self
	 */
	interaction_response& fill_from_json(nlohmann::json* j);

	/**
	 * @brief Build a json string for this object
	 *
	 * @return std::string JSON string
	 */
	std::string build_json() const;

	/**
	 * @brief Destroy the interaction response object
	 */
	~interaction_response();

};

/**
 * @brief Resolved snowflake ids to usernames.
 * TODO: Needs implementation. Not needed something that
 * functions as we have cache.
 */
struct CoreExport command_resolved {
};

/**
 * @brief Values in the command interaction.
 * These are the values specified by the user when actually issuing
 * the command on a channel or in DM.
 */
struct CoreExport command_data_option {
	std::string name;                          //!< the name of the parameter
	command_option_type type;                  //!< value of ApplicationCommandOptionType
	command_value value;                       //!< Optional: the value of the pair
	std::vector<command_data_option> options;  //!< Optional: present if this option is a group or subcommand
	dpp::snowflake target_id;                  //!< Non-zero target ID for context menu actions
};

/**
 * @brief helper function to deserialize a command_data_option from json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param cdo command_data_option to be deserialized
 */
void from_json(const nlohmann::json& j, command_data_option& cdo);

/** Types of interaction in the dpp::interaction class
 */
enum interaction_type {
	it_ping = 1,			//!< ping
	it_application_command = 2,	//!< application command (slash command)
	it_component_button = 3		//!< button click (component interaction)
};

/**
 * @brief Details of a command within an interaction.
 * This subobject represents the application command associated
 * with the interaction.
 */
struct CoreExport command_interaction {
	snowflake id;                              //!< the ID of the invoked command
	std::string name;                          //!< the name of the invoked command
	command_resolved resolved;                 //!< Optional: converted users + roles + channels
	std::vector<command_data_option> options;  //!< Optional: the params + values from the user
};

/**
 * @brief helper function to deserialize a command_interaction from json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param ci command_interaction to be deserialized
 */
void from_json(const nlohmann::json& j, command_interaction& ci);

enum component_type_t {
	cotype_button = 2,
	cotype_select = 3
};

/**
 * @brief A button click for a button component
 */
struct CoreExport component_interaction {
	uint8_t component_type;
	std::string custom_id;
	std::vector<std::string> values;
};

/**
 * @brief helper function to deserialize a component_interaction from json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param bi button_interaction to be deserialized
 */
void from_json(const nlohmann::json& j, component_interaction& bi);

/**
 * @brief An interaction represents a user running a command and arrives
 * via the dpp::cluster::on_interaction_create event.
 */
class CoreExport interaction : public managed {
public:
	snowflake application_id;                                   //!< id of the application this interaction is for
	uint8_t	type;                                               //!< the type of interaction
	std::variant<command_interaction, component_interaction> data; //!< Optional: the command data payload
	snowflake guild_id;                                         //!< Optional: the guild it was sent from
	snowflake channel_id;                                       //!< Optional: the channel it was sent from
	snowflake message_id;					    //!< Originating message id
	guild_member member;                                        //!< Optional: guild member data for the invoking user, including permissions
	user usr;                                                   //!< Optional: user object for the invoking user, if invoked in a DM
	std::string token;                                          //!< a continuation token for responding to the interaction
	uint8_t version;                                            //!< read-only property, always 1

	/**
	 * @brief Fill object properties from JSON
	 *
	 * @param j JSON to fill from
	 * @return interaction& Reference to self
	 */
	interaction& fill_from_json(nlohmann::json* j);

	/**
	 * @brief Build a json string for this object
	 *
	 * @param with_id True if to include the ID in the JSON
	 * @return std::string JSON string
	 */
	std::string build_json(bool with_id = false) const;
};

/**
 * @brief helper function to deserialize an interaction from json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param i interaction to be deserialized
 */
void from_json(const nlohmann::json& j, interaction& i);

/**
 * @brief type of permission in the dpp::command_permission class
 */
enum command_permission_type {
	cpt_role = 1,
	cpt_user = 2,
};

/**
 * @brief Application command permissions allow you to enable or
 * disable commands for specific users or roles within a guild
 */
class CoreExport command_permission {
public:
	snowflake id;                  //!< the ID of the role or uses
	command_permission_type type;  //!< the type of permission
	bool permission;               //!< true to allow, false, to disallow
};

/**
 * @brief helper function to serialize a command_permission to json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param cp command_permission to be serialized
 */
void to_json(nlohmann::json& j, const command_permission& cp);

/**
 * @brief Returned when fetching the permissions for a command in a guild.
 */
class CoreExport guild_command_permissions {
public:
	snowflake id;                                 //!< the id of the command
	snowflake application_id;                     //!< the id of the application the command belongs to
	snowflake guild_id;                           //!< the id of the guild
	std::vector<command_permission> permissions;  //!< the permissions for the command in the guild
};

/**
 * @brief helper function to serialize a guild_command_permissions to json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param gcp guild_command_permissions to be serialized
 */
void to_json(nlohmann::json& j, const guild_command_permissions& gcp);

enum slashcommand_contextmenu_type {
	ctxm_none = 0,
	ctxm_chat_input = 1,	//!< DEFAULT, these are the slash commands you're used to
	ctxm_user = 2,		//!< Add command to user context menu
	ctxm_message = 3	//!< Add command to message context menu
};

/**
 * @brief Represents an application command, created by your bot
 * either globally, or on a guild.
 */
class CoreExport slashcommand : public managed {
public:
	/**
	 * @brief Application id (usually matches your bots id)
	 */
	snowflake application_id;

	/**
	 * @brief Context menu type, defaults to none
	 * 
	 */
	slashcommand_contextmenu_type type;

	/**
	 * @brief Command name (1-32 chars)
	 */
	std::string name;

	/**
	 * @brief Command description (1-100 chars)
	 */
	std::string description;

	/**
	 * @brief Command options (parameters)
	 */
	std::vector<command_option> options;

	/**
	 * @brief whether the command is enabled by default when the app is added to a guild
	 */
	bool default_permission;

	/**
	 * @brief command permissions
	 */
	std::vector<command_permission> permissions;

	/**
	 * @brief Construct a new slashcommand object
	 */
	slashcommand();

	/**
	 * @brief Destroy the slashcommand object
	 */
	~slashcommand();

	/**
	 * @brief Add an option (parameter)
	 *
	 * @param o option (parameter) to add
	 * @return slashcommand& reference to self for chaining of calls
	 */
	slashcommand& add_option(const command_option &o);

	/**
	 * @brief Set the type of the slash command (only for context menu entries)
	 * 
	 * @param _type Type of context menu entry this command represents
	 * @return slashcommand& reference to self for chaining of calls
	 */
	slashcommand& set_type(slashcommand_contextmenu_type _type);

	/**
	 * @brief Set the name of the command
	 *
	 * @param n name of command
	 * @return slashcommand& reference to self for chaining of calls
	 */
	slashcommand& set_name(const std::string &n);

	/**
	 * @brief Set the description of the command
	 *
	 * @param d description
	 * @return slashcommand& reference to self for chaining of calls
	 */
	slashcommand& set_description(const std::string &d);

	/**
	 * @brief Set the application id of the command
	 *
	 * @param i application id
	 * @return slashcommand& reference to self for chaining of calls
	 */
	slashcommand& set_application_id(snowflake i);

	/**
	 * @brief Adds a permission to the command
	 *
	 * @param p permission to add
	 * @return slashcommand& reference to self for chaining of calls
	 */
	slashcommand& add_permission(const command_permission& p);

	/**
	 * @brief Disable default permissions, command will be unusable unless
	 *        permissions are overriden with add_permission and
	 *        dpp::guild_command_edit_permissions
	 *
	 * @return slashcommand& reference to self for chaining of calls
	 */
	slashcommand& disable_default_permissions();

	/**
	 * @brief Fill object properties from JSON
	 *
	 * @param j JSON to fill from
	 * @return slashcommand& Reference to self
	 */
	slashcommand& fill_from_json(nlohmann::json* j);

	/**
	 * @brief Build a json string for this object
	 *
	 * @param with_id True if to include the ID in the JSON
	 * @return std::string JSON string
	 */
	std::string build_json(bool with_id = false) const;
};

/**
 * @brief helper function to serialize a slashcommand to json
 *
 * @see https://github.com/nlohmann/json#arbitrary-types-conversions
 *
 * @param j output json object
 * @param cmd slashcommand to be serialized
 */
void to_json(nlohmann::json& j, const slashcommand& cmd);

/**
 * @brief A group of application slash commands
 */
typedef std::unordered_map<snowflake, slashcommand> slashcommand_map;

};
