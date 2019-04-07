#include "command_args.h"

#include <exception>
#include <string>
using namespace std::string_literals;

auto parse_window_size(gsl::span<char const* const> args) -> math::vector2i {
    if(args.size() < 2 || args[0][0] == '-' || args[1][0] == '-') {
        throw std::invalid_argument("Missing arguments after '--window-size'");
    }

    char* arg_end;
    int const x = std::strtol(args[0], &arg_end, 10);
    if(arg_end == args[0] || x <= 0) {
        throw std::invalid_argument("First argument to '--window-size' was not a positive integer: "s + args[0]);
    }
    int const y = std::strtol(args[1], &arg_end, 10);
    if(arg_end == args[1] || x <= 0) {
        throw std::invalid_argument("Second argument to '--window-size' was not a positive integer: "s + args[1]);
    }

    return {x, y};
}

auto parse_args(gsl::span<char const* const> args) -> command_args {
    command_args result;
    for(int i = 0; i < args.size(); ++i) {
        std::string_view const arg = args[i];
        if(arg == "--window-size") {
            result.window_size = parse_window_size(args.subspan(i + 1));
            i += 2;
        } else if(arg == "--print-video-drivers") {
            result.print_video_drivers = true;
        }
    }
    return result;
}