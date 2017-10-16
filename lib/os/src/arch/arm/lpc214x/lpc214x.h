/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright KEIL ELEKTRONIK GmbH 2002-2005                           */
/***********************************************************************/
/*                                                                     */
/*  LPC214X.H:  Header file for Philips LPC2141/42/44/46/48            */
/*                                                                     */
/***********************************************************************/

#ifndef __LPC214x_H
#define __LPC214x_H

/* Vectored Interrupt Controller (VIC) */
#define VICIRQStatus    (*((volatile uint32_t *) 0xFFFFF000))
#define VICFIQStatus    (*((volatile uint32_t *) 0xFFFFF004))
#define VICRawIntr      (*((volatile uint32_t *) 0xFFFFF008))
#define VICIntSelect    (*((volatile uint32_t *) 0xFFFFF00C))
#define VICIntEnable    (*((volatile uint32_t *) 0xFFFFF010))
#define VICIntEnClr     (*((volatile uint32_t *) 0xFFFFF014))
#define VICSoftInt      (*((volatile uint32_t *) 0xFFFFF018))
#define VICSoftIntClr   (*((volatile uint32_t *) 0xFFFFF01C))
#define VICProtection   (*((volatile uint32_t *) 0xFFFFF020))
#define VICVectAddr     (*((volatile uint32_t *) 0xFFFFF030))
#define VICDefVectAddr  (*((volatile uint32_t *) 0xFFFFF034))
#define VICVectAddr0    (*((volatile uint32_t *) 0xFFFFF100))
#define VICVectAddr1    (*((volatile uint32_t *) 0xFFFFF104))
#define VICVectAddr2    (*((volatile uint32_t *) 0xFFFFF108))
#define VICVectAddr3    (*((volatile uint32_t *) 0xFFFFF10C))
#define VICVectAddr4    (*((volatile uint32_t *) 0xFFFFF110))
#define VICVectAddr5    (*((volatile uint32_t *) 0xFFFFF114))
#define VICVectAddr6    (*((volatile uint32_t *) 0xFFFFF118))
#define VICVectAddr7    (*((volatile uint32_t *) 0xFFFFF11C))
#define VICVectAddr8    (*((volatile uint32_t *) 0xFFFFF120))
#define VICVectAddr9    (*((volatile uint32_t *) 0xFFFFF124))
#define VICVectAddr10   (*((volatile uint32_t *) 0xFFFFF128))
#define VICVectAddr11   (*((volatile uint32_t *) 0xFFFFF12C))
#define VICVectAddr12   (*((volatile uint32_t *) 0xFFFFF130))
#define VICVectAddr13   (*((volatile uint32_t *) 0xFFFFF134))
#define VICVectAddr14   (*((volatile uint32_t *) 0xFFFFF138))
#define VICVectAddr15   (*((volatile uint32_t *) 0xFFFFF13C))
#define VICVectCntl0    (*((volatile uint32_t *) 0xFFFFF200))
#define VICVectCntl1    (*((volatile uint32_t *) 0xFFFFF204))
#define VICVectCntl2    (*((volatile uint32_t *) 0xFFFFF208))
#define VICVectCntl3    (*((volatile uint32_t *) 0xFFFFF20C))
#define VICVectCntl4    (*((volatile uint32_t *) 0xFFFFF210))
#define VICVectCntl5    (*((volatile uint32_t *) 0xFFFFF214))
#define VICVectCntl6    (*((volatile uint32_t *) 0xFFFFF218))
#define VICVectCntl7    (*((volatile uint32_t *) 0xFFFFF21C))
#define VICVectCntl8    (*((volatile uint32_t *) 0xFFFFF220))
#define VICVectCntl9    (*((volatile uint32_t *) 0xFFFFF224))
#define VICVectCntl10   (*((volatile uint32_t *) 0xFFFFF228))
#define VICVectCntl11   (*((volatile uint32_t *) 0xFFFFF22C))
#define VICVectCntl12   (*((volatile uint32_t *) 0xFFFFF230))
#define VICVectCntl13   (*((volatile uint32_t *) 0xFFFFF234))
#define VICVectCntl14   (*((volatile uint32_t *) 0xFFFFF238))
#define VICVectCntl15   (*((volatile uint32_t *) 0xFFFFF23C))

