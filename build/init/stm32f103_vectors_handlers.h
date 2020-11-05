/*
buck50: Test and measurement firmware for “Blue Pill” STM32F103 development board
Copyright (C) 2019,2020 Mark R. Rubin aka "thanks4opensource"

This file is part of buck50.

The buck50 program is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

The buck50 program is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
(LICENSE.txt) along with the buck50 program.  If not, see
<https://www.gnu.org/licenses/gpl.html>
*/


#ifndef STM32F103_VECTORS_HANDLERS_H
#define STM32F103_VECTORS_HANDLERS_H

#ifdef ALL_HANDLERS_DEFAULT
void    Default_Handler             (void);
#else
/* ARM interrupt handlers */
void    NMI_Handler                 (void),
        HardFault_Handler           (void),
        MemManage_Handler           (void),
        BusFault_Handler            (void),
        UsageFault_Handler          (void),
        Reserved1_Handler           (void),
        Reserved2_Handler           (void),
        Reserved3_Handler           (void),
        Reserved4_Handler           (void),
        SVCall_Handler              (void),
        Debug_Handler               (void),
        Reserved5_Handler           (void),
        PendSV_Handler              (void),
        SysTick_Handler             (void),
/* STM32f103-specific interrupt handlers */
        WWDG_IRQHandler             (void),   /*  0  Window WatchDog        */
        PVD_IRQHandler              (void),   /*  1  PVD through EXTI       */
        TAMPER_IRQHandler           (void),   /*  2  Tamper                 */
        RTC_IRQHandler              (void),   /*  2  Tamper                 */
        FLASH_IRQHandler            (void),   /*  4  FLASH global           */
        RCC_IRQHandler              (void),   /*  5  RCC global             */
        EXTI0_IRQHandler            (void),   /*  6  EXTI Line0             */
        EXTI1_IRQHandler            (void),   /*  7  EXTI Line1             */
        EXTI2_IRQHandler            (void),   /*  8  EXTI Line2             */
        EXTI3_IRQHandler            (void),   /*  9  EXTI Line3             */
        EXTI4_IRQHandler            (void),   /* 10  EXTI Line4             */
        DMA1_Channel1_IRQHandler    (void),   /* 11  DMA1 Channel 1 global  */
        DMA1_Channel2_IRQHandler    (void),   /* 12  DMA1 Channel 2         */
        DMA1_Channel3_IRQHandler    (void),   /* 13  DMA1 Channel 3         */
        DMA1_Channel4_IRQHandler    (void),   /* 14  DMA1 Channel 4         */
        DMA1_Channel5_IRQHandler    (void),   /* 15  DMA1 Channel 5         */
        DMA1_Channel6_IRQHandler    (void),   /* 16  DMA1 Channel 6         */
        DMA1_Channel7_IRQHandler    (void),   /* 17  DMA1 Channel 7         */
        ADC1_2_IRQHandler           (void),   /* 18  ADC1 and ADC2          */
        USB_HP_CAN1_TX_IRQHandle    (void),   /* 19  USB Device High        */
        USB_LP_CAN1_RX0_IRQHandler  (void),   /* 20  USB Device Low         */
        CAN1_RX1_IRQHandler         (void),   /* 21  CAN1 RX1               */
        CAN1_SCE_IRQHandler         (void),   /* 22  CAN1 SCE               */
        EXTI9_5_IRQHandler          (void),   /* 23  External Lines[9:5]    */
        TIM1_BRK_IRQHandler         (void),   /* 24  TIM1 Break             */
        TIM1_UP_IRQHandler          (void),   /* 25  TIM1 Update            */
        TIM1_TRG_COM_IRQHandler     (void),   /* 26  TIM1 Trigger and       */
        TIM1_CC_IRQHandler          (void),   /* 27  TIM1 Capture Compare   */
        TIM2_IRQHandler             (void),   /* 28  TIM2 global            */
        TIM3_IRQHandler             (void),   /* 29  TIM3 global            */
        TIM4_IRQHandler             (void),   /* 30  TIM4 global            */
        I2C1_EV_IRQHandler          (void),   /* 31  I2C1 Event             */
        I2C1_ER_IRQHandler          (void),   /* 32  I2C1 Error             */
        I2C2_EV_IRQHandler          (void),   /* 33  I2C2 Event             */
        I2C2_ER_IRQHandler          (void),   /* 34  I2C2 Error             */
        SPI1_IRQHandler             (void),   /* 35  SPI1 global            */
        SPI2_IRQHandler             (void),   /* 36  SPI2 global            */
        USART1_IRQHandler           (void),   /* 37  USART1 global          */
        USART2_IRQHandler           (void),   /* 38  USART2 global          */
        USART3_IRQHandler           (void),   /* 39  USART3 global          */
        EXTI15_10_IRQHandler        (void),   /* 40  External Line[15       */
        RTC_Alarm_IRQHandler        (void),   /* 41  RTC Alarm through      */
        USBWakeUp_IRQHandler        (void);   /* 42  USB Device WakeUp      */
