/*
 * Copyright (c) 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <reg.h>
#include <platform/iomap.h>
#include <mmu.h>
#include <arch/arm/mmu.h>
#include <dev/fbcon.h>
#include <kernel/thread.h>
#include <platform/debug.h>
#include <mddi.h>

static struct fbcon_config *fb_config;

void platform_init_interrupts(void);
void platform_init_timer(void);

void uart3_clock_init(void);
void uart_init(void);

struct fbcon_config *lcdc_init(void);

static uint32_t ticks_per_sec = 0;

#define MB (1024*1024)

#define MSM_IOMAP_SIZE ((MSM_IOMAP_END - MSM_IOMAP_BASE)/MB)

/* LK memory - cacheable, write through */
#define LK_MEMORY         (MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH | \
                           MMU_MEMORY_AP_READ_WRITE)

/* Peripherals - non-shared device */
#define IOMAP_MEMORY      (MMU_MEMORY_TYPE_DEVICE_SHARED | \
                           MMU_MEMORY_AP_READ_WRITE | MMU_MEMORY_XN)

/* Kernel region - cacheable, write through */
#define KERNEL_MEMORY     (MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH   | \
                           MMU_MEMORY_AP_READ_WRITE | MMU_MEMORY_XN)

/* Scratch region - cacheable, write through */
#define SCRATCH_MEMORY    (MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH   | \
                           MMU_MEMORY_AP_READ_WRITE | MMU_MEMORY_XN)

/* IMEM memory - cacheable, write through */
#define IMEM_MEMORY       (MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH | \
                           MMU_MEMORY_AP_READ_WRITE | MMU_MEMORY_XN)

static mmu_section_t mmu_section_table[] = {
/*  Physical addr,    Virtual addr,    Size (in MB),   Flags */
	{MEMBASE, MEMBASE, (MEMSIZE / MB), LK_MEMORY},
	{BASE_ADDR, BASE_ADDR, 44, KERNEL_MEMORY},
	{SCRATCH_ADDR, SCRATCH_ADDR, 384, SCRATCH_MEMORY},
	{MSM_IOMAP_BASE, MSM_IOMAP_BASE, MSM_IOMAP_SIZE, IOMAP_MEMORY},
	{MSM_IMEM_BASE, MSM_IMEM_BASE, 1, IMEM_MEMORY},
	{MSM_SHARED_BASE, MSM_SHARED_BASE, 1, KERNEL_MEMORY},
};

void platform_early_init(void)
{
        //uart3_clock_init();
	//uart_init();

	platform_init_interrupts();
	platform_init_timer();
}

void platform_init(void)
{
        dprintf(INFO, "platform_init()\n");
#if (!ENABLE_NANDWRITE)
        acpu_clock_init();
#endif
}

void display_init(void)
{
        struct fbcon_config *fb_cfg;

#if DISPLAY_TYPE_MDDI
	fb_config = mddi_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
#endif

#if DISPLAY_TYPE_LCDC
	fb_config = lcdc_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
#endif

}

void platform_init_timer(void)
{
	writel(0, DGT_ENABLE);

	ticks_per_sec = DGT_HZ;
}

/* Returns timer ticks per sec */
uint32_t platform_tick_rate(void)
{
	return ticks_per_sec;
}

int platform_use_identity_mmu_mappings(void)
{
	/* Use only the mappings specified in this file. */
	return 0;
}

addr_t platform_get_virt_to_phys_mapping(addr_t virt_addr)
{
	/* Return same address as we are using 1-1 mapping. */
	return virt_addr;
}

addr_t platform_get_phys_to_virt_mapping(addr_t phys_addr)
{
	/* Return same address as we are using 1-1 mapping. */
	return phys_addr;
}

/* Setup memory for this platform */
void platform_init_mmu_mappings(void)
{
	uint32_t i;
	uint32_t sections;
	uint32_t table_size = ARRAY_SIZE(mmu_section_table);
	for (i = 0; i < table_size; i++) {
		sections = mmu_section_table[i].num_of_sections;
		while (sections--) {
			arm_mmu_map_section(mmu_section_table[i].paddress +
					    sections * MB,
					    mmu_section_table[i].vaddress +
					    sections * MB,
					    mmu_section_table[i].flags);
		}
	}
}

int target_supports_qgic(void)
{
	return false;
}
