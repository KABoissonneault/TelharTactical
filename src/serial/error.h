#pragma once

#include <system_error>

namespace serial {
    struct error {
        std::error_code code;
        std::string description;
    };
}