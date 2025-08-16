#pragma once

#ifndef MSR_RAPL_POWER_UNIT
#    define MSR_RAPL_POWER_UNIT     0x606
#endif
/*
 * Platform specific RAPL Domains.
 * Note that PP1 RAPL Domain is supported on 062A only
 * And DRAM RAPL Domain is supported on 062D only
 */
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT    0x610
#ifndef MSR_PKG_ENERGY_STATUS
#    define MSR_PKG_ENERGY_STATUS   0x611
#endif
#define MSR_PKG_PERF_STATUS         0x613
#ifndef MSR_PKG_POWER_INFO
#    define MSR_PKG_POWER_INFO      0x614
#endif

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT         0x638
#define MSR_PP0_ENERGY_STATUS       0x639
#define MSR_PP0_POLICY              0x63A
#define MSR_PP0_PERF_STATUS         0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT         0x640
#define MSR_PP1_ENERGY_STATUS       0x641
#define MSR_PP1_POLICY              0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT        0x618
#ifndef MSR_DRAM_ENERGY_STATUS
#    define MSR_DRAM_ENERGY_STATUS  0x619
#endif
#define MSR_DRAM_PERF_STATUS        0x61B
#define MSR_DRAM_POWER_INFO         0x61C

/* PSYS RAPL Domain */
#define MSR_PLATFORM_ENERGY_STATUS  0x64d

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET           0
#define POWER_UNIT_MASK             0x0F

#define ENERGY_UNIT_OFFSET          0x08
#define ENERGY_UNIT_MASK            0x1F00

#define TIME_UNIT_OFFSET            0x10
#define TIME_UNIT_MASK              0xF000

// #define SIGNATURE_MASK				0xFFFF0
// #define IVYBRIDGE_E					0x306F0
// #define SANDYBRIDGE_E				0x206D0