/* Pin Connect Block */
#define PINSEL0         (*((volatile uint32_t *) 0xE002C000))
#define PINSEL1         (*((volatile uint32_t *) 0xE002C004))
#define PINSEL2         (*((volatile uint32_t *) 0xE002C014))

/* General Purpose Input/Output (GPIO) */
#define IOPIN0          (*((volatile uint32_t *) 0xE0028000))
#define IOSET0          (*((volatile uint32_t *) 0xE0028004))
#define IODIR0          (*((volatile uint32_t *) 0xE0028008))
#define IOCLR0          (*((volatile uint32_t *) 0xE002800C))
#define IOPIN1          (*((volatile uint32_t *) 0xE0028010))
#define IOSET1          (*((volatile uint32_t *) 0xE0028014))
#define IODIR1          (*((volatile uint32_t *) 0xE0028018))
#define IOCLR1          (*((volatile uint32_t *) 0xE002801C))
#define IO0PIN          (*((volatile uint32_t *) 0xE0028000))
#define IO0SET          (*((volatile uint32_t *) 0xE0028004))
#define IO0DIR          (*((volatile uint32_t *) 0xE0028008))
#define IO0CLR          (*((volatile uint32_t *) 0xE002800C))
#define IO1PIN          (*((volatile uint32_t *) 0xE0028010))
#define IO1SET          (*((volatile uint32_t *) 0xE0028014))
#define IO1DIR          (*((volatile uint32_t *) 0xE0028018))
#define IO1CLR          (*((volatile uint32_t *) 0xE002801C))
#define FIO0DIR         (*((volatile uint32_t *) 0x3FFFC000))
#define FIO0MASK        (*((volatile uint32_t *) 0x3FFFC010))
#define FIO0PIN         (*((volatile uint32_t *) 0x3FFFC014))
#define FIO0SET         (*((volatile uint32_t *) 0x3FFFC018))
#define FIO0CLR         (*((volatile uint32_t *) 0x3FFFC01C))
#define FIO1DIR         (*((volatile uint32_t *) 0x3FFFC020))
#define FIO1MASK        (*((volatile uint32_t *) 0x3FFFC030))
#define FIO1PIN         (*((volatile uint32_t *) 0x3FFFC034))
#define FIO1SET         (*((volatile uint32_t *) 0x3FFFC038))
#define FIO1CLR         (*((volatile uint32_t *) 0x3FFFC03C))

/* Memory Accelerator Module (MAM) */
#define MAMCR           (*((volatile unsigned char *) 0xE01FC000))
#define MAMTIM          (*((volatile unsigned char *) 0xE01FC004))
#define MEMMAP          (*((volatile unsigned char *) 0xE01FC040))

/* Phase Locked Loop 0 (PLL0) */
#define PLL0CON         (*((volatile unsigned char *) 0xE01FC080))
#define PLL0CFG         (*((volatile unsigned char *) 0xE01FC084))
#define PLL0STAT        (*((volatile unsigned short*) 0xE01FC088))
#define PLL0FEED        (*((volatile unsigned char *) 0xE01FC08C))

/* Phase Locked Loop 1 (PLL1) */
#define PLL1CON         (*((volatile unsigned char *) 0xE01FC0A0))
#define PLL1CFG         (*((volatile unsigned char *) 0xE01FC0A4))
#define PLL1STAT        (*((volatile unsigned short*) 0xE01FC0A8))
#define PLL1FEED        (*((volatile unsigned char *) 0xE01FC0AC))

/* VPB Divider */
#define VPBDIV          (*((volatile unsigned char *) 0xE01FC100))

/* Power Control */
#define PCON            (*((volatile unsigned char *) 0xE01FC0C0))
#define PCONP           (*((volatile uint32_t *) 0xE01FC0C4))

/* External Interrupts */
#define EXTINT          (*((volatile unsigned char *) 0xE01FC140))
#define INTWAKE         (*((volatile unsigned short*) 0xE01FC144))
#define EXTMODE         (*((volatile unsigned char *) 0xE01FC148))
#define EXTPOLAR        (*((volatile unsigned char *) 0xE01FC14C))

/* Reset */
#define RSID            (*((volatile unsigned char *) 0xE01FC180))

/* Code Security / Debugging */
#define CSPR            (*((volatile unsigned char *) 0xE01FC184))

