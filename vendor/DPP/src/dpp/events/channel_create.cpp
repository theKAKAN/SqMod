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
#include <dpp/discord.h>
#include <dpp/event.h>
#include <string>
#include <iostream>
#include <fstream>
#include <dpp/discordclient.h>
#include <dpp/discordevents.h>
#include <dpp/discord.h>
#include <dpp/cache.h>
#include <dpp/stringops.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>

using json = nlohmann::json;

namespace dpp { namespace events {

using namespace dpp;

/**
 * @brief Handle event
 * 
 * @param client Websocket client (current shard)
 * @param j JSON data for the event
 * @param raw Raw JSON string
 */
void channel_create::handle(discord_client* client, json &j, const std::string &raw) {
	json& d = j["d"];
	
	dpp::channel* c = dpp::find_channel(SnowflakeNotNull(&d, "id"));
	if (!c) {
		c = new dpp::channel();
	}
	c->fill_from_json(&d);
	dpp::get_channel_cache()->store(c);
	if (c->recipients.size()) {
		for (auto & u : c->recipients) {
			client->log(dpp::ll_debug, fmt::format("Got a DM channel {} for user {}", c->id, u));
			client->creator->set_dm_channel(u, c->id);
		}
	}
	dpp::guild* g = dpp::find_guild(c->guild_id);
	if (g) {
		g->channels.push_back(c->id);

		if (client->creator->dispatch.channel_create) {
			dpp::channel_create_t cc(client, raw);
			cc.created = c;
			cc.creating_guild = g;
			client->creator->dispatch.channel_create(cc);
		}
	}
}

}};