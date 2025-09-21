
namespace hs::terra {

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes> &source,
                 proto::terra::Cell &proto_destination) {
  proto_destination.Clear();

  SerializeTo(source.GetRegion(), *proto_destination.mutable_region());

  return;
}

template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::terra::Cell &source,
                          serialize::To<Cell<BaseTypes>>) {
  Cell<BaseTypes> result;
  if (source.has_region()) {
    result.SetRegion(
        ParseFrom(source.region(), serialize::To<region::Region<BaseTypes>>{}));
  }

  return result;
}

} // namespace hs::terra