/* System Control Miscellaneous */
#define SCS             (*((volatile uint32_t *) 0xE01FC1A0))

/* Timer 0 */
#define T0IR            (*((volatile uint32_t *) 0xE0004000))
#define T0TCR           (*((volatile uint32_t *) 0xE0004004))
#define T0TC            (*((volatile uint32_t *) 0xE0004008))
#define T0PR            (*((volatile uint32_t *) 0xE000400C))
#define T0PC            (*((volatile uint32_t *) 0xE0004010))
#define T0MCR           (*((volatile uint32_t *) 0xE0004014))
#define T0MR0           (*((volatile uint32_t *) 0xE0004018))
#define T0MR1           (*((volatile uint32_t *) 0xE000401C))
#define T0MR2           (*((volatile uint32_t *) 0xE0004020))
#define T0MR3           (*((volatile uint32_t *) 0xE0004024))
#define T0CCR           (*((volatile uint32_t *) 0xE0004028))
#define T0CR0           (*((volatile uint32_t *) 0xE000402C))
#define T0CR1           (*((volatile uint32_t *) 0xE0004030))
#define T0CR2           (*((volatile uint32_t *) 0xE0004034))
#define T0CR3           (*((volatile uint32_t *) 0xE0004038))
#define T0EMR           (*((volatile uint32_t *) 0xE000403C))
#define T0CTCR          (*((volatile uint32_t *) 0xE0004070))

/* Timer 1 */
#define T1IR            (*((volatile uint32_t *) 0xE0008000))
#define T1TCR           (*((volatile uint32_t *) 0xE0008004))
#define T1TC            (*((volatile uint32_t *) 0xE0008008))
#define T1PR            (*((volatile uint32_t *) 0xE000800C))
#define T1PC            (*((volatile uint32_t *) 0xE0008010))
#define T1MCR           (*((volatile uint32_t *) 0xE0008014))
#define T1MR0           (*((volatile uint32_t *) 0xE0008018))
#define T1MR1           (*((volatile uint32_t *) 0xE000801C))
#define T1MR2           (*((volatile uint32_t *) 0xE0008020))
#define T1MR3           (*((volatile uint32_t *) 0xE0008024))
#define T1CCR           (*((volatile uint32_t *) 0xE0008028))
#define T1CR0           (*((volatile uint32_t *) 0xE000802C))
#define T1CR1           (*((volatile uint32_t *) 0xE0008030))
#define T1CR2           (*((volatile uint32_t *) 0xE0008034))
#define T1CR3           (*((volatile uint32_t *) 0xE0008038))
#define T1EMR           (*((volatile uint32_t *) 0xE000803C))
#define T1CTCR          (*((volatile uint32_t *) 0xE0008070))

/* Pulse Width Modulator (PWM) */
#define PWMIR           (*((volatile uint32_t *) 0xE0014000))
#define PWMTCR          (*((volatile uint32_t *) 0xE0014004))
#define PWMTC           (*((volatile uint32_t *) 0xE0014008))
#define PWMPR           (*((volatile uint32_t *) 0xE001400C))
#define PWMPC           (*((volatile uint32_t *) 0xE0014010))
#define PWMMCR          (*((volatile uint32_t *) 0xE0014014))
#define PWMMR0          (*((volatile uint32_t *) 0xE0014018))
#define PWMMR1          (*((volatile uint32_t *) 0xE001401C))
#define PWMMR2          (*((volatile uint32_t *) 0xE0014020))
#define PWMMR3          (*((volatile uint32_t *) 0xE0014024))
#define PWMMR4          (*((volatile uint32_t *) 0xE0014040))
#define PWMMR5          (*((volatile uint32_t *) 0xE0014044))
#define PWMMR6          (*((volatile uint32_t *) 0xE0014048))
#define PWMPCR          (*((volatile uint32_t *) 0xE001404C))
#define PWMLER          (*((volatile uint32_t *) 0xE0014050))

