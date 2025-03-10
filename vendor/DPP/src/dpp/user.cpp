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
#include <dpp/discordevents.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>

using json = nlohmann::json;

/* A mapping of discord's flag values to our bitmap (theyre different bit positions to fit other stuff in) */
std::map<uint32_t, dpp::user_flags> usermap = {
	{ 1 << 0,       dpp::u_discord_employee },
	{ 1 << 1,       dpp::u_partnered_owner },
	{ 1 << 2,       dpp::u_hypesquad_events },
	{ 1 << 3,       dpp::u_bughunter_1 },
	{ 1 << 6,       dpp::u_house_bravery },
	{ 1 << 7,       dpp::u_house_brilliance },
	{ 1 << 8,       dpp::u_house_balanace },
	{ 1 << 9,       dpp::u_early_supporter },
	{ 1 << 10,      dpp::u_team_user },
	{ 1 << 14,      dpp::u_bughunter_2 },
	{ 1 << 16,      dpp::u_verified_bot },
	{ 1 << 17,      dpp::u_verified_bot_dev },
	{ 1 << 18,      dpp::u_certified_moderator }
};

namespace dpp {

user::user() :
	managed(),
	discriminator(0),
	flags(0),
	refcount(1)
{
}

user::~user()
{
}

std::string user::get_avatar_url()  const {
	/* XXX: Discord were supposed to change their CDN over to discord.com, they havent.
	 * At some point in the future this URL *will* change!
	 */
	return fmt::format("https://cdn.discordapp.com/avatars/{}/{}{}.{}",
		this->id,
		(has_animated_icon() ? "a_" : ""),
		this->avatar.to_string(),
		(has_animated_icon() ? "gif" : "png")
	);
}

bool user::is_bot() const {
	 return this->flags & u_bot;
}

bool user::is_system() const {
	 return this->flags & u_system;
}

bool user::is_mfa_enabled() const {
	 return this->flags & u_mfa_enabled;
}

bool user::is_verified() const {
	 return this->flags & u_verified;
}

bool user::has_nitro_full() const {
	 return this->flags & u_nitro_full;
}

bool user::has_nitro_classic() const {
	 return this->flags & u_nitro_classic;
}

bool user::is_discord_employee() const {
	 return this->flags & u_discord_employee;
}

bool user::is_partnered_owner() const {
	 return this->flags & u_partnered_owner;
}

bool user::has_hypesquad_events() const {
	 return this->flags & u_hypesquad_events;
}

bool user::is_bughunter_1() const {
	 return this->flags & u_bughunter_1;
}

bool user::is_house_bravery() const {
	 return this->flags & u_house_bravery;
}

bool user::is_house_brilliance() const {
	 return this->flags & u_house_brilliance;
}

bool user::is_house_balanace() const {
	 return this->flags & u_house_balanace;
}

bool user::is_early_supporter() const {
	 return this->flags & u_early_supporter;
}

bool user::is_team_user() const {
	 return this->flags & u_team_user;
}

bool user::is_bughunter_2() const {
	 return this->flags & u_bughunter_2;
}

bool user::is_verified_bot() const {
	 return this->flags & u_verified_bot;
}

bool user::is_verified_bot_dev() const {
	 return this->flags & u_verified_bot_dev;
}

bool user::is_certified_moderator() const {
	 return this->flags & u_certified_moderator;
}

bool user::has_animated_icon() const {
	return this->flags & u_animated_icon;
}

user& user::fill_from_json(json* j) {
	j->get_to(*this);
	return *this;
}

void from_json(const nlohmann::json& j, user& u) {
	u.id = SnowflakeNotNull(&j, "id");
	u.username = StringNotNull(&j, "username");

	std::string av = StringNotNull(&j, "avatar");
	if (av.length() > 2 && av.substr(0, 2) == "a_") {
		av = av.substr(2, av.length());
		u.flags |= u_animated_icon;
	}
	u.avatar = av;

	u.discriminator = SnowflakeNotNull(&j, "discriminator");

	u.flags |= BoolNotNull(&j, "bot") ? dpp::u_bot : 0;
	u.flags |= BoolNotNull(&j, "system") ? dpp::u_system : 0;
	u.flags |= BoolNotNull(&j, "mfa_enabled") ? dpp::u_mfa_enabled : 0;
	u.flags |= BoolNotNull(&j, "verified") ? dpp::u_verified : 0;
	u.flags |= Int8NotNull(&j, "premium_type") == 1 ? dpp::u_nitro_classic : 0;
	u.flags |= Int8NotNull(&j, "premium_type") == 2 ? dpp::u_nitro_full : 0;
	uint32_t flags = Int32NotNull(&j, "flags");
	for (auto & flag : usermap) {
		if (flags & flag.first) {
			u.flags |= flag.second;
		}
	}
}

};
