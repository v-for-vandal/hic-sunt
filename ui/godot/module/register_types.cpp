#include <gdextension_interface.h>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <iostream>
#include <ui/godot/module/game/pnl_object.hpp>
#include <ui/godot/module/game/ruleset_object.hpp>
#include <ui/godot/module/region/cell_object.hpp>
#include <ui/godot/module/region/region_object.hpp>
#include <ui/godot/module/scope/scope_object.hpp>
#include <ui/godot/module/system/spdlog_sink.hpp>
#include <ui/godot/module/system/surface_event.hpp>
#include <ui/godot/module/system/system.hpp>
#include <ui/godot/module/terra/plane_object.hpp>
#include <ui/godot/module/terra/world_object.hpp>

namespace hs::godot {

using namespace godot;

void initialize_example_module(ModuleInitializationLevel p_level) {
  if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
    // for some reason, this initialization level never happens
  }
  if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
    /* initialize spdlog */
    auto logger = godot_logger_mt("godot_logger");
    logger->info("Initialized godot sink");
    spdlog::set_default_logger(logger);
    // ClassDB::register_class<HicSuntSystem>();
    ClassDB::register_class<RegionObject>();
    ClassDB::register_class<PnlObject>();
    ClassDB::register_class<PlaneObject>();
    ClassDB::register_class<WorldObject>();
    ClassDB::register_class<RulesetObject>();
    ClassDB::register_class<CellObject>();
    ClassDB::register_class<ScopeObject>();
  }
}

void uninitialize_example_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT
sichunt_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                     const GDExtensionClassLibraryPtr p_library,
                     GDExtensionInitialization *r_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 r_initialization);

  init_obj.register_initializer(initialize_example_module);
  init_obj.register_terminator(uninitialize_example_module);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_CORE);

  return init_obj.init();
}
}

}  // namespace hs::godot
