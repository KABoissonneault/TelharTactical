#pragma once

#include <optional>
#include <string>
#include <map>
#include <iosfwd>

#include <string_view>

#include "serial/error.h"
#include <tl/expected.hpp>

class config_args {
public:
	auto get_value(std::string_view section, std::string_view key) const noexcept->std::optional<std::string_view>;
	void set_value(std::string_view section, std::string_view key, std::string_view value);

private:
	std::map<std::string, std::map<std::string, std::string, std::less<void>>, std::less<void>> config_data;
};

auto parse_config_args(std::istream & i) -> tl::expected<config_args, serial::error>;