#endif


INTERRUPT_VECTORS_CONST void *INTERRUPT_VECTORS[]
__attribute__((section(".vectors"))) = {
    (void *)&TOP_OF_STACK,  /* end of RAM, 0x20000000 + 20K == 0x20005000 */
    init,                   /* Reset Handler */
#ifdef ALL_HANDLERS_DEFAULT
/* ARM interrupt handlers */
    Default_Handler,    /* NMI */
    Default_Handler,    /* Hard Fault */
    Default_Handler,    /* MemManage */
    Default_Handler,    /* BusFault  */
    Default_Handler,    /* UsageFault */
    Default_Handler,    /* Reserved */
    Default_Handler,    /* Reserved */
    Default_Handler,    /* Reserved */
    Default_Handler,    /* Reserved */
    Default_Handler,    /* SVCall */
    Default_Handler,    /* Debug */
    Default_Handler,    /* Reserved */
    Default_Handler,    /* PendSV */
    Default_Handler,    /* SysTick */
/* STM32f103-specific interrupt handlers */
    Default_Handler,    /* WWDG_IRQHandler             0 Window WatchDog      */
    Default_Handler,    /* PVD_IRQHandler              1 PVD through EXTI     */
    Default_Handler,    /* TAMPER_IRQHandler           2 Tamper               */
    Default_Handler,    /* RTC_IRQHandler              2 Tamper               */
    Default_Handler,    /* FLASH_IRQHandler            4 FLASH global         */
    Default_Handler,    /* RCC_IRQHandler              5 RCC global           */
    Default_Handler,    /* EXTI0_IRQHandler            6 EXTI Line0           */
    Default_Handler,    /* EXTI1_IRQHandler            7 EXTI Line1           */
    Default_Handler,    /* EXTI2_IRQHandler            8 EXTI Line2           */
    Default_Handler,    /* EXTI3_IRQHandler            9 EXTI Line3           */
    Default_Handler,    /* EXTI4_IRQHandler           10 EXTI Line4           */
    Default_Handler,    /* DMA1_Channel1_IRQHandler   11 DMA1 Channel 1 global*/
    Default_Handler,    /* DMA1_Channel2_IRQHandler   12 DMA1 Channel 2       */
    Default_Handler,    /* DMA1_Channel3_IRQHandler   13 DMA1 Channel 3       */
    Default_Handler,    /* DMA1_Channel4_IRQHandler   14 DMA1 Channel 4       */
    Default_Handler,    /* DMA1_Channel5_IRQHandler   15 DMA1 Channel 5       */
    Default_Handler,    /* DMA1_Channel6_IRQHandler   16 DMA1 Channel 6       */
    Default_Handler,    /* DMA1_Channel7_IRQHandler   17 DMA1 Channel 7       */
    Default_Handler,    /* ADC1_2_IRQHandler          18 ADC1 and ADC2        */
    Default_Handler,    /* USB_HP_CAN1_TX_IRQHandle   19 USB Device High      */
    Default_Handler,    /* USB_LP_CAN1_RX0_IRQHandler 20 USB Device Low       */
    Default_Handler,    /* CAN1_RX1_IRQHandler        21 CAN1 RX1             */
    Default_Handler,    /* CAN1_SCE_IRQHandler        22 CAN1 SCE             */
    Default_Handler,    /* EXTI9_5_IRQHandler         23 External Lines[9:5]  */
    Default_Handler,    /* TIM1_BRK_IRQHandler        24 TIM1 Break           */
    Default_Handler,    /* TIM1_UP_IRQHandler         25 TIM1 Update          */
    Default_Handler,    /* TIM1_TRG_COM_IRQHandler    26 TIM1 Trigger and     */
    Default_Handler,    /* TIM1_CC_IRQHandler         27 TIM1 Capture Compare */
    Default_Handler,    /* TIM2_IRQHandler            28 TIM2 global          */
    Default_Handler,    /* TIM3_IRQHandler            29 TIM3 global          */
    Default_Handler,    /* TIM4_IRQHandler            30 TIM4 global          */
    Default_Handler,    /* I2C1_EV_IRQHandler         31 I2C1 Event           */
    Default_Handler,    /* I2C1_ER_IRQHandler         32 I2C1 Error           */
    Default_Handler,    /* I2C2_EV_IRQHandler         33 I2C2 Event           */
    Default_Handler,    /* I2C2_ER_IRQHandler         34 I2C2 Error           */
    Default_Handler,    /* SPI1_IRQHandler            35 SPI1 global          */
    Default_Handler,    /* SPI2_IRQHandler            36 SPI2 global          */
    Default_Handler,    /* USART1_IRQHandler          37 USART1 global        */
    Default_Handler,    /* USART2_IRQHandler          38 USART2 global        */
    Default_Handler,    /* USART3_IRQHandler          39 USART3 global        */
    Default_Handler,    /* EXTI15_10_IRQHandler       40 External Line[15     */
    Default_Handler,    /* RTC_Alarm_IRQHandler       41 RTC Alarm through    */
    Default_Handler,    /* USBWakeUp_IRQHandler       42 USB Device WakeUp    */
#else
/* ARM interrupt handlers */
    NMI_Handler,                    /* NMI */
    HardFault_Handler,              /* Hard Fault */
    MemManage_Handler,              /* MemManage */
    BusFault_Handler,               /* BusFault  */
    UsageFault_Handler,             /* UsageFault */
    Reserved1_Handler,              /* Reserved */
    Reserved2_Handler,              /* Reserved */
    Reserved3_Handler,              /* Reserved */
    Reserved4_Handler,              /* Reserved */
    SVCall_Handler,                 /* SVCall */
    Debug_Handler,                  /* Debug */
    Reserved5_Handler,              /* Reserved */
    PendSV_Handler,                 /* PendSV */
    SysTick_Handler,                /* SysTick */
/* STM32f103-specific interrupt handlers */
    WWDG_IRQHandler,                /*  0   Window WatchDog     */
    PVD_IRQHandler,                 /*  1   PVD through EXTI        */
    TAMPER_IRQHandler,              /*  2   Tamper                  */
    RTC_IRQHandler,                 /*  2   Tamper                  */
    FLASH_IRQHandler,               /*  4   FLASH global            */
    RCC_IRQHandler,                 /*  5   RCC global              */
    EXTI0_IRQHandler,               /*  6   EXTI Line0              */
    EXTI1_IRQHandler,               /*  7   EXTI Line1              */
    EXTI2_IRQHandler,               /*  8   EXTI Line2              */
    EXTI3_IRQHandler,               /*  9   EXTI Line3              */
    EXTI4_IRQHandler,               /* 10   EXTI Line4              */
    DMA1_Channel1_IRQHandler,       /* 11   DMA1 Channel 1 global   */
    DMA1_Channel2_IRQHandler,       /* 12   DMA1 Channel 2          */
    DMA1_Channel3_IRQHandler,       /* 13   DMA1 Channel 3          */
    DMA1_Channel4_IRQHandler,       /* 14   DMA1 Channel 4          */
    DMA1_Channel5_IRQHandler,       /* 15   DMA1 Channel 5          */
    DMA1_Channel6_IRQHandler,       /* 16   DMA1 Channel 6          */
    DMA1_Channel7_IRQHandler,       /* 17   DMA1 Channel 7          */
    ADC1_2_IRQHandler,              /* 18   ADC1 and ADC2           */
    USB_HP_CAN1_TX_IRQHandle,       /* 19   USB Device High         */
    USB_LP_CAN1_RX0_IRQHandler,     /* 20   USB Device Low          */
    CAN1_RX1_IRQHandler,            /* 21   CAN1 RX1                */
    CAN1_SCE_IRQHandler,            /* 22   CAN1 SCE                */
    EXTI9_5_IRQHandler,             /* 23   External Lines[9:5]     */
    TIM1_BRK_IRQHandler,            /* 24   TIM1 Break              */
    TIM1_UP_IRQHandler,             /* 25   TIM1 Update             */
    TIM1_TRG_COM_IRQHandler,        /* 26   TIM1 Trigger and        */
    TIM1_CC_IRQHandler,             /* 27   TIM1 Capture Compare    */
    TIM2_IRQHandler,                /* 28   TIM2 global             */
    TIM3_IRQHandler,                /* 29   TIM3 global             */
    TIM4_IRQHandler,                /* 30   TIM4 global             */
    I2C1_EV_IRQHandler,             /* 31   I2C1 Event              */
    I2C1_ER_IRQHandler,             /* 32   I2C1 Error              */
    I2C2_EV_IRQHandler,             /* 33   I2C2 Event              */
    I2C2_ER_IRQHandler,             /* 34   I2C2 Error              */
    SPI1_IRQHandler,                /* 35   SPI1 global             */
    SPI2_IRQHandler,                /* 36   SPI2 global             */
    USART1_IRQHandler,              /* 37   USART1 global           */
    USART2_IRQHandler,              /* 38   USART2 global           */
    USART3_IRQHandler,              /* 39   USART3 global           */
    EXTI15_10_IRQHandler,           /* 40   External Line[15        */
    RTC_Alarm_IRQHandler,           /* 41   RTC Alarm through       */
    USBWakeUp_IRQHandler,           /* 42   USB Device WakeUp       */
#endif
};



