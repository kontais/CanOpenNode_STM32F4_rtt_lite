#ifndef __CACHE_H__
#define __CACHE_H__

#include <asm/processor.h>

#if !defined(__ASSEMBLY__)
void flush_dcache_range(uint32_t start, uint32_t stop);
void clean_dcache_range(uint32_t start, uint32_t stop);
void invalidate_dcache_range(uint32_t start, uint32_t stop);
void flush_dcache(void);
void invalidate_dcache(void);
void invalidate_icache(void);

void icache_enable(void);
void icache_disable(void);
uint32_t icache_status(void);

void dcache_enable(void);
void dcache_disable(void);
uint32_t dcache_status(void);
#endif

#endif
