/*
 * Copyright (c) 2020 TriaGnoSys GmbH
 * Copyright (c) 2025 Silicon Laboratories Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 * @brief Silabs EFR32MG24 MCU pin definitions.
 *
 * This file contains pin configuration data required by different MCU
 * modules to correctly configure GPIO controller.
 */

#ifndef _SILABS_EFR32MG24_SOC_PINMAP_H_
#define _SILABS_EFR32MG24_SOC_PINMAP_H_

#include <em_gpio.h>

#ifdef CONFIG_LOG_BACKEND_SWO
#define PIN_SWO { gpioPortA, 3, gpioModePushPull, 1 }
#endif  /* CONFIG_LOG_BACKEND_SWO */

#endif  /* _SILABS_EFR32MG24_SOC_PINMAP_H_ */
