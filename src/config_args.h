#pragma once

#include <optional>
#include <string>
#include <map>
#include <iosfwd>

#include <gsl/string_span>

#include "serial/error.h"
#include <tl/expected.hpp>

class config_args {
public:

	auto get_value(gsl::cstring_span<> section, gsl::cstring_span<> key) const noexcept -> std::optional<gsl::cstring_span<>> {
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
	auto set_value(gsl::cstring_span<> section, gsl::cstring_span<> key, gsl::cstring_span<> value) {
		config_data[to_string(section)][to_string(key)] = to_string(value);
	}

private:
	std::map<std::string, std::map<std::string, std::string, std::less<void>>, std::less<void>> config_data;
};

auto parse_config_args(std::istream & i) -> tl::expected<config_args, serial::error>;