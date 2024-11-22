/*
 * InterruptVects.c
 *
 *  Created on: Jun 10, 2024
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "InterruptHandlers.h"

typedef void (*fp) (void);

#define RVECT_SECT          __attribute__ ((section (".rvectors")))

const fp RelocatableVectors[] RVECT_SECT  = {
//;0x0000  BRK
    (fp)INT_Excep_BRK,
//;0x0004  Reserved
    (fp)0,
//;0x0008  Reserved
    (fp)0,
//;0x000C  Reserved
    (fp)0,
//;0x0010  Reserved
    (fp)0,
//;0x0014  Reserved
    (fp)0,
//;0x0018  Reserved
    (fp)0,
//;0x001C  Reserved
    (fp)0,
//;0x0020  Reserved
    (fp)0,
//;0x0024  Reserved
    (fp)0,
//;0x0028  Reserved
    (fp)0,
//;0x002C  Reserved
    (fp)0,
//;0x0030  Reserved
    (fp)0,
//;0x0034  Reserved
    (fp)0,
//;0x0038  Reserved
    (fp)0,
//;0x003C  Reserved
    (fp)0,
//;0x0040  BSC_BUSERR
    (fp)INT_Excep_BSC_BUSERR,
//;0x0044  Reserved
    (fp)0,
//;0x0048  Reserved
    (fp)0,
//;0x004C  Reserved
    (fp)0,
//;0x0050  Reserved
    (fp)0,
//;0x0054 FCUERR
    (fp)0,
//;0x0058  Reserved
    (fp)0,
//;0x005C  FRDYI
    (fp)0,
//;0x0060  Reserved
    (fp)0,
//;0x0064  Reserved
    (fp)0,
//;0x0068  Reserved
    (fp)0,
//;0x006C  ICU_SWINT
    (fp)INT_Excep_ICU_SWINT,
//;0x0070  CMT0_CMI0
    (fp)INT_Excep_CMT0_CMI0,
//;0x0074  CMT1_CMI1
    (fp)INT_Excep_CMT1_CMI1,
//;0x0078  Reserved
    (fp)0,
//;0x007C  Reserved
    (fp)0,
//;0x0080  CAC_FERRF
    (fp)INT_Excep_CAC_FERRF,
//;0x0084  CAC_MENDF
    (fp)INT_Excep_CAC_MENDF,
//;0x0088  CAC_OVFF
    (fp)INT_Excep_CAC_OVFF,
//;0x008C  Reserved
    (fp)0,
//;0x0090  USB0_D0FIFO0
    (fp)INT_Excep_USB0_D0FIFO0,
//;0x0094  USB0_D1FIFO0
    (fp)INT_Excep_USB0_D1FIFO0,
//;0x0098  USB0_USBI0
    (fp)INT_Excep_USB0_USBI0,
//;0x009C  Reserved
    (fp)0,
//;0x00A0  Reserved
    (fp)0,
//;0x00A4  Reserved
    (fp)0,
//;0x00A8  Reserved
    (fp)0,
//;0x00AC  Reserved
    (fp)0,
//;0x00B0  RSPI0_SPEI0
    (fp)INT_Excep_RSPI0_SPEI0,
//;0x00B4  RSPI0_SPRI0
    (fp)INT_Excep_RSPI0_SPRI0,
//;0x00B8  RSPI0_SPTI0
    (fp)INT_Excep_RSPI0_SPTI0,
//;0x00BC  RSPI0_SPII0
    (fp)INT_Excep_RSPI0_SPII0,
//;0x00C0  Reserved
    (fp)0,
//;0x00C4  Reserved
    (fp)0,
//;0x00C8  Reserved
    (fp)0,
//;0x00CC  Reserved
    (fp)0,
//;0x00D0  Reserved
    (fp)0,
//;0x00D4  Reserved
    (fp)0,
//;0x00D8  Reserved
    (fp)0,
//;0x00DC  Reserved
    (fp)0,
//;0x00E0  Reserved
    (fp)0,
//;0x00E4  DOC_DOPCF
    (fp)INT_Excep_DOC_DOPCF,
//;0x00E8  Reserved
    (fp)0,
//;0x00EC  Reserved
    (fp)0,
//;0x00F0  Reserved
    (fp)0,
//;0x00F4  Reserved
    (fp)0,
//;0x00F8  Reserved
    (fp)0,
//;0x00FC  Excep_RTC_CUP
    (fp)INT_Excep_RTC_CUP,
//;0x0100  IRQ0
    (fp)INT_Excep_ICU_IRQ0,
//;0x0104 IRQ1
    (fp)INT_Excep_ICU_IRQ1,
//;0x0108 IRQ2
    (fp)INT_Excep_ICU_IRQ2,
//;0x010C IRQ3
    (fp)INT_Excep_ICU_IRQ3,
//;0x0110 IRQ4
    (fp)INT_Excep_ICU_IRQ4,
//;0x0114 IRQ5
    (fp)INT_Excep_ICU_IRQ5,
//;0x0118 IRQ6
    (fp)INT_Excep_ICU_IRQ6,
//;0x011C IRQ7
    (fp)INT_Excep_ICU_IRQ7,
//;0x0120  Reserved
    (fp)0,
//;0x0124  Reserved
    (fp)0,
//;0x0128  Reserved
    (fp)0,
//;0x012C  Reserved
    (fp)0,
//;0x0130  Reserved
    (fp)0,
//;0x0134  Reserved
    (fp)0,
//;0x0138  Reserved
    (fp)0,
//;0x013C  Reserved
    (fp)0,
//;0x0140  Reserved
    (fp)0,
//;0x0144  Reserved
    (fp)0,
//;0x0148  Reserved
    (fp)0,
//;0x014C  Reserved
    (fp)0,
//;0x0150  Reserved
    (fp)0,
//;0x0154  Reserved
    (fp)0,
//;0x0158  Reserved
    (fp)0,
//;0x015C  Reserved
    (fp)0,
//;0x0160  LVD_LVD1
    (fp)INT_Excep_LVD_LVD1,
//;0x0164  LVD_LVD2
    (fp)INT_Excep_LVD_LVD2,
//;0x0168  USB0_USBR0
    (fp)INT_Excep_USB0_USBR0,
//;0x016C  Reserved
    (fp)0,
//;0x0170  RTC_ALM
    (fp)INT_Excep_RTC_ALM,
//;0x0174  RTC_PRD
    (fp)INT_Excep_RTC_PRD,
//;0x0178  Reserved
    (fp)0,
//;0x017C  Reserved
    (fp)0,
//;0x0180  Reserved
    (fp)0,
//;0x0184  Reserved
    (fp)0,
//;0x0188  Reserved
    (fp)0,
//;0x018C  Reserved
    (fp)0,
//;0x0190  Reserved
    (fp)0,
//;0x0194  Reserved
    (fp)0,
//;0x0198 S12AD_S12ADI0
    (fp)INT_Excep_S12AD_S12ADI0,
//;0x019C   S12AD_GBADI
    (fp)INT_Excep_S12AD_GBADI,
//104;0x01A0  Reserved
   (fp)0,
//105;0x01A4  Reserved
    (fp)0,
//;0x01A8  ELC_ELSR18I
    (fp)INT_Excep_ELC_ELSR18I,
//;0x01AC  Reserved
    (fp)0,
//;0x01B0  Reserved
    (fp)0,
//;0x01B4  Reserved
    (fp)0,
//;0x01B8  Reserved
    (fp)0,
//;0x01BC  Reserved
    (fp)0,
//;0x01C0  Reserved
    (fp)0,
//;0x01C4  Reserved
    (fp)0,
//;0x01C8  MTU0_TGIA0
    (fp)INT_Excep_MTU0_TGIA0,
//;0x01CC  MTU0_TGIB0
    (fp)INT_Excep_MTU0_TGIB0,
//;0x01D0  MTU0_TGIC0
    (fp)INT_Excep_MTU0_TGIC0,
//;0x01D4  MTU0_TGID0
    (fp)INT_Excep_MTU0_TGID0,
//;0x01D8  MTU0_TCIV0
    (fp)INT_Excep_MTU0_TCIV0,
//;0x01DC  MTU0_TGIE0
    (fp)INT_Excep_MTU0_TGIE0,
//;0x01E0  MTU0_TGIF0
    (fp)INT_Excep_MTU0_TGIF0,
//;0x01E4  MTU1_TGIA1
    (fp)INT_Excep_MTU1_TGIA1,
//;0x01E8  MTU1_TGIB1
    (fp)INT_Excep_MTU1_TGIB1,
//;0x01EC  MTU1_TCIV1
    (fp)INT_Excep_MTU1_TCIV1,
//;0x01F0  MTU1_TCIU1
    (fp)INT_Excep_MTU1_TCIU1,
//;0x01F4  MTU2_TGIA2
    (fp)INT_Excep_MTU2_TGIA2,
//;0x01F8  MTU2_TGIB2
    (fp)INT_Excep_MTU2_TGIB2,
//;0x01FC  MTU2_TCIV2
   (fp)INT_Excep_MTU2_TCIV2,
//;0x0200  MTU2_TCIU2
    (fp)INT_Excep_MTU2_TCIU2,
//;0x0204  MTU3_TGIA3
   (fp)INT_Excep_MTU3_TGIA3,
//;0x0208  MTU3_TGIB3
    (fp)INT_Excep_MTU3_TGIB3,
//;0x020C MTU3_TGIC3
    (fp)INT_Excep_MTU3_TGIC3,
//;0x0210 MTU3_TGID3
    (fp)INT_Excep_MTU3_TGID3,
//;0x0214  MTU3_TCIV3
    (fp)INT_Excep_MTU3_TCIV3,
//;0x0218  MTU4_TGIA4
    (fp)INT_Excep_MTU4_TGIA4,
//;0x021C  MTU4_TGIB4
    (fp)INT_Excep_MTU4_TGIB4,
//;0x0220  MTU4_TGIC4
    (fp)INT_Excep_MTU4_TGIC4,
//;0x0224  MTU4_TGID4
    (fp)INT_Excep_MTU4_TGID4,
//;0x0228  MTU4_TCIV4
    (fp)INT_Excep_MTU4_TCIV4,
//;0x022C  MTU5_TGIU5
    (fp)INT_Excep_MTU5_TGIU5,
//;0x0230  MTU5_TGIV5
    (fp)INT_Excep_MTU5_TGIV5,
//;0x0234  MTU5_TGIW5
    (fp)INT_Excep_MTU5_TGIW5,
//;0x0238  Reserved
    (fp)0,
//;0x023C  Reserved
    (fp)0,
//;0x0240  Reserved
    (fp)0,
//;0x0244  Reserved
    (fp)0,
//;0x0248  Reserved
    (fp)0,
//;0x024C  Reserved
    (fp)0,
//;0x0250  Reserved
    (fp)0,
//;0x0254  Reserved
    (fp)0,
//;0x0258  Reserved
    (fp)0,
//;0x025C  Reserved
    (fp)0,
//;0x0260  Reserved
    (fp)0,
//;0x0264  Reserved
    (fp)0,
//;0x0268  Reserved
    (fp)0,
//;0x026C  Reserved
    (fp)0,
//;0x0270  Reserved
    (fp)0,
//;0x0274  Reserved
    (fp)0,
//;0x0278  Reserved
    (fp)0,
//;0x027C  Reserved
    (fp)0,
//;0x0280  Reserved
    (fp)0,
//;0x0284  Reserved
    (fp)0,
//;0x0288  Reserved
    (fp)0,
//;0x028C  Reserved
    (fp)0,
//;0x0290  Reserved
    (fp)0,
//;0x0294  Reserved
    (fp)0,
//;0x0298  Reserved
    (fp)0,
//;0x029C  Reserved
    (fp)0,
//;0x02A0  Reserved
    (fp)0,
//;0x02A4  Reserved
    (fp)0,
//;0x02A8  POE_OEI1
    (fp)INT_Excep_POE_OEI1,
//;0x02AC  POE_OEI2
    (fp)INT_Excep_POE_OEI2,
//;0x02B0  Reserved
    (fp)0,
//;0x02B4  Reserved
    (fp)0,
//;0x02B8   Reserved
    (fp)0,
//;0x02BC   Reserved
    (fp)0,
//;0x02C0   Reserved
    (fp)0,
//;0x02C4  Reserved
    (fp)0,
//;0x02C8   Reserved
    (fp)0,
//;0x02CC   Reserved
    (fp)0,
//;0x02D0  Reserved
    (fp)0,
//;0x02D4   Reserved
    (fp)0,
//;0x02D8   Reserved
    (fp)0,
//;0x02DC   Reserved
    (fp)0,
//;0x02E0  Reserved
    (fp)0,
//;0x02E4   Reserved
    (fp)0,
//;0x02E8  Reserved
    (fp)0,
//;0x02EC  Reserved
    (fp)0,
//;0x02F0  Reserved
    (fp)0,
//;0x02F4  Reserved
    (fp)0,
//;0x02F8  Reserved
    (fp)0,
//;0x02FC  Reserved
    (fp)0,
//;0x0300  Reserved
    (fp)0,
//;0x0304  Reserved
    (fp)0,
//;0x0308  Reserved
    (fp)0,
//;0x030C  Reserved
    (fp)0,
//;0x0310  Reserved
    (fp)0,
//;0x0314  Reserved
    (fp)0,
//;0x0318  Reserved
    (fp)0,
//;0x031C  Reserved
    (fp)0,
//;0x0320  Reserved
    (fp)0,
//;0x0324  Reserved
    (fp)0,
//;0x0328  Reserved
    (fp)0,
//;0x032C  Reserved
    (fp)0,
//;0x0330  Reserved
    (fp)0,
//;0x0334  Reserved
    (fp)0,
//;0x0338  Reserved
    (fp)0,
//;0x033C  Reserved
    (fp)0,
//;0x0340  Reserved
    (fp)0,
//;0x0344  Reserved
    (fp)0,
//;0x0348  Reserved
    (fp)0,
//;0x034C  Reserved
    (fp)0,
//;0x0350  Reserved
    (fp)0,
//;0x0354  Reserved
    (fp)0,
//;0x0358  Reserved
    (fp)0,
//;0x035C  Reserved
    (fp)0,
//;0x0360  Reserved
    (fp)0,
//;0x0364  Reserved
    (fp)0,
//;0x0368  SCI1_ERI1
    (fp)INT_Excep_SCI1_ERI1,
//;0x036C  SCI1_RXI1
    (fp)INT_Excep_SCI1_RXI1,
//;0x0370  SCI1_TXI1
    (fp)INT_Excep_SCI1_TXI1,
//;0x0374  SCI1_TEI1
    (fp)INT_Excep_SCI1_TEI1,
//;0x0378  SCI5_ERI5
    (fp)INT_Excep_SCI5_ERI5,
//;0x037C  SCI5_RXI5
    (fp)INT_Excep_SCI5_RXI5,
//;0x0380  SCI5_TXI5
    (fp)INT_Excep_SCI5_TXI5,
//;0x0384  SCI5_TEI5
    (fp)INT_Excep_SCI5_TEI5,
//;0x0388  Reserved
    (fp)0,
//;0x038C Reserved
    (fp)0,
//;0x0390  Reserved
    (fp)0,
//;0x0394 Reserved
    (fp)0,
//;0x0398  Reserved
    (fp)0,
//;0x039C  Reserved
    (fp)0,
//;0x03A0  Reserved
    (fp)0,
//;0x03A4  Reserved
    (fp)0,
//;0x03A8  Reserved
    (fp)0,
//;0x03AC  Reserved
    (fp)0,
//;0x03B0  Reserved
    (fp)0,
//;0x03B4  Reserved
    (fp)0,
//;0x03B8  SCI12_ERI12
    (fp)INT_Excep_SCI12_ERI12,
//;0x03BC  SCI12_RXI12
    (fp)INT_Excep_SCI12_RXI12,
//;0x03C0  SCI12_TXI12
    (fp)INT_Excep_SCI12_TXI12,
//;0x03C4  SCI12_TEI12
    (fp)INT_Excep_SCI12_TEI12,
//;0x03C8  SCI12_SCIX0
    (fp)INT_Excep_SCI12_SCIX0,
//;0x03CC  SCI12_SCIX1
    (fp)INT_Excep_SCI12_SCIX1,
//;0x03D0  SCI12_SCIX2
    (fp)INT_Excep_SCI12_SCIX2,
//;0x03D4  SCI12_SCIX3
    (fp)INT_Excep_SCI12_SCIX3,
//;0x03D8  RIIC0_EEI0
    (fp)INT_Excep_RIIC0_EEI0,
//;0x03DC  RIIC0_RXI0
    (fp)INT_Excep_RIIC0_RXI0,
//;0x03E0  RIIC0_TXI0
    (fp)INT_Excep_RIIC0_TXI0,
//;0x03E4  RIIC0_TEI0
    (fp)INT_Excep_RIIC0_TEI0,
//;0x03E8  Reserved
    (fp)0,
//;0x03EC  Reserved
    (fp)0,
//;0x03F0  Reserved
    (fp)0,
//;0x03F4  Reserved
    (fp)0,
//;0x03F8  Reserved
    (fp)0,
//;0x03FC  Reserved
    (fp)0,
};



