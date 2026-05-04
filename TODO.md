Scope:
  * support TTL for modifiers
  * support serialization, including serialization from inside world/region/cell.
    Most likely we need to implement some kind of placeholder with id, and later pull actual scope from Session
Ruleset:
  * Support returning errors when loading. Right now we return bool and it is always true.

Session:
  * Implement cleaning modifiers that are no longer present in loaded ruleset