/* Universal Asynchronous Receiver Transmitter 0 (UART0) */
#define U0RBR           (*((volatile unsigned char *) 0xE000C000))
#define U0THR           (*((volatile unsigned char *) 0xE000C000))
#define U0IER           (*((volatile uint32_t *) 0xE000C004))
#define U0IIR           (*((volatile uint32_t *) 0xE000C008))
#define U0FCR           (*((volatile unsigned char *) 0xE000C008))
#define U0LCR           (*((volatile unsigned char *) 0xE000C00C))
#define U0MCR           (*((volatile unsigned char *) 0xE000C010))
#define U0LSR           (*((volatile unsigned char *) 0xE000C014))
#define U0MSR           (*((volatile unsigned char *) 0xE000C018))
#define U0SCR           (*((volatile unsigned char *) 0xE000C01C))
#define U0DLL           (*((volatile unsigned char *) 0xE000C000))
#define U0DLM           (*((volatile unsigned char *) 0xE000C004))
#define U0ACR           (*((volatile uint32_t *) 0xE000C020))
#define U0FDR           (*((volatile uint32_t *) 0xE000C028))
#define U0TER           (*((volatile unsigned char *) 0xE000C030))

/* Universal Asynchronous Receiver Transmitter 1 (UART1) */
#define U1RBR           (*((volatile unsigned char *) 0xE0010000))
#define U1THR           (*((volatile unsigned char *) 0xE0010000))
#define U1IER           (*((volatile uint32_t *) 0xE0010004))
#define U1IIR           (*((volatile uint32_t *) 0xE0010008))
#define U1FCR           (*((volatile unsigned char *) 0xE0010008))
#define U1LCR           (*((volatile unsigned char *) 0xE001000C))
#define U1MCR           (*((volatile unsigned char *) 0xE0010010))
#define U1LSR           (*((volatile unsigned char *) 0xE0010014))
#define U1MSR           (*((volatile unsigned char *) 0xE0010018))
#define U1SCR           (*((volatile unsigned char *) 0xE001001C))
#define U1DLL           (*((volatile unsigned char *) 0xE0010000))
#define U1DLM           (*((volatile unsigned char *) 0xE0010004))
#define U1ACR           (*((volatile uint32_t *) 0xE0010020))
#define U1FDR           (*((volatile uint32_t *) 0xE0010028))
#define U1TER           (*((volatile unsigned char *) 0xE0010030))

/* I2C Interface 0 */
#define I2C0CONSET      (*((volatile unsigned char *) 0xE001C000))
#define I2C0STAT        (*((volatile unsigned char *) 0xE001C004))
#define I2C0DAT         (*((volatile unsigned char *) 0xE001C008))
#define I2C0ADR         (*((volatile unsigned char *) 0xE001C00C))
#define I2C0SCLH        (*((volatile unsigned short*) 0xE001C010))
#define I2C0SCLL        (*((volatile unsigned short*) 0xE001C014))
#define I2C0CONCLR      (*((volatile unsigned char *) 0xE001C018))

/* I2C Interface 1 */
#define I2C1CONSET      (*((volatile unsigned char *) 0xE005C000))
#define I2C1STAT        (*((volatile unsigned char *) 0xE005C004))
#define I2C1DAT         (*((volatile unsigned char *) 0xE005C008))
#define I2C1ADR         (*((volatile unsigned char *) 0xE005C00C))
#define I2C1SCLH        (*((volatile unsigned short*) 0xE005C010))
#define I2C1SCLL        (*((volatile unsigned short*) 0xE005C014))
#define I2C1CONCLR      (*((volatile unsigned char *) 0xE005C018))

/* SPI0 (Serial Peripheral Interface 0) */
#define S0SPCR          (*((volatile unsigned short*) 0xE0020000))
#define S0SPSR          (*((volatile unsigned char *) 0xE0020004))
#define S0SPDR          (*((volatile unsigned short*) 0xE0020008))
#define S0SPCCR         (*((volatile unsigned char *) 0xE002000C))
#define S0SPINT         (*((volatile unsigned char *) 0xE002001C))

/* SSP Controller (SPI1) */
#define SSPCR0          (*((volatile unsigned short*) 0xE0068000))
#define SSPCR1          (*((volatile unsigned char *) 0xE0068004))
#define SSPDR           (*((volatile unsigned short*) 0xE0068008))
#define SSPSR           (*((volatile unsigned char *) 0xE006800C))
#define SSPCPSR         (*((volatile unsigned char *) 0xE0068010))
#define SSPIMSC         (*((volatile unsigned char *) 0xE0068014))
#define SSPRIS          (*((volatile unsigned char *) 0xE0068018))
#define SSPMIS          (*((volatile unsigned char *) 0xE006801C))
#define SSPICR          (*((volatile unsigned char *) 0xE0068020))

