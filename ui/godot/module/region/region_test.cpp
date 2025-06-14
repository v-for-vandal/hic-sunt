#include "region.hpp"

#include <absl/container/flat_hash_map.h>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <ui/godot/module/types/godot_base_types.hpp>

#include <gtest/gtest.h>

namespace hs::godot {

using namespace ::godot;

// Alas, godot classes can not be created without godot runtime
/*
TEST(RegionObject, AbseilMap) {
    // This tests checks that abesil map works with godot string and string ids
    absl::flat_hash_map<StringName, String> user_data_string;

    StringName key{"some_key"};
    String value{"some_value"};

    user_data_string[key] = value;

    ASSERT_EQ(value, user_data_string[key]);
}
*/


}
