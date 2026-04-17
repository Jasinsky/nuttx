/****************************************************************************
 * boards/arm/stm32h7/stm32h7b3i-dk/scripts/flash.ld
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

/* The STM32H7B3LI has 2048Kb of main FLASH memory.  The flash memory is
 * partitioned into 256 sectors of 8Kb each, arranged in two banks:
 *
 *   1) User Flash memory:
 *
 *      Bank 1: Start address 0x0800:0000 to 0x080F:FFFF (128 sectors, 8Kb each)
 *      Bank 2: Start address 0x0810:0000 to 0x081F:FFFF (128 sectors, 8Kb each)
 *
 * The STM32H7B3LI has 1376 Kb total SRAM:
 *
 *   1) TCM SRAMs are dedicated to the Cortex-M7 and are accessible with
 *      0 wait states by the Cortex-M7 and by MDMA through AHBS slave bus
 *
 *      1.1) 128Kb of DTCM-RAM beginning at address 0x2000:0000
 *
 *           The DTCM-RAM is organized as 2 x 64Kb DTCM-RAMs on 2 x 32 bit
 *           DTCM ports.
 *
 *      1.2)  64Kb of ITCM-RAM beginning at address 0x0000:0000
 *
 *   2) AXI SRAM (D1 domain) accessible by all system masters except BDMA
 *      through D1 domain AXI bus matrix
 *
 *      2.1) 1024Kb of SRAM beginning at address 0x2400:0000
 *
 *   3) AHB SRAM (D2 domain) accessible by all system masters except BDMA
 *      through D2 domain AHB bus matrix
 *
 *      3.1) 64Kb of SRAM1 beginning at address 0x3000:0000
 *      3.2) 64Kb of SRAM2 beginning at address 0x3001:0000
 *
 *      Note: SRAM3 is NOT present on STM32H7B3
 *
 *   4) AHB SRAM (D3 domain) accessible by most of system masters
 *      through D3 domain AHB bus matrix
 *
 *      4.1)  32Kb of SRAM4 beginning at address 0x3800:0000
 *      4.2)   4Kb of backup RAM beginning at address 0x3880:0000
 *
 * When booting from FLASH, FLASH memory is aliased to address 0x0000:0000
 * where the code expects to begin execution by jumping to the entry point in
 * the 0x0800:0000 address range.
 */

MEMORY
{
  itcm  (rwx) : ORIGIN = 0x00000000, LENGTH =   64K
  flash (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
  dtcm1 (rwx) : ORIGIN = 0x20000000, LENGTH =   64K
  dtcm2 (rwx) : ORIGIN = 0x20010000, LENGTH =   64K
  sram  (rwx) : ORIGIN = 0x24000000, LENGTH = 1024K
  sram1 (rwx) : ORIGIN = 0x30000000, LENGTH =   64K
  sram2 (rwx) : ORIGIN = 0x30010000, LENGTH =   64K
  sram4 (rwx) : ORIGIN = 0x38000000, LENGTH =   32K
  bbram (rwx) : ORIGIN = 0x38800000, LENGTH =    4K
}

OUTPUT_ARCH(arm)
EXTERN(_vectors)
ENTRY(_stext)
SECTIONS
{
    .text :
    {
        _stext = ABSOLUTE(.);
        *(.vectors)
        *(.text .text.*)
        *(.fixup)
        *(.gnu.warning)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.got)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        _etext = ABSOLUTE(.);
    } > flash

    .init_section :
    {
        _sinit = ABSOLUTE(.);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .ctors))
        _einit = ABSOLUTE(.);
    } > flash

    .ARM.extab :
    {
        *(.ARM.extab*)
    } > flash

    __exidx_start = ABSOLUTE(.);
    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > flash
    __exidx_end = ABSOLUTE(.);

    _eronly = ABSOLUTE(.);

    .data :
    {
        _sdata = ABSOLUTE(.);
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        CONSTRUCTORS
        . = ALIGN(4);
        _edata = ABSOLUTE(.);
    } > sram AT > flash

    .bss :
    {
        _sbss = ABSOLUTE(.);
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = ABSOLUTE(.);
    } > sram

    /* Emit the the D3 power domain section for locating BDMA data
     *
     * Static data with locate_data(".sram4") will be located
     * at start of SRAM4; the rest of SRAM4 will be added to the heap.
     */

    .sram4_reserve (NOLOAD) :
    {
        *(.sram4)
        . = ALIGN(4);
        _sram4_heap_start = ABSOLUTE(.);
    } > sram4

    /* Stabs debugging sections. */

    .stab 0 : { *(.stab) }
    .stabstr 0 : { *(.stabstr) }
    .stab.excl 0 : { *(.stab.excl) }
    .stab.exclstr 0 : { *(.stab.exclstr) }
    .stab.index 0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 : { *(.comment) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_info 0 : { *(.debug_info) }
    .debug_line 0 : { *(.debug_line) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_aranges 0 : { *(.debug_aranges) }
}
