#include <string.h>
#include <stdio.h>

#include <board.h>
#include <os.h>

#include "at24c16.h"
#include "nvdata.h"

uint32_t WriteBuffer[512], ReadBuffer[512];

void at24c16_test(void)
{
    uint32_t cnt, i;

    cnt = 1;
    do {
        for(i = 0; i < 512; i++) {
            WriteBuffer[i] = cnt;
            ReadBuffer[i]  = ~cnt;
        }

//        at24c16_write(0, (uint8_t *)WriteBuffer, sizeof(WriteBuffer));
//        at24c16_read(0, (uint8_t *)ReadBuffer, sizeof(ReadBuffer));

        nvdata_write(0, (uint8_t *)WriteBuffer, sizeof(WriteBuffer));
        nvdata_read(0, (uint8_t *)ReadBuffer, sizeof(ReadBuffer));
        
        if(memcmp(WriteBuffer, ReadBuffer, sizeof(WriteBuffer)) == 0) {
            printf("EEPROM test OK %d\n", cnt);
        } else {
            printf("EEPROM test fail %d\n", cnt);
        }
        os_task_sleep(10);
    } while (cnt++);
}
