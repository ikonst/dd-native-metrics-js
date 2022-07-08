#include "GarbageCollection.hpp"

namespace datadog {
  GarbageCollection::GarbageCollection() {
#if NODE_MODULE_VERSION >= 108
    types_[1] = "scavenge";
    types_[2] = "minor_mark_compact";
    types_[4] = "mark_sweep_compact";
    types_[8] = "incremental_marking";
    types_[16] = "process_weak_callbacks";
    types_[31] = "all";
#else
    types_[1] = "scavenge";
    types_[2] = "mark_sweep_compact";
    types_[3] = "all";
    types_[4] = "incremental_marking";
    types_[8] = "process_weak_callbacks";
    types_[15] = "all";
#endif

    pause_[v8::GCType::kGCTypeAll] = Histogram();
  }

  void GarbageCollection::before(v8::GCType type) {
    start_time_ = uv_hrtime();
  }

  void GarbageCollection::after(v8::GCType type) {
    uint64_t usage = uv_hrtime() - start_time_;

    if (pause_.find(type) == pause_.end()) {
      pause_[type] = Histogram();
    }

    pause_[type].add(usage);
    pause_[v8::GCType::kGCTypeAll].add(usage);
  }

  void GarbageCollection::inject(Object carrier) {
    Object value;

    for (auto &it : pause_) {
      value.set(types_[it.first], it.second);
      it.second.reset();
    }

    carrier.set("gc", value);
  }
}
