#include "config_args.h"

#include "serial/config.h"

auto parse_config_args(std::istream & i) -> tl::expected<config_args, serial::error> {
    config_args args;
    auto result = serial::parse_config(i, [] (gsl::cstring_span<> section, gsl::cstring_span<> key, gsl::cstring_span<> value, void* user_data) {
        auto & args = *static_cast<config_args*>(user_data);
        if(section == "resource") {
            if(key == "path") {
                args.resource_config.path = to_string(value);
            }
        }
    }, &args);
    if(!result) {
        return tl::make_unexpected(std::move(result).error());
    }
    return args;
}