#include <os.h>
#include "pmu.h"

void os_arch_pmu_dump_feature(void)
{
    uint32_t reg;

    reg = os_arch_pmu_get_control();
    printk("ARM PMU Implementor: %c, ID code: %02x, %d counters\n",
               reg >> 24, (reg >> 16) & 0xff, (reg >> 11) & 0x1f);
    OS_ASSERT(ARM_PMU_CNTER_NR == ((reg >> 11) & 0x1f));
}
