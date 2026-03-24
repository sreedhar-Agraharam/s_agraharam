#include "monitoring_hooks.h"
#include <atomic>

#include <algorithm>
#include <memory>
#include <vector>

#ifdef USE_MONITORING

using cb_t = nfs_metrics_update_cb_t;

//static std::atomic<std::shared_ptr<std::vector<cb_t>>> g_callbacks;
static std::shared_ptr<std::vector<cb_t>> g_callbacks;
static constexpr std::size_t kMaxCallbacks = 64;


// Ensure a non-null, empty vector by default for readers.
struct CallbacksInit {
  CallbacksInit() {
    std::atomic_store_explicit(
        &g_callbacks,
        std::make_shared<std::vector<cb_t>>(),
        std::memory_order_release);
  }
} g_callbacks_init;


extern "C" void nfs_register_metrics_collector(nfs_metrics_update_cb_t cb) {

  if (!cb) return;

  for (;;) {
  //  auto cur = g_callbacks.load(std::memory_order_acquire);
   auto cur = std::atomic_load_explicit(&g_callbacks, std::memory_order_acquire);
  // Fast-path: if already present, succeed without changing the snapshot.
    if (std::find(cur->begin(), cur->end(), cb) != cur->end()) {
      return;  // already registered
    }

    // Copy-on-write
    auto next = std::make_shared<std::vector<cb_t>>(*cur);
    if (next->size() >= kMaxCallbacks) {
      return ;  // capacity exceeded (tune kMaxCallbacks if needed)
    }
    next->push_back(cb);

    // Try to publish
//    if (g_callbacks.compare_exchange_weak(cur, next,
//                                          std::memory_order_release,
//                                          std::memory_order_acquire)) {

    if (std::atomic_compare_exchange_weak_explicit(
            &g_callbacks, &cur, next,
            std::memory_order_release, std::memory_order_acquire)) {


    return;
    }
    // 
	}


}

//extern "C"
//{
void nfs_metrics_collect_now(void) {

//  auto snapshot = g_callbacks.load(std::memory_order_acquire);
   auto snapshot = std::atomic_load_explicit(&g_callbacks, std::memory_order_acquire);

// Iterate in registration order
  for (auto &fn : *snapshot) {
    if (fn) {
      // Ensure no exceptions escape C boundary; also protects
      // metrics path from accidental throw in any one callback.
      try {
        fn();
      } catch (...) {
        // If you have a logging macro available here, log the failure.
        // e.g., LogWarn(COMPONENT_MONITORING, "export info callback threw");
      }
    }
  }
}


//}


/* Back-compat shims */
extern "C" bool ganesha_register_export_info_collector(cb_t cb) {
  nfs_register_metrics_collector(cb);
  return true;
}
//extern "C" bool ganesha_unregister_export_info_collector(cb_t cb) {
//  return nfs_unregister_metrics_collector(cb);
//}
extern "C" void ganesha_collect_export_info_now(void) {
  nfs_metrics_collect_now();
}







#else  // !USE_MONITORING

using cb_t = nfs_metrics_update_cb_t;
//extern "C" void nfs_register_metrics_collector(nfs_metrics_collector_cb_t) { return ; }
//extern "C" bool nfs_unregister_metrics_collector(nfs_metrics_collector_cb_t) { return true; }
//extern "C" void nfs_metrics_collect_now(void) {}

//extern "C" bool ganesha_register_export_info_collector(nfs_metrics_collector_cb_t) { return true; }
//extern "C" bool ganesha_unregister_export_info_collector(nfs_metrics_collector_cb_t) { return true; }
extern "C" void ganesha_collect_export_info_now(void) {}

#endif





