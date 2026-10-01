#include <gtest/gtest.h>
#include <region/cell.pb.h>

#include <core/utils/serialize.hpp>

#include "cell.hpp"

namespace hs::region {

using StdCell = Cell<>;
using StdScope = scope::Scope<>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;

namespace {

scope::ScopeParseContext<StdBaseTypes> MakeScopeContext(StdCell& source) {
  scope::ScopeParseContext<StdBaseTypes> context;
  source.VisitScopes([&context](const auto& scope_ptr) {
    proto::scope::Scope proto_scope;
    SerializeTo(*scope_ptr, proto_scope);
    StdScopePtr parsed_scope{ParseFrom(proto_scope, serialize::To<StdScope>{}, context)};
    context.scopes_by_id.try_emplace(parsed_scope->GetId(), parsed_scope);
  });
  return context;
}

}  // namespace

TEST(StdCellSerialize, RoundTripPreservesScopeAndImprovements) {
  StdCell source;
  StdScopePtr improvement{"improvement.alpha", types::ScopeType::SCOPE_TYPE_IMPROVEMENT};
  ASSERT_TRUE(source.AddImprovement(2, improvement).has_value());

  proto::region::Cell proto_cell;
  SerializeTo(source, proto_cell);

  auto context = MakeScopeContext(source);
  auto parsed = ParseFrom(proto_cell, serialize::To<StdCell>{}, context);

  EXPECT_EQ(parsed.GetScope()->GetId(), source.GetScope()->GetId());
  EXPECT_EQ(parsed.GetScope()->GetType(), source.GetScope()->GetType());
  EXPECT_TRUE(parsed.HasImprovement(2));
  EXPECT_EQ(parsed.GetImprovementId(2), "improvement.alpha");
  EXPECT_EQ(parsed.GetImprovement(2)->GetParent()->GetId(), parsed.GetScope()->GetId());
}

}  // namespace hs::region