/* Real Time Clock */
#define ILR             (*((volatile unsigned char *) 0xE0024000))
#define CTC             (*((volatile unsigned short*) 0xE0024004))
#define CCR             (*((volatile unsigned char *) 0xE0024008))
#define CIIR            (*((volatile unsigned char *) 0xE002400C))
#define AMR             (*((volatile unsigned char *) 0xE0024010))
#define CTIME0          (*((volatile uint32_t *) 0xE0024014))
#define CTIME1          (*((volatile uint32_t *) 0xE0024018))
#define CTIME2          (*((volatile uint32_t *) 0xE002401C))
#define SEC             (*((volatile unsigned char *) 0xE0024020))
#define MIN             (*((volatile unsigned char *) 0xE0024024))
#define HOUR            (*((volatile unsigned char *) 0xE0024028))
#define DOM             (*((volatile unsigned char *) 0xE002402C))
#define DOW             (*((volatile unsigned char *) 0xE0024030))
#define DOY             (*((volatile unsigned short*) 0xE0024034))
#define MONTH           (*((volatile unsigned char *) 0xE0024038))
#define YEAR            (*((volatile unsigned short*) 0xE002403C))
#define ALSEC           (*((volatile unsigned char *) 0xE0024060))
#define ALMIN           (*((volatile unsigned char *) 0xE0024064))
#define ALHOUR          (*((volatile unsigned char *) 0xE0024068))
#define ALDOM           (*((volatile unsigned char *) 0xE002406C))
#define ALDOW           (*((volatile unsigned char *) 0xE0024070))
#define ALDOY           (*((volatile unsigned short*) 0xE0024074))
#define ALMON           (*((volatile unsigned char *) 0xE0024078))
#define ALYEAR          (*((volatile unsigned short*) 0xE002407C))
#define PREINT          (*((volatile unsigned short*) 0xE0024080))
#define PREFRAC         (*((volatile unsigned short*) 0xE0024084))

/* A/D Converter 0 (AD0) */
#define AD0CR           (*((volatile uint32_t *) 0xE0034000))
#define AD0GDR          (*((volatile uint32_t *) 0xE0034004))
#define AD0STAT         (*((volatile uint32_t *) 0xE0034030))
#define AD0INTEN        (*((volatile uint32_t *) 0xE003400C))
#define AD0DR0          (*((volatile uint32_t *) 0xE0034010))
#define AD0DR1          (*((volatile uint32_t *) 0xE0034014))
#define AD0DR2          (*((volatile uint32_t *) 0xE0034018))
#define AD0DR3          (*((volatile uint32_t *) 0xE003401C))
#define AD0DR4          (*((volatile uint32_t *) 0xE0034020))
#define AD0DR5          (*((volatile uint32_t *) 0xE0034024))
#define AD0DR6          (*((volatile uint32_t *) 0xE0034028))
#define AD0DR7          (*((volatile uint32_t *) 0xE003402C))

/* A/D Converter 1 (AD1) */
#define AD1CR           (*((volatile uint32_t *) 0xE0060000))
#define AD1GDR          (*((volatile uint32_t *) 0xE0060004))
#define AD1STAT         (*((volatile uint32_t *) 0xE0060030))
#define AD1INTEN        (*((volatile uint32_t *) 0xE006000C))
#define AD1DR0          (*((volatile uint32_t *) 0xE0060010))
#define AD1DR1          (*((volatile uint32_t *) 0xE0060014))
#define AD1DR2          (*((volatile uint32_t *) 0xE0060018))
#define AD1DR3          (*((volatile uint32_t *) 0xE006001C))
#define AD1DR4          (*((volatile uint32_t *) 0xE0060020))
#define AD1DR5          (*((volatile uint32_t *) 0xE0060024))
#define AD1DR6          (*((volatile uint32_t *) 0xE0060028))
#define AD1DR7          (*((volatile uint32_t *) 0xE006002C))

/* A/D Converter Global */
#define ADGSR           (*((volatile uint32_t *) 0xE0034008))

/* D/A Converter */
#define DACR            (*((volatile uint32_t *) 0xE006C000))

