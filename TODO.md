Scope:
  * support TTL for modifiers
  * support serialization, including serialization from inside world/region/cell.
    Most likely we need to implement some kind of placeholder with id, and later pull actual scope from Session
Ruleset:
  * Support returning errors when loading. Right now we return bool and it is always true.

Session:
  * Implement cleaning modifiers that are no longer present in loaded ruleset. But care
    must be taken for manual modifiers:
     1. modifiers for world generation. We could require having effect for them
     2. core.turn - we set it manually. May be some flag for variable definition to mark
        this variable as manual? Or as 'system'

Variables:
  * Support variables with disabled multiplier. For example, 'core.turn' can not have
    any multipliers.
  * Support immutable variables that can not be changed in descendant scopes. Example:
    'core.turn'
  * Support variables that can only exist in specific type of scopes. For example,
    variable that influences job production can not exist in scope 'ARMY'
