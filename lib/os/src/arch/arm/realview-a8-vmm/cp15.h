#ifndef __CP15_H__
#define __CP15_H__

uint32_t rt_os_arch_get_smp_id(void);

void rt_os_arch_mmu_disable(void);
void rt_os_arch_mmu_enable(void);
void rt_os_arch_tlb_set(volatile uint32_t*);

void rt_os_arch_vector_set_base(unsigned int addr);

#endif
