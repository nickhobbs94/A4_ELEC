#include <io.h>
#include <fcntl.h>
#include <unistd.h>   // for usleep()
#include "alt_types.h"

#include "altera_avalon_pio_regs.h"   // for GPIOs connected to buttons 
                         // and switches. Located under the 
						// bsp_folder/drivers/inc 


#include "altera_avalon_timer_regs.h" // for some macros used for 
						// dealing with timers. Located under the 
						// bsp_folder/drivers/inc



#include "sys/alt_irq.h"   // For interupt handlling
                           // located in bsp_folder/HAL/inc            



#include "system.h"       // system-wide standard macros
						  // located under the bsp_folder	


/* See 
 * 
 * http://en.wikipedia.org/wiki/Volatile_variable 
 * 
 * to understand the use of the keyword volatile
 * 
 * 
 * The variable ledg will be used to store the 
 * status of green LEDs
 */
static volatile alt_32 ledg = 3<<6;




/* Button pio functions */

/*
  Some simple functions to:
  1.  Define an interrupt handler function.
  2.  Register this handler in the system.
*/

/*******************************************************************
 * static void handle_button_interrupts( void* context, alt_u32 id)
 *                                                                 
 * Interrupt service routine for push-buttons.                     
 * This interrupt event is triggered by a push-button press.       
 * This handler modifies *context according as the value read from 
 * the button edge capture register.  The button edge capture register        
 * is then cleared and normal program execution resumes.           
 * 
 * To understand how the edge-capture register works, see Chapter 
 * 10 of the Embedded Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 10-5,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_pio_regs.h at the same time.                       
 *                                                                 
 * Provision is made here for systems that might have either the   
 * legacy or enhanced interrupt API active, or for the Nios II IDE 
 * which does not support enhanced interrupts. For systems created 
 * using the Nios II softawre build tools, the enhanced API is     
 * recommended for new designs.                                    
 * 
 * See project_bsp_folder/system.h and 
 * project_bsp_folder/drivers/inc/altera_avalon_pio_regs.h 
 * for the macros used in this function
 * 
 ******************************************************************/
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
static void handle_button_interrupts(void* context)
#else
static void handle_button_interrupts(void* context, alt_u32 id)
#endif
{
    /* Store the value in the Button's edge capture register in tmp. */
    alt_u32 tmp = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);
    /* Reset the Button's edge capture register. */
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0);

	/* Change the value of *context */
	*((alt_32*) context) &= ~(0xf);
	*((alt_32*) context) |= tmp;
	
	/* Write the value of *context on Green LEDs*/ 
	IOWR(LED_GREEN_BASE, 0, *((alt_32*) context)); 

    /*
     * Read the PIO to delay ISR exit. This is done to prevent a spurious
     * interrupt in systems with high processor -> pio latency and fast
     * interrupts.
     */
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);
}



/* ******************************************************************** 
 * static void init_button_pio()
 * 
 * Initialize the button_pio. 
 * See project_bsp_folder/system.h and 
 * project_bsp_folder/drivers/inc/altera_avalon_pio_regs.h 
 * for the macros used in these functions
 * 
 * To understand how the GPIO registers work, see Chapter 
 * 10 of the Embedded Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 10-5,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_pio_regs.h at the same time.                       
 * 
 * ********************************************************************/

static void init_button_pio()   
{
    /* Enable all 4 button interrupts. */
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_BASE, 0xf);


    /* Reset the edge capture register. */
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0x0);


    /* Register the interrupt handler.
     * Note how the address of ledg is being passed to irq register 
     * function. The irq register function will then copy this address on
     * the variable context within the interrupt handler function 
     * handle_button_interrupts */
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
    alt_ic_isr_register(BUTTON_PIO_IRQ_INTERRUPT_CONTROLLER_ID, 
                        BUTTON_PIO_IRQ,
                        handle_button_interrupts, 
                        &ledg, 
                        0x0);
#else
    alt_irq_register( BUTTON_PIO_IRQ, &ledg,handle_button_interrupts);
#endif

}


/* Switch pio functions */

/*
  Some simple functions to:
  1.  Define an interrupt handler function.
  2.  Register this handler in the system.
*/

/*******************************************************************
 * static void handle_switch_interrupts( void* context, alt_u32 id)
 *                                                                 
 * Handle interrupts from the switchs.                             
 * This interrupt event is triggered by a switch press.     
 * This handler does not use *context. Instead it copies the value read 
 * from the button edge capture register onto red LEDs. 
 * The button edge capture register        
 * is then cleared and normal program execution resumes.           
 *                                                                 
 * To understand how the edge-capture register works, see Chapter 
 * 10 of the Embedded Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 10-5,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_pio_regs.h at the same time.                       

 * Provision is made here for systems that might have either the   
 * legacy or enhanced interrupt API active, or for the Nios II IDE 
 * which does not support enhanced interrupts. For systems created 
 * using the Nios II softawre build tools, the enhanced API is     
 * recommended for new designs.                                    
 ******************************************************************/

#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
static void handle_switch_interrupts(void* context)
#else
static void handle_switch_interrupts(void* context, alt_u32 id)
#endif
{
    /* We don't use context in this interrupt handler */
    
    /* Store the value in the switch's edge capture register in x */
    alt_32 x = IORD_ALTERA_AVALON_PIO_EDGE_CAP(SWITCH_PIO_BASE);

    /* Reset the switch's edge capture register. */
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SWITCH_PIO_BASE, 0);


	/* Write the value of x onto Red LEDs */
    IOWR(LED_RED_BASE, 0, x);

    /*
     * Read the PIO to delay ISR exit. This is done to prevent a spurious
     * interrupt in systems with high processor -> pio latency and fast
     * interrupts.
     */
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(SWITCH_PIO_BASE);
}

