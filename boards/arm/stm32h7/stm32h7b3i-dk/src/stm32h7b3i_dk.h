/****************************************************************************
 * boards/arm/stm32h7/stm32h7b3i-dk/src/stm32h7b3i_dk.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __BOARDS_ARM_STM32H7_STM32H7B3I_DK_SRC_STM32H7B3I_DK_H
#define __BOARDS_ARM_STM32H7_STM32H7B3I_DK_SRC_STM32H7B3I_DK_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* procfs File System */

#ifdef CONFIG_FS_PROCFS
#  ifdef CONFIG_NSH_PROC_MOUNTPOINT
#    define STM32_PROCFS_MOUNTPOINT CONFIG_NSH_PROC_MOUNTPOINT
#  else
#    define STM32_PROCFS_MOUNTPOINT "/proc"
#  endif
#endif

/* LED
 *
 * The STM32H7B3I-DK has two user LEDs:
 *   LD2 (Blue)  connected to PG2  (active high)
 *   LD3 (Red)   connected to PG11 (active high)
 */

#define GPIO_LD2       (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                        GPIO_OUTPUT_CLEAR | GPIO_PORTG | GPIO_PIN2)
#define GPIO_LD3       (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                        GPIO_OUTPUT_CLEAR | GPIO_PORTG | GPIO_PIN11)

#define GPIO_LED_BLUE  GPIO_LD2
#define GPIO_LED_RED   GPIO_LD3

#define LED_DRIVER_PATH "/dev/userleds"

/* BUTTONS
 *
 * The User push-button B1 is connected to GPIO PC13.
 * A high value will be sensed when the button is depressed.
 */

#define GPIO_BTN_USER  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | \
                        GPIO_PORTC | GPIO_PIN13)

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=n && CONFIG_BOARDCTL=y :
 *     Called from the NSH library via board_app_initialize()
 *
 ****************************************************************************/

int stm32_bringup(void);

#endif /* __BOARDS_ARM_STM32H7_STM32H7B3I_DK_SRC_STM32H7B3I_DK_H */
