#include <catch.hpp>

#include "serial/config.h"

#include <sstream>

TEST_CASE("INI Config file", "[serial]") {
    std::stringstream ss;
    ss << "key1=value1" << "\n";
    ss << "; This is an ignored comment" << "\n";
    ss << "[section1]" << "\n";
    ss << "key2=value2" << "\n";
    ss << "key3=value3" << "\n";
    ss << "[section2]" << "\n";
    ss << "key4=value4" << "\n";

    int i = 0;
    auto const result = serial::parse_config(ss, [] (gsl::cstring_span<> section, gsl::cstring_span<> key, gsl::cstring_span<> value, void* user_data) {
        int & counter = *static_cast<int*>(user_data);
        switch(counter) {
            case 0:
                REQUIRE(section == "");
                REQUIRE(key == "key1");
                REQUIRE(value == "value1");
                break;
            case 1:
                REQUIRE(section == "section1");
                REQUIRE(key == "key2");
                REQUIRE(value == "value2");
				break;
            case 2:
                REQUIRE(section == "section1");
                REQUIRE(key == "key3");
                REQUIRE(value == "value3");
				break;
			case 3:
				REQUIRE(section == "section2");
				REQUIRE(key == "key4");
				REQUIRE(value == "value4");
				break;
			default:
				REQUIRE(false);
        }
        ++counter;
    }, &i);

	REQUIRE(result);
	REQUIRE(i == 4);
}