/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "project_config.h"

#include "targetdev.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;
volatile uint32_t counter_systick = 0;

/* If crystal-less is enabled, system won't use any crystal as clock source
   If using crystal-less, system will be 48MHz, otherwise, system is 72MHz
*/
// #define CRYSTAL_LESS        1
#define HIRC_AUTO_TRIM      0x611   /* Use USB signal to fine tune HIRC 48MHz */
#define TRIM_INIT           (SYS_BASE+0x110)


#define TIMEOUT_INTERVAL    				(5)   // sec
__IO uint32_t timeout_cnt = 0;
uint32_t u32TrimInit;
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
extern uint8_t bUsbDataReady;
void USBD_IRQHandler(void);


uint32_t get_systick(void)
{
	return (counter_systick);
}

void set_systick(uint32_t t)
{
	counter_systick = t;
}

void systick_counter(void)
{
	counter_systick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    #if 1
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}
    #else
    if (memcmp(src, des, nBytes))
    {
        printf("\nMismatch!! - %d\n", nBytes);
        for (i = 0; i < nBytes; i++)
            printf("0x%02x    0x%02x\n", src[i], des[i]);
        return -1;
    }
    #endif

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}


//
// check_reset_source
//
uint8_t check_reset_source(void)
{
    uint8_t tag;
    uint32_t src = SYS_GetResetSrc();

    SYS->RSTSTS |= 0x1FF;
    LDROM_DEBUG("Reset Source <0x%08X>\r\n", src);

    #if 1   //DEBUG , list reset source
    if (src & BIT0)
    {
        LDROM_DEBUG("0)POR Reset Flag\r\n");       
    }
    if (src & BIT1)
    {
        LDROM_DEBUG("1)NRESET Pin Reset Flag\r\n");       
    }
    if (src & BIT2)
    {
        LDROM_DEBUG("2)WDT Reset Flag\r\n");       
    }
    if (src & BIT3)
    {
        LDROM_DEBUG("3)LVR Reset Flag\r\n");       
    }
    if (src & BIT4)
    {
        LDROM_DEBUG("4)BOD Reset Flag\r\n");       
    }
    if (src & BIT5)
    {
        LDROM_DEBUG("5)MCU Reset Flag\r\n");       
    }
    if (src & BIT6)
    {
        LDROM_DEBUG("6)Reserved.\r\n");       
    }
    if (src & BIT7)
    {
        LDROM_DEBUG("7)CPU Reset Flag\r\n");       
    }
    if (src & BIT8)
    {
        LDROM_DEBUG("8)CPU Lockup Reset Flag\r\n");       
    }
    #endif

    tag = read_magic_tag();
  
	#if 0	// use button : PG15 to control update			  
	if (PG15 == FALSE)
	{
		write_magic_tag(0);
		LDROM_DEBUG("Enter BOOTLOADER w/ Button pressed\r\n");
		return TRUE;
	}
	#endif
	
    if (src & SYS_RSTSTS_PORF_Msk) 
    {
        SYS_ClearResetSrc(SYS_RSTSTS_PORF_Msk);
        
        if (tag == 0xA5) {
            write_magic_tag(0);
            LDROM_DEBUG("Enter BOOTLOADER from APPLICATION(POR)\r\n");
            return TRUE;
        } else if (tag == 0xBB) {
            write_magic_tag(0);
            LDROM_DEBUG("Upgrade finished...(POR)\r\n");
            return FALSE;
        } else {
            LDROM_DEBUG("Enter BOOTLOADER from POR\r\n");
            return FALSE;
        }
    } 
    else if (src & SYS_RSTSTS_CPURF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_CPURF_Msk);        
        if (tag == 0xA5) {
            write_magic_tag(0);
            LDROM_DEBUG("Enter BOOTLOADER from APPLICATION(CPU)\r\n");
            return TRUE;
        } else if (tag == 0xBB) {
            write_magic_tag(0);
            LDROM_DEBUG("Upgrade finished...(CPU)\r\n");
            return FALSE;
        } else {
            LDROM_DEBUG("Enter BOOTLOADER from CPU reset\r\n");
            return FALSE;
        }          
    }    
    else if 
    (src & SYS_RSTSTS_PINRF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_PINRF_Msk);
        LDROM_DEBUG("Enter BOOTLOADER from nRESET pin\r\n");
        return FALSE;
    }
    
    LDROM_DEBUG("Enter BOOTLOADER from unhandle reset source\r\n");
    return FALSE;
}


uint32_t caculate_crc32_checksum(uint32_t start, uint32_t size)
{
    volatile uint32_t addr, data;

    CRC_Open(CRC_32, (CRC_WDATA_RVS | CRC_CHECKSUM_RVS | CRC_CHECKSUM_COM), 0xFFFFFFFF, CRC_CPU_WDATA_32);
    
    for(addr = start; addr < size; addr += 4){
        data = FMC_Read(addr);
        CRC_WRITE_DATA(data);
    }
    return CRC_GetChecksum();
}