/* Initialize the switch_pio. ******************************************
 * See project_bsp_folder/system.h and 
 * project_bsp_folder/drivers/inc/altera_avalon_pio_regs.h 
 * for the macros used in these functions
 * 
 * To understand how the GPIO registers work, see Chapter 
 * 10 of the Embedded Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 10-5,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_pio_regs.h at the same time.                       
 * ********************************************************************/

static void init_switch_pio()
{
    /* Enable all 4 switch interrupts. */
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(SWITCH_PIO_BASE, 0x3ffff);

    /* Reset the edge capture register. */
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SWITCH_PIO_BASE, 0x0);

    /* Register the interrupt handler. */
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
    alt_ic_isr_register(SWITCH_PIO_IRQ_INTERRUPT_CONTROLLER_ID, 
                        SWITCH_PIO_IRQ,
                        handle_switch_interrupts, 
                        NULL, 
                        0x0);
#else
    alt_irq_register( SWITCH_PIO_IRQ, NULL,  handle_switch_interrupts);
#endif
}





/*******************************************************************
 * static void handle_timer0_interrupts( void* context, alt_u32 id)
 *                                                                 
 * Handle interrupts from timer 0.                             
 * This interrupt event is triggered when timer 0 count becomes 0.     
 * This handler toggles the state of the bit 7 of *context.
 *                                                                 
 * To understand how the timers work, see Chapter 28 of the Embedded 
 * Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 28-4,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_timer_regs.h at the same time.                       
 ******************************************************************/

static void handle_timer0_interrupts(void* context, alt_u32 id) {
	//Reset T0 bit in status register to clear the timer flag
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE,0);

	// change the value of the *context
	*((alt_32*) context) ^= 1<<7;
	
	IOWR(LED_GREEN_BASE, 0, *((alt_32*) context)); // Write the value of *context
	                                   // on Green LEDs 

	//Enable interrupt,start timer, reset CONT bit
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE,5);
}




/* static void init_timer0()   --  Initialize timer 0. *****************
 * See project_bsp_folder/system.h and 
 * project_bsp_folder/drivers/inc/altera_avalon_timer_regs.h 
 * for the macros used in these function
 * 
 * To understand how the timers  work, see Chapter 
 * 28 of the Embedded Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 28-4,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_timer_regs.h at the same time.                       
 * ********************************************************************/

static void init_timer0(){

	//Load values to timer 0 period registers
	IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_0_BASE,0xff);
	IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_0_BASE,0xffff);

	//Reset T0 bit in status register
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE,0);

    /* Install timer 0 ISR
     * Note how the address of ledg is being passed to irq register 
     * function. The irq register function will then copy this address on
     * the variable context within the interrupt handler function 
     * handle_timer0_interrupts */
	alt_irq_register( TIMER_0_IRQ, &ledg,  handle_timer0_interrupts);

    //Enable interrupt,start timer, set CONT bit 0
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE,5);
}


/*******************************************************************
 * static void handle_timer1_interrupts( void* context, alt_u32 id)
 *                                                                 
 * Handle interrupts from timer 1.                             
 * This interrupt event is triggered when timer 1 count becomes 0.     
 * This handler toggles the state of the bit 6 of *context.
 *                                                                 
 * To understand how the timers work, see Chapter 28 of the Embedded 
 * Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 28-4,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_timer_regs.h at the same time.                       
 ******************************************************************/

static void handle_timer1_interrupts(void* context, alt_u32 id) {
	//Reset T0 bit in status register
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE,0);

	// change the value of static int tmp defined in init_timer0()
	alt_32* temp = (alt_32*) context;
	*temp ^= 1<<6;
	IOWR(LED_GREEN_BASE, 0, *temp);

	//Enable interrupt,start timer, set CONT bit 0
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_1_BASE,5);
}


/* static void init_timer1()   --  Initialize timer 1. *****************
 * See project_bsp_folder/system.h and 
 * project_bsp_folder/drivers/inc/altera_avalon_timer_regs.h 
 * for the macros used in these function
 * 
 * To understand how the timers  work, see Chapter 
 * 28 of the Embedded Peripherals IP User Guide from Altera.       
 * To understand the software programming aspects, see Page 28-4,  
 * Section entitled "Software Programming Model". It  might be     
 * useful to read this section and refer to the files system.h and 
 * altera_avalon_timer_regs.h at the same time.                       
 * ********************************************************************/

static void init_timer1(){
	//Load values to timer 1 period registers
	IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_1_BASE,0x1ff);
	IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_1_BASE,0xfffe);

	//Reset T0 bit in status register
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE,0);

    /* Install timer 1 ISR
     * Note how the address of ledg is being passed to irq register 
     * function. The irq register function will then copy this address on
     * the variable context within the interrupt handler function 
     * handle_timer1_interrupts */
	alt_irq_register( TIMER_1_IRQ, &ledg,  handle_timer1_interrupts);

    //Enable interrupt,start timer, set CONT bit 0
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_1_BASE,5);
}




/*
alt_16 hardware_init(void)
{
    AUDIO_Init();
    return 0;
}
*/

int main(int argc, char* argv[]) {
    init_button_pio();
    init_switch_pio();
    init_timer0();
    init_timer1();
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
    printf("Enhanced present\n");
#else
    printf("Non-enhanced present\n");
#endif
    while(1);
}
