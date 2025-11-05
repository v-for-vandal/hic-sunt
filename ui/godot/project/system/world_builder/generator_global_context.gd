extends RefCounted

## This is base class for global context. You can extend it and use
## in your own world generator, or just stick to this one
class_name WorldGeneratorGlobalContext

## Debug control. Allows one to send debug views to the system.
var debug_control: WorldGeneratorDebugControl

## Some seed. No meaning attached
var seed : int = 0

## Arbitary attributes
var custom :  Dictionary[StringName, Variant] = {}
