#pragma once

#ifndef GANESHA_DYNAMIC_METRICS_H
#define GANESHA_DYNAMIC_METRICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "gsh_types.h"
#include "monitoring.h"

#ifdef HAVE_PROCPS
#include <proc/readproc.h>
#endif

void nfs_metrics_collect_now(void);
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nfs_metrics_update_cb_t)(void);
#ifdef USE_MONITORING

/** Register a daemon-side collector to be called just before scrape. */
void nfs_register_metrics_collector(nfs_metrics_update_cb_t cb);

/** Called by the monitoring code to run the registered collector (if any). */
//void nfs_metrics_collect_now(void);

void update_metrics(void);




/* Back-compat aliases (old names still callable) */

typedef void (*ganesha_collect_export_info_cb_t)(void);

bool ganesha_register_export_info_collector(ganesha_collect_export_info_cb_t cb);
bool ganesha_unregister_export_info_collector(ganesha_collect_export_info_cb_t cb);
void ganesha_collect_export_info_now(void);



#else // USE_MONITORING

#ifndef UNUSED
#define UNUSED_ATTR __attribute__((unused))
#define UNUSED(...) UNUSED_(__VA_ARGS__)
#define UNUSED_(arg) NOT_USED_##arg UNUSED_ATTR
#endif

static inline void nfs_register_metrics_collector(nfs_metrics_update_cb_t UNUSED(cb)) 
{
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* GANESHA_DYNAMIC_METRICS_H */
