#include <os.h>
#include <asm/processor.h>
#include <asm/ppc4xx-uic.h>

/* Returns 0 if exception not found and fixup otherwise.  */
extern uint32_t search_exception_table(uint32_t);

/* THIS NEEDS CHANGING to use the board info structure.
 */
#define END_OF_MEM    0x800000
#define UICB0_ALL    0

#define ESR_MCI 0x80000000
#define ESR_PIL 0x08000000
#define ESR_PPR 0x04000000
#define ESR_PTR 0x02000000
#define ESR_DST 0x00800000
#define ESR_DIZ 0x00400000
#define ESR_U0F 0x00008000

rt_inline void set_tsr(uint32_t val)
{
    mtspr(SPRN_TSR, val);
}

rt_inline uint32_t get_esr(void)
{
    uint32_t val;

    val = mfspr(SPRN_ESR);
    return val;
}

/*
 * Trap & Exception support
 */

void print_backtrace(uint32_t *sp)
{
    int cnt = 0;
    uint32_t i;

    printk("Call backtrace: ");
    while (sp) {
        if ((uint32_t)sp > END_OF_MEM)
            break;

        i = sp[1];
        if (cnt++ % 7 == 0)
            printk("\n");
        printk("%08lX ", i);
        if (cnt > 32) break;
        sp = (uint32_t *)*sp;
    }
    printk("\n");
}

void show_regs(struct pt_regs * regs)
{
    int i;

    printk("NIP: %08lX XER: %08lX LR: %08lX REGS: %p TRAP: %04lx DEAR: %08lX\n",
           regs->nip, regs->xer, regs->link, regs, regs->trap, regs->dar);
    printk("MSR: %08lx EE: %01x PR: %01x FP: %01x ME: %01x IR/DR: %01x%01x\n",
           regs->msr, regs->msr&MSR_EE ? 1 : 0, regs->msr&MSR_PR ? 1 : 0,
           regs->msr & MSR_FP ? 1 : 0,regs->msr&MSR_ME ? 1 : 0,
           regs->msr&MSR_IR ? 1 : 0,
           regs->msr&MSR_DR ? 1 : 0);

    printk("\n");
    for (i = 0;  i < 32;  i++) {
        if ((i % 8) == 0) {
            printk("GPR%02d: ", i);
        }

        printk("%08lX ", regs->gpr[i]);
        if ((i % 8) == 7) {
            printk("\n");
        }
    }
}

void panic(const char *fmt, ...)
{
    while (1);
}

void
_exception(int signr, struct pt_regs *regs)
{
    show_regs(regs);
    print_backtrace((uint32_t *)regs->gpr[1]);
    panic("Exception");
}

uint32_t
search_exception_table(uint32_t addr)
{
        uint32_t ret = 0;

        /* There is only the kernel to search.  */
        // ret = search_one_table(__start___ex_table, __stop___ex_table-1, addr);
        /* if the serial port does not hang in exception, printk can be used */
        if (ret) return ret;

        return 0;
}

/*
 * Handle external interrupts
 */
void external_interrupt(struct pt_regs *regs)
{
    u32 uic_msr;

    /*
     * Read masked interrupt status register to determine interrupt source
     */
    uic_msr = mfdcr(uic0msr);

    mtdcr(uic0sr, (uic_msr & UICB0_ALL));

    if (uic_msr & ~(UICB0_ALL)) {
        uic_interrupt(UIC0_DCR_BASE, 0);
    }

    return;
}

void MachineCheckException(struct pt_regs *regs)
{
    uint32_t fixup, val;

    if ((fixup = search_exception_table(regs->nip)) != 0) {
        regs->nip = fixup;
        val = mfspr(MCSR);
        /* Clear MCSR */
        mtspr(SPRN_MCSR, val);
        return;
    }

    printk("Machine Check Exception.\n");
    printk("Caused by (from msr): ");
    printk("regs %p ", regs);

    val = get_esr();

    if (val& ESR_IMCP) {
        printk("Instruction");
        mtspr(ESR, val & ~ESR_IMCP);
    } else {
        printk("Data");
    }
    printk(" machine check.\n");

    show_regs(regs);
    print_backtrace((uint32_t *)regs->gpr[1]);
    panic("machine check");
}

void AlignmentException(struct pt_regs *regs)
{
    show_regs(regs);
    print_backtrace((uint32_t *)regs->gpr[1]);
    panic("Alignment Exception");
}

void ProgramCheckException(struct pt_regs *regs)
{
    long esr_val;

    show_regs(regs);

    esr_val = get_esr();
    if (esr_val & ESR_PIL)
        printk( "** Illegal Instruction **\n" );
    else if (esr_val & ESR_PPR)
        printk( "** Privileged Instruction **\n" );
    else if (esr_val & ESR_PTR)
        printk( "** Trap Instruction **\n" );

    print_backtrace((uint32_t *)regs->gpr[1]);
    panic("Program Check Exception");
}

void DecrementerPITException(struct pt_regs *regs)
{
    /* reset PIT interrupt */
    set_tsr(0x08000000);

    /* increase a OS Tick */
    os_tick_increase();
}

void UnknownException(struct pt_regs *regs)
{

    printk("Bad trap at PC: %lx, SR: %lx, vector=%lx\n",
           regs->nip, regs->msr, regs->trap);
    _exception(0, regs);
}

void DebugException(struct pt_regs *regs)
{
    printk("Debugger trap at @ %lx @regs %lx\n", regs->nip, (uint32_t)regs);
    show_regs(regs);
}
