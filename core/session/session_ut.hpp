#pragma once

#include <core/session/session.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::session {

using StdSession = Session<StdBaseTypes>;

namespace test {

StdSession MakePreparedSession();

}  // namespace test
}  // namespace hs::session