#ifdef WEAK_FUNCTIONS
#define MAYBE_WEAK __attribute__((weak))
#else
#define MAYBE_WEAK
#endif


#ifdef ALL_HANDLERS_DEFAULT
MAYBE_WEAK void Default_Handler()
{
    while(1);
}
#else
/* ARM interrupt handlers */
MAYBE_WEAK void NMI_Handler                 () { while (1); }
MAYBE_WEAK void HardFault_Handler           () { while (1); }
MAYBE_WEAK void MemManage_Handler           () { while (1); }
MAYBE_WEAK void BusFault_Handler            () { while (1); }
MAYBE_WEAK void UsageFault_Handler          () { while (1); }
MAYBE_WEAK void Reserved1_Handler           () { while (1); }
MAYBE_WEAK void Reserved2_Handler           () { while (1); }
MAYBE_WEAK void Reserved3_Handler           () { while (1); }
MAYBE_WEAK void Reserved4_Handler           () { while (1); }
MAYBE_WEAK void SVCall_Handler              () { while (1); }
MAYBE_WEAK void Debug_Handler               () { while (1); }
MAYBE_WEAK void Reserved5_Handler           () { while (1); }
MAYBE_WEAK void PendSV_Handler              () { while (1); }
MAYBE_WEAK void SysTick_Handler             () { while (1); }
/* STM32f103-specific interrupt handlers */
MAYBE_WEAK void WWDG_IRQHandler             () { while(1); }
MAYBE_WEAK void PVD_IRQHandler              () { while(1); }
MAYBE_WEAK void TAMPER_IRQHandler           () { while(1); }
MAYBE_WEAK void RTC_IRQHandler              () { while(1); }
MAYBE_WEAK void FLASH_IRQHandler            () { while(1); }
MAYBE_WEAK void RCC_IRQHandler              () { while(1); }
MAYBE_WEAK void EXTI0_IRQHandler            () { while(1); }
MAYBE_WEAK void EXTI1_IRQHandler            () { while(1); }
MAYBE_WEAK void EXTI2_IRQHandler            () { while(1); }
MAYBE_WEAK void EXTI3_IRQHandler            () { while(1); }
MAYBE_WEAK void EXTI4_IRQHandler            () { while(1); }
MAYBE_WEAK void DMA1_Channel1_IRQHandler    () { while(1); }
MAYBE_WEAK void DMA1_Channel2_IRQHandler    () { while(1); }
MAYBE_WEAK void DMA1_Channel3_IRQHandler    () { while(1); }
MAYBE_WEAK void DMA1_Channel4_IRQHandler    () { while(1); }
MAYBE_WEAK void DMA1_Channel5_IRQHandler    () { while(1); }
MAYBE_WEAK void DMA1_Channel6_IRQHandler    () { while(1); }
MAYBE_WEAK void DMA1_Channel7_IRQHandler    () { while(1); }
MAYBE_WEAK void ADC1_2_IRQHandler           () { while(1); }
MAYBE_WEAK void USB_HP_CAN1_TX_IRQHandle    () { while(1); }
MAYBE_WEAK void USB_LP_CAN1_RX0_IRQHandler  () { while(1); }
MAYBE_WEAK void CAN1_RX1_IRQHandler         () { while(1); }
MAYBE_WEAK void CAN1_SCE_IRQHandler         () { while(1); }
MAYBE_WEAK void EXTI9_5_IRQHandler          () { while(1); }
MAYBE_WEAK void TIM1_BRK_IRQHandler         () { while(1); }
MAYBE_WEAK void TIM1_UP_IRQHandler          () { while(1); }
MAYBE_WEAK void TIM1_TRG_COM_IRQHandler     () { while(1); }
MAYBE_WEAK void TIM1_CC_IRQHandler          () { while(1); }
MAYBE_WEAK void TIM2_IRQHandler             () { while(1); }
MAYBE_WEAK void TIM3_IRQHandler             () { while(1); }
MAYBE_WEAK void TIM4_IRQHandler             () { while(1); }
MAYBE_WEAK void I2C1_EV_IRQHandler          () { while(1); }
MAYBE_WEAK void I2C1_ER_IRQHandler          () { while(1); }
MAYBE_WEAK void I2C2_EV_IRQHandler          () { while(1); }
MAYBE_WEAK void I2C2_ER_IRQHandler          () { while(1); }
MAYBE_WEAK void SPI1_IRQHandler             () { while(1); }
MAYBE_WEAK void SPI2_IRQHandler             () { while(1); }
MAYBE_WEAK void USART1_IRQHandler           () { while(1); }
MAYBE_WEAK void USART2_IRQHandler           () { while(1); }
MAYBE_WEAK void USART3_IRQHandler           () { while(1); }
MAYBE_WEAK void EXTI15_10_IRQHandler        () { while(1); }
MAYBE_WEAK void RTC_Alarm_IRQHandler        () { while(1); }
MAYBE_WEAK void USBWakeUp_IRQHandler        () { while(1); }
#endif

#endif  // #ifndef  STM32F103_VECTORS_HANDLERS_H
