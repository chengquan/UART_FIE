#include "FIE_uart.h"
#include "mnist_int8.h"
#include "basic.h"

char FIE_frame[15];

uint8_t mode;
uint32_t addr;
uint32_t len;
uint8_t flip_pos;
//Fault Injection Env   CMD define
//byte      |      1-2     |   3    |  4-7  |  8-11  |    12    |    13-14    |
//function  | Begin of cmd | w/R/M  |  Addr |  Len   | Flip-pos |  End of cmd |
//content   |  A5    5A    | O 1 2  |       |        |          |   \r\n      |
uint8_t FIE_cmd_sts; 

void muart0_irq_handler(void)
{
    //int0_cnt++;
    //uint8_t reg;
    uint8_t len;
    if(FIE_cmd_sts == FIE_CMD_EMPTY)
    {
        for(len=0;len< FIE_FRAME_LEN ;len++)
        {
            FIE_frame[len] = uart_read(UART0);
            //printf("len%d:%d\r\n",len,FIE_frame[len]);
            //printf("333:%x",UART0->IER);
            //printf("444:%x",UART0->IIR);
        }
        printf("cmd:%x %x %x %x %x %x %x %x %x %x %x %x %x %x.\r\n", \
                            FIE_frame[0],FIE_frame[1],FIE_frame[2],FIE_frame[3], \
                            FIE_frame[4],FIE_frame[5],FIE_frame[6],FIE_frame[7], \
                            FIE_frame[8],FIE_frame[9],FIE_frame[10],FIE_frame[11], \
                            FIE_frame[12],FIE_frame[13]);
        FIE_cmd_sts = FIE_CMD_RDY;
        PLIC_DisableInterrupt(PLIC_UART0_IRQn);
    }
    //printf("444:%x",UART0->FCR);
}

void FIE_cmd_init()
{
    uint8_t len;
    FIE_cmd_sts = FIE_CMD_EMPTY;
    for(len=0;len< FIE_FRAME_LEN+1 ;len++)
    {
        FIE_frame[len] = 0;
    }
}

void FIE_cmd_fsm()
{
    if(FIE_cmd_sts == FIE_CMD_RDY)
    {
        FIE_cmd_check_and_decode();
    } else if(FIE_cmd_sts == FIE_CMD_CHECK)
    {
        FIE_cmd_exe();
        FIE_cmd_sts = FIE_CMD_FINISH;
    }
    else if(FIE_cmd_sts == FIE_CMD_FINISH)
    {
        FIE_cmd_init();
        PLIC_EnableInterrupt(PLIC_UART0_IRQn);
        UART0->FCR = 0xC6;//TODO:
    } else
    {
        //FIE_cmd_sts == FIE_CMD_EMPTY;
    }

}

//Fault Injection Env   CMD define
//byte      |      1-2     |   3    |  4-7  |  8-11  |    12    |    13-14    |
//function  | Begin of cmd | W/R/M  |  Addr |  Len   | Flip-pos |  End of cmd |
//content   |  A5    5A    | O 1    |       |        |          |   \r\n 0D 0A| for FIE
//content   |  A5    5A    |  2 3 4 |ImgBase| ImgNo  | 0:all 1:f|   \r\n 0D 0A|
void FIE_cmd_check_and_decode()
{
    if((FIE_frame[0] == 0xA5) && (FIE_frame[1] == 0x5A) && (FIE_frame[12] == '\r') && (FIE_frame[13] == '\n'))
    {
        mode = FIE_frame[2];
        addr = (FIE_frame[3]<<24) + (FIE_frame[4]<<16) + (FIE_frame[5] << 8) + FIE_frame[6] ;  
        len =  (FIE_frame[7]<<24) + (FIE_frame[8]<<16) + (FIE_frame[9] << 8) + FIE_frame[10] ;  
        flip_pos = FIE_frame[11]; 
        FIE_cmd_sts = FIE_CMD_CHECK;
    } else
    {
        printf("Please input the correct cmd frame\r\n");
        FIE_cmd_sts = FIE_CMD_FINISH;
    }
}

void FIE_cmd_exe()
{
    uint32_t datalen;
    if(mode == 0)//Write
    {
        for(datalen = 0 ; datalen < len ; datalen++)
        {
            //flip the certain bit of data 
            FIE_DATA(addr) ^= (1 << flip_pos); 
        }
    }else if (mode == 1)//Read
    {
        for(datalen = 0 ; datalen < len ; datalen++)
        {
            printf("The data of %x is %x. \r\n",addr,FIE_DATA(addr));
        }
    }else if((mode == 2) || (mode == 3) || (mode == 4))//mode2: Run_AI full speed mode    mode3: only record error output data  mode4:recode all the data.
    {
        //run_AImode(); TODO
        mnist_statistic_clear();
        if(mode == 4)
        {   //when recording all data, we should reset the model and record all reg data.
            mnist_init();
        }
        for(uint16_t i = (uint16_t)(addr) ;i< (uint16_t)(addr + len) ; i++)
        {
            mnist_imageprocess(i);
            mnist_run();
            mnist_predictresult();
            mnist_result_check(i,(mode-2));
        }
        if(mode == 4)
        {
            printf("print acc regs\n");
            for(int i=0;i<41;i++)
            {
                printf("reg %d is %08x\n",i,CSB_Read(i));
            }
            printf("print pool regs\n");
            for(int i=128;i<172;i++)
            {
                printf("reg %d is %08x\n",i,CSB_Read(i));
            }
            /*
            for(uint16_t k = 0 ; k <15 ; k ++ ) //1475；
            {
                printf("local %d is %d\n", k, testerrsample[k]);
            }*/
        }
    }else if(mode == 5) //此处最好加一个local data 的 monitor 以及 加速器所有寄存器数据的monitor   
    {
        printf("print acc regs\n");
        for(int i=0;i<41;i++)
        {
            printf("reg %d is %08x\n",i,CSB_Read(i));
        }
        for(int i=128;i<172;i++)
        {
            printf("reg %d is %08x\n",i,CSB_Read(i));
        }
        for(uint16_t k = 0 ; k <1475 ; k ++ )
        {
            printf("local %d is %d\n", k, testerrsample[k]);
        }
    }else if(mode == 6) //reset model
    {
        mnist_init();
    }
     

    printf("FIE cmd exe finish\r\n");
}


int uart0_init()
{
    uint32_t returnCode;
    uint8_t reg;
    //gpio_enable_interrupt(GPIOA, IOF_UART_MASK, GPIO_INT_RISE);
    //UART0->LCR = 0x00;
    returnCode = uart_enable_rx_th_int(UART0);
    UART0->FCR = 0xC6;
    returnCode |= PLIC_Register_IRQ(PLIC_UART0_IRQn, 1 ,muart0_irq_handler); /* register uart0 interrupt */
    //PLIC_DisableInterrupt(PLIC_UART0_IRQn);
    __enable_irq(); /* enable global interrupt */
    //__disable_core_irq(PLIC_UART0_IRQn); /* Disable MTIP iterrupt */
    /*
    while(1)
    {
        reg = uart_read(UART0);
        printf("%c",reg);
    }*/
    if (returnCode != 0) { /* Check return code for errors */
        return -1;
    }
    return 0;
}