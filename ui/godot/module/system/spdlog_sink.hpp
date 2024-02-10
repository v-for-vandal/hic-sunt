#include "spdlog/sinks/base_sink.h"
#include <godot_cpp/core/error_macros.hpp>

template<typename Mutex>
class godot_sink : public spdlog::sinks::base_sink<Mutex>
{
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
      spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        godot::_err_print_error("spdlog", "spdlog", 0, formatted.data(), false, false);
        //Godot::print(formatted.data());
    }

    void flush_() override {}
};

using godot_sink_mt = godot_sink<std::mutex>;

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<spdlog::logger> godot_logger_mt(const std::string &logger_name)
{
    return Factory::template create<godot_sink_mt>(logger_name);
}
