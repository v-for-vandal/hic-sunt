#include <gtest/gtest.h>
#include <session/session.pb.h>

#include <core/session/session_ut.hpp>
#include <core/utils/serialize.hpp>

#include "session.hpp"

namespace hs::session {

TEST(StdSessionSerialize, RoundTripPreservesWorldRulesetAndIndexes) {
  auto source = test::MakePreparedSession();
  ASSERT_TRUE(source.CreateCivilizationScope("civ.serialize").has_value());
  source.SetCurrentTurn(5);

  proto::session::Session proto_session;
  SerializeTo(source, proto_session);

  auto parsed = ParseFrom(proto_session, serialize::To<StdSession>{});

  EXPECT_NE(parsed.GetWorld(), nullptr);
  EXPECT_TRUE(parsed.GetWorld()->HasCivilization("civ.serialize"));
  EXPECT_EQ(parsed.GetCurrentTurn(), 5u);
  EXPECT_TRUE(parsed.GetScopesById().contains("world.root"));
  EXPECT_EQ(parsed.GetEffects().size(), source.GetEffects().size());
}

}  // namespace hs::session