/* Watchdog */
#define WDMOD           (*((volatile unsigned char *) 0xE0000000))
#define WDTC            (*((volatile uint32_t *) 0xE0000004))
#define WDFEED          (*((volatile unsigned char *) 0xE0000008))
#define WDTV            (*((volatile uint32_t *) 0xE000000C))

/* USB Controller */
#define USBIntSt        (*((volatile uint32_t *) 0xE01FC1C0))
#define USBDevIntSt     (*((volatile uint32_t *) 0xE0090000))
#define USBDevIntEn     (*((volatile uint32_t *) 0xE0090004))
#define USBDevIntClr    (*((volatile uint32_t *) 0xE0090008))
#define USBDevIntSet    (*((volatile uint32_t *) 0xE009000C))
#define USBDevIntPri    (*((volatile unsigned char *) 0xE009002C))
#define USBEpIntSt      (*((volatile uint32_t *) 0xE0090030))
#define USBEpIntEn      (*((volatile uint32_t *) 0xE0090034))
#define USBEpIntClr     (*((volatile uint32_t *) 0xE0090038))
#define USBEpIntSet     (*((volatile uint32_t *) 0xE009003C))
#define USBEpIntPri     (*((volatile uint32_t *) 0xE0090040))
#define USBReEp         (*((volatile uint32_t *) 0xE0090044))
#define USBEpInd        (*((volatile uint32_t *) 0xE0090048))
#define USBMaxPSize     (*((volatile uint32_t *) 0xE009004C))
#define USBRxData       (*((volatile uint32_t *) 0xE0090018))
#define USBRxPLen       (*((volatile uint32_t *) 0xE0090020))
#define USBTxData       (*((volatile uint32_t *) 0xE009001C))
#define USBTxPLen       (*((volatile uint32_t *) 0xE0090024))
#define USBCtrl         (*((volatile uint32_t *) 0xE0090028))
#define USBCmdCode      (*((volatile uint32_t *) 0xE0090010))
#define USBCmdData      (*((volatile uint32_t *) 0xE0090014))
#define USBDMARSt       (*((volatile uint32_t *) 0xE0090050))
#define USBDMARClr      (*((volatile uint32_t *) 0xE0090054))
#define USBDMARSet      (*((volatile uint32_t *) 0xE0090058))
#define USBUDCAH        (*((volatile uint32_t *) 0xE0090080))
#define USBEpDMASt      (*((volatile uint32_t *) 0xE0090084))
#define USBEpDMAEn      (*((volatile uint32_t *) 0xE0090088))
#define USBEpDMADis     (*((volatile uint32_t *) 0xE009008C))
#define USBDMAIntSt     (*((volatile uint32_t *) 0xE0090090))
#define USBDMAIntEn     (*((volatile uint32_t *) 0xE0090094))
#define USBEoTIntSt     (*((volatile uint32_t *) 0xE00900A0))
#define USBEoTIntClr    (*((volatile uint32_t *) 0xE00900A4))
#define USBEoTIntSet    (*((volatile uint32_t *) 0xE00900A8))
#define USBNDDRIntSt    (*((volatile uint32_t *) 0xE00900AC))
#define USBNDDRIntClr   (*((volatile uint32_t *) 0xE00900B0))
#define USBNDDRIntSet   (*((volatile uint32_t *) 0xE00900B4))
#define USBSysErrIntSt  (*((volatile uint32_t *) 0xE00900B8))
#define USBSysErrIntClr (*((volatile uint32_t *) 0xE00900BC))
#define USBSysErrIntSet (*((volatile uint32_t *) 0xE00900C0))

#define VIC_BASE_ADDR    0xFFFFF000

enum LPC214x_INT
{
    WDT_INT    = 0,
    SW_INT_reserved,
    DbgCommRx_INT,
    DbgCommTx_INT,
    TIMER0_INT,
    TIMER1_INT,
    UART0_INT,
    UART1_INT,
    PWM0_INT,
    I2C0_INT,
    SP0_INT,
    SP1_INT,
    PLL_INT,
    RTC_INT,
    EINT0_INT,
    EINT1_INT,
    EINT2_INT,
    EINT3_INT,
    ADC0_INT,
    I2C1_INT,
    BOD_INT,
    ADC1_INT,
    USB_INT
};

#endif  // __LPC214x_H