uint8_t verify_application_chksum(void)
{
    uint32_t chksum_cal, chksum_app;
    
    LDROM_DEBUG("Verify Checksum\r\n");
    
    chksum_cal = caculate_crc32_checksum(0x00000000, (g_apromSize - 4));//(g_apromSize - FMC_FLASH_PAGE_SIZE)
    LDROM_DEBUG("Caculated .....<0x%08X>\r\n", chksum_cal);
    
    chksum_app = FMC_Read(g_apromSize - 4);    
    LDROM_DEBUG("In APROM ......<0x%08X>\r\n", chksum_app);
    
    if (chksum_cal == chksum_app) {
        LDROM_DEBUG("Verify ........<PASS>\r\n");
        return TRUE;
    } else {
        LDROM_DEBUG("Verify ........<FAIL>\r\n");
        return FALSE;
    }
}

void BTN_Init(void)	//BTN1 
{
	// SYS->GPG_MFPH = (SYS->GPG_MFPH & ~(SYS_GPG_MFPH_PG15MFP_Msk)) | (SYS_GPG_MFPH_PG15MFP_GPIO);
	// GPIO_SetMode(PG,BIT15,GPIO_MODE_INPUT);
	
}

void USB_trim(void)
{
    /* Start USB trim if it is not enabled. */
    if((SYS->IRCTCTL & SYS_IRCTCTL_FREQSEL_Msk) != 1)
    {
        /* Start USB trim only when SOF */
        if(USBD->INTSTS & USBD_INTSTS_SOFIF_Msk)
        {
            /* Clear SOF */
            USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

            /* Re-enable crystal-less */
            SYS->IRCTCTL = HIRC_AUTO_TRIM | (8 << SYS_IRCTCTL_BOUNDARY_Pos);
        }
    }

    /* Disable USB Trim when error */
    if(SYS->IRCTISTS & (SYS_IRCTISTS_CLKERRIF_Msk | SYS_IRCTISTS_TFAILIF_Msk))
    {
        /* Init TRIM */
        M32(TRIM_INIT) = u32TrimInit;

        /* Disable crystal-less */
        SYS->IRCTCTL = 0;

        /* Clear error flags */
        SYS->IRCTISTS = SYS_IRCTISTS_CLKERRIF_Msk | SYS_IRCTISTS_TFAILIF_Msk;

        /* Clear SOF */
        USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;
    }		
    
}

void SysTick_Handler(void)
{

    systick_counter();

    if (get_systick() >= 0xFFFFFFFF)
    {
        set_systick(0);      
    }

    // if ((get_systick() % 1000) == 0)
    // {
       
    // }

    #if defined (ENABLE_TICK_EVENT)
    TickCheckTickEvent();
    #endif    
}

void SysTick_delay(unsigned long delay)
{  
    
    uint32_t tickstart = get_systick(); 
    uint32_t wait = delay; 

    while((get_systick() - tickstart) < wait) 
    { 
    } 

}

void SysTick_enable(int ticks_per_second)
{
    set_systick(0);
    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        printf("Set system tick error!!\n");
        while (1);
    }

    #if defined (ENABLE_TICK_EVENT)
    TickInitTickEvent();
    #endif
}

void TMR1_IRQHandler(void)
{
	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
        timeout_cnt++;

		// tick_counter();

		// if ((get_tick() % 1000) == 0)
		// {
        //     set_flag(flag_timer_period_1000ms ,ENABLE);
		// }

		// if ((get_tick() % 50) == 0)
		// {

		// }	
    }
}

void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    // TIMER_Start(TIMER1);
}

void loop(void)
{
	static uint32_t LOG1 = 0;
	// static uint32_t LOG2 = 0;

    if ((get_systick() % 1000) == 0)
    {
        // printf("%s(systick) : %4d\r\n",__FUNCTION__,LOG2++);    
    }

    if (is_flag_set(flag_timer_period_1000ms))
    {
        set_flag(flag_timer_period_1000ms ,DISABLE);

        printf("%s(timer) : %4d\r\n",__FUNCTION__,LOG1++);
        PB14 ^= 1;        
    }
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		switch(res)
		{
			case '1':
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
				NVIC_SystemReset();		
				break;
		}
	}
}

void UART0_IRQHandler(void)
{
    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
			UARTx_Process();
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	LDROM_DEBUG("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	LDROM_DEBUG("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	LDROM_DEBUG("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	LDROM_DEBUG("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	LDROM_DEBUG("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
	LDROM_DEBUG("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());		
	#endif	

}

void GPIO_Init (void)
{
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB14MFP_Msk)) | (SYS_GPB_MFPH_PB14MFP_GPIO);
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB15MFP_Msk)) | (SYS_GPB_MFPH_PB15MFP_GPIO);
	
    GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT15, GPIO_MODE_OUTPUT);		

}

