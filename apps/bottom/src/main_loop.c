#include <board.h>

#include <os.h>
#include <tick.h>


int CO_loop(void);

os_tick_t tick_old = 0;
os_tick_t tick_use = 0;

void main_loop(void)
{
    os_tick_t current;

    while (1) {
        current  = os_tick_get();
        tick_use = current - tick_old;
        tick_old = current;
        
        CO_loop();
    }
}
