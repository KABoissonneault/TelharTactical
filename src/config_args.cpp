#include "config_args.h"

#include "serial/config.h"

auto config_args::get_value(std::string_view section, std::string_view key) const noexcept -> std::optional<std::string_view> {
	auto const it_section = config_data.find(section);
	if(it_section == config_data.end()) {
		return std::nullopt;
	}
	auto const& section_map = it_section->second;
	auto const it_data = section_map.find(key);
	if(it_data == section_map.end()) {
		return std::nullopt;
	}
	return it_data->second;
}

void config_args::set_value(std::string_view section, std::string_view key, std::string_view value) {
	auto it_section = config_data.lower_bound(section);
	if(it_section == config_data.end()) {
		it_section = config_data.emplace(std::pair{section, std::map<std::string, std::string, std::less<void>>()}).first;
	} else if(it_section->first != section) {
		it_section = config_data.emplace_hint(it_section, std::pair{section, std::map<std::string, std::string, std::less<void>>()});
	}
	auto & section_map = it_section->second;

	auto const it_key = section_map.lower_bound(key);
	if(it_key == section_map.end()) {
		section_map.emplace(key, value);
	} else if(it_key->first != key) {
		section_map.emplace_hint(it_key, key, value);
	} else {
		it_key->second = value;
	}
}

auto parse_config_args(std::istream & i) -> tl::expected<config_args, serial::error> {
    config_args args;
    auto result = serial::parse_config(i, [] (std::string_view section, std::string_view key, std::string_view value, void* user_data) {
        auto & args = *static_cast<config_args*>(user_data);
        args.set_value(section, key, value);
    }, &args);
    if(!result) {
        return tl::make_unexpected(std::move(result).error());
    }
    return args;
}