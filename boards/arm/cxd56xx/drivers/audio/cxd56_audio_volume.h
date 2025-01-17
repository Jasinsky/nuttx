/****************************************************************************
 * boards/arm/cxd56xx/drivers/audio/cxd56_audio_volume.h
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

#ifndef __BOARDS_ARM_CXD56XX_DRIVERS_AUDIO_CXD56_AUDIO_VOLUME_H
#define __BOARDS_ARM_CXD56XX_DRIVERS_AUDIO_CXD56_AUDIO_VOLUME_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <arch/chip/audio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

CXD56_AUDIO_ECODE cxd56_audio_volume_set(cxd56_audio_volid_t id,
                                         int16_t vol);
CXD56_AUDIO_ECODE cxd56_audio_volume_mute(cxd56_audio_volid_t id);
CXD56_AUDIO_ECODE cxd56_audio_volume_unmute(cxd56_audio_volid_t id);
CXD56_AUDIO_ECODE cxd56_audio_volume_mute_fade(cxd56_audio_volid_t id,
                                               bool wait);
CXD56_AUDIO_ECODE cxd56_audio_volume_unmute_fade(cxd56_audio_volid_t id,
                                                 bool wait);

#endif /* __BOARDS_ARM_CXD56XX_DRIVERS_AUDIO_CXD56_AUDIO_VOLUME_H */