void SYS_Init(void)
{
    SYS_UnlockReg();	

    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Set PF multi-function pins for XT1_OUT(PF.2) and XT1_IN(PF.3) */
//    SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF2MFP_Msk)) | SYS_GPF_MFPL_PF2MFP_XT1_OUT;
//    SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF3MFP_Msk)) | SYS_GPF_MFPL_PF3MFP_XT1_IN;

    /* Enable GPIO Port F module clock */
//    CLK_EnableModuleClock(GPIOF_MODULE);

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) as input mode to use HXT */
//    GPIO_SetMode(PF, BIT2|BIT3, GPIO_MODE_INPUT);


//    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);	

//    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);		
	
    /* Set core clock to 72MHz */
    // CLK_SetCoreClock(FREQ_72MHZ);

	/* Select HCLK clock source as HIRC and HCLK source divider as 1 */
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Select USB clock source as HIRC and USB clock divider as 1 */
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL3_USBDSEL_HIRC, CLK_CLKDIV0_USB(1));

    /* Enable USBD module clock */
    CLK_EnableModuleClock(USBD_MODULE);
	
    CLK_EnableModuleClock(ISP_MODULE);	    
	CLK_EnableModuleClock(CRC_MODULE);

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC_DIV2, CLK_CLKDIV0_UART0(1));

    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC_DIV2, 0);

    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

//    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~(UART0_RXD_PB12_Msk | UART0_TXD_PB13_Msk))) | (UART0_RXD_PB12 | UART0_TXD_PB13);

   /* Update System Core Clock */
    // SystemCoreClockUpdate();

    /* Lock protected registers */
    // SYS_LockReg();
}

int main()
{
    SYS_Init();
    
	UART0_Init();
	TIMER1_Init();
	BTN_Init();

    CLK->AHBCLK |= CLK_AHBCLK_ISPCKEN_Msk;
    //FMC->ISPCTL |= FMC_ISPCTL_ISPEN_Msk;

    // Enable FMC and APROM update
    //
    FMC_Open();
    FMC_ENABLE_AP_UPDATE();

    //
    // Get APROM and data flash information
    //
    g_apromSize = APROM_APPLICATION_SIZE;
    GetDataFlashInfo(&g_dataFlashAddr, &g_dataFlashSize);

    //
    // Stay in BOOTLOADER or jump to APPLICATION
    //
    #if 1
    if (!check_reset_source()) 
	{
        if (verify_application_chksum()) 
		{
            goto _APROM;
        } 
		else 
		{
            LDROM_DEBUG("Stay in BOOTLOADER (checksum invaild)\r\n");
        }
    } 
	else 
    {
        LDROM_DEBUG("Stay in BOOTLOADER (from APPLICATION)\r\n");
        
        //
        // start timer
        //
        LDROM_DEBUG("Time-out counter start....\r\n");
        TIMER_Start(TIMER1);
    }
    
    #else
    if((SYS->RSTSTS & SYS_RSTSTS_CPURF_Msk) == SYS_RSTSTS_CPURF_Msk) 
	{
        SYS->RSTSTS &= SYS->RSTSTS;

	    if (flag_start_ISP_process != 0x5A5A)	//(DetectPin != 0)
	    {
	        goto _APROM;
	    }
    }
    #endif

	/* Open USB controller */
	USBD_Open(&gsInfo, HID_ClassRequest, NULL);
	/*Init Endpoint configuration for HID */
	HID_Init();
	/* Start USB device */
	USBD_Start();

	/* Backup default trim */
	u32TrimInit = M32(TRIM_INIT);
	/* Clear SOF */
	USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

	/* DO NOT Enable USB device interrupt */
	NVIC_EnableIRQ(USBD_IRQn);

    while (1)
    {

		USB_trim();

		// polling USBD interrupt flag
		// USBD_IRQHandler();

        if (bUsbDataReady == TRUE)
        {
			ParseCmd((uint8_t *)usb_rcvbuf, 64);
			EP2_Handler();
			bUsbDataReady = FALSE;
			timeout_cnt = 0;
		}

        if (timeout_cnt > TIMEOUT_INTERVAL) {
            LDROM_DEBUG("Time-out, perform RESET\r\n");
            // while(!UART_IS_TX_EMPTY(UART0));
        
            // Reset chip to enter bootloader
            // SYS_UnlockReg();
            // SYS_ResetCPU(); //SYS_ResetChip();
            SystemReboot_RST(RST_ADDR_LDROM,RST_SEL_CHIP);
        }
    }

_APROM:
    LDROM_DEBUG("Jump to <APPLICATION>\r\n");
    while(!UART_IS_TX_EMPTY(UART0));

	#if 1    
    SystemReboot_RST(RST_ADDR_APROM,RST_SEL_CPU);

	#else

    SYS->RSTSTS = (SYS_RSTSTS_PORF_Msk | SYS_RSTSTS_PINRF_Msk);//clear bit
    FMC->ISPCTL &=  ~(FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_BS_Msk);
    SCB->AIRCR = (V6M_AIRCR_VECTKEY_DATA | V6M_AIRCR_SYSRESETREQ);
	#endif
	
    /* Trap the CPU */
    while (1);
}

/*** (C) COPYRIGHT 2021 Nuvoton Technology Corp. ***/
