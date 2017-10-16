/*
 * File      : rtc.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-04-26     yi.qiu           first version
 * 2010-03-18     Gary Lee    add functions such as GregorianDay
 *                                     and rtc_time_to_tm
 * 2009-03-20     yi.qiu           clean up
 */

#include <os.h>
#include <time.h>
#include <s3c24x0.h>

// #define RTC_DEBUG

#define RTC_ENABLE        RTCCON |=  0x01;    /*RTC read and write enable */
#define RTC_DISABLE        RTCCON &= ~0x01;    /* RTC read and write disable */
#define BCD2BIN(n)        (((((n) >> 4) & 0x0F) * 10) + ((n) & 0x0F))
#define BIN2BCD(n)        ((((n) / 10) << 4) | ((n) % 10))

/**
 * This function get rtc time
 */
void os_arch_rtc_get(struct tm *ti)
{
    uint8_t sec, min, hour, mday, wday, mon, year;

    /* enable access to RTC registers */
    RTCCON |= RTC_ENABLE;

    /* read RTC registers */
    do {
        sec     = BCDSEC;
        min     = BCDMIN;
        hour     = BCDHOUR;
        mday    = BCDDATE;
        wday     = BCDDAY;
        mon     = BCDMON;
        year     = BCDYEAR;
    } while (sec != BCDSEC);

#ifdef RTC_DEBUG
    printk("sec:%x min:%x hour:%x mday:%x wday:%x mon:%x year:%x\n",
        sec, min, hour, mday, wday, mon, year);
#endif

    /* disable access to RTC registers */
    RTC_DISABLE

    ti->tm_sec      = BCD2BIN(sec  & 0x7F);
    ti->tm_min      = BCD2BIN(min  & 0x7F);
    ti->tm_hour     = BCD2BIN(hour & 0x3F);
    ti->tm_mday     = BCD2BIN(mday & 0x3F);
    ti->tm_mon      = BCD2BIN(mon & 0x1F);
    ti->tm_year     = BCD2BIN(year);
    ti->tm_wday     = BCD2BIN(wday & 0x07);
    ti->tm_yday     = 0;
    ti->tm_isdst     = 0;
}

/**
 * This function set rtc time
 */
void os_arch_rtc_set(struct tm *ti)
{
    uint8_t sec, min, hour, mday, wday, mon, year;

    year    = BIN2BCD(ti->tm_year);
    mon     = BIN2BCD(ti->tm_mon);
    wday     = BIN2BCD(ti->tm_wday);
    mday     = BIN2BCD(ti->tm_mday);
    hour     = BIN2BCD(ti->tm_hour);
    min     = BIN2BCD(ti->tm_min);
    sec     = BIN2BCD(ti->tm_sec);

    /* enable access to RTC registers */
    RTC_ENABLE

    do{
        /* write RTC registers */
        BCDSEC         = sec;
        BCDMIN         = min;
        BCDHOUR     = hour;
        BCDDATE     = mday;
        BCDDAY         = wday;
        BCDMON     = mon;
        BCDYEAR     = year;
    }while (sec != BCDSEC);

    /* disable access to RTC registers */
    RTC_DISABLE
}

/**
 * This function reset rtc
 */
void os_arch_rtc_reset (void)
{
    RTCCON = (RTCCON & ~0x06) | 0x08;
    RTCCON &= ~(0x08|0x01);
}

static struct rt_device rtc;
static os_err_t rtc_open(device_t dev, uint16_t oflag)
{
    RTC_ENABLE
    return OS_OK;
}

static os_err_t rtc_close(device_t dev)
{
    RTC_DISABLE
    return OS_OK;
}

static size_t rtc_read(device_t dev, offset_t pos, void* buffer, size_t size)
{
    return OS_OK;
}

static os_err_t rtc_control(device_t dev, uint8_t cmd, void *args)
{
    struct tm tm, *tm_ptr;
    time_t *time;
    OS_ASSERT(dev != NULL);

    time = (time_t *)args;
    switch (cmd) {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        /* read device */
        os_arch_rtc_get(&tm);
        *((time_t *)args) = mktime(&tm);
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
        tm_ptr = localtime(time);
        /* write device */
        os_arch_rtc_set(tm_ptr);
        break;
    }

    return OS_OK;
}

void os_arch_rtc_init(void)
{
    rtc.type    = RT_Device_Class_RTC;

    /* register rtc device */
    rtc.init     = NULL;
    rtc.open     = rtc_open;
    rtc.close    = rtc_close;
    rtc.read     = rtc_read;
    rtc.write    = NULL;
    rtc.control = rtc_control;

    /* no private */
    rtc.user_data = NULL;

    rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void list_date()
{
    time_t time;
    device_t device;

    device = rt_device_find("rtc");
    if (device != NULL) {
        rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

        printk("%d, %s\n", time, ctime(&time));
    }
}
FINSH_FUNCTION_EXPORT(list_date, list date);
#endif
