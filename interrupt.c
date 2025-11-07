/*
 * interrupt.c -
 */
#include "sched.h"
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <keyboard.h>
#include <clock.h>
#include <syscall.h>
#include <page_fault.h>
#include <libc.h>
#include <utils.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void keyboard_service()
{
	unsigned char c, scancode;
	if ((c=inb(0x60)) && (c & 0x80)) {
		scancode = c & 0x7F;
		if(scancode < sizeof(char_map))				// Si es '\0' tambien se escribira 'C'
			c = char_map[scancode];
		else
			c = 'C';

		//task switch testing (to be deleted)
		if(char_map[scancode] == 'd') {
			task_switch(&(task[0]));
		} 
		if (char_map[scancode] == 'n') {
			task_switch(&(task[1]));
		}

		printc_xy(0, 0, c);
	}

	
}

int zeos_ticks = 0;

void clock_service() {
	zeos_ticks++;
	zeos_show_clock();
	scheduler();
}


void segmentation_fault_service(unsigned int eip, unsigned int err)
{
	char buff[8];

	int fault_addr;
	__asm__ __volatile__("mov %%cr2, %0" : "=r" (fault_addr));

	printk("\nProcess generates a ");

	//err handling
	if ((err >> 2) & 1)
		printk("user ");
	else
		printk("system ");

	if ((err >> 4) & 1)
		printk("instruction fetch ");
	else {
		if ((err >> 1) & 1)
			printk("write ");
		else
			printk("read ");
	}

	printk("(");
	itoa(err, buff);
	printk(buff);
	printk(") ");

	printk("PAGE FAULT exception\nEIP: 0x");
	itoa_hex(eip, buff);
	printk(buff);
	// printk(" (");
	// itoa(eip, buff);
	// printk(buff);
	// printk(")");
	
	printk("   fault address: 0x");
	itoa_hex(fault_addr, buff);
	printk(buff);
	// printk(" (");
	// itoa(fault_addr, buff);
	// printk(buff);
	// printk(")");

	while (1);
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(0x0E, segmentation_fault_handler, 0);
  setTrapHandler(0x80, system_call_handler, 3);
  set_idt_reg(&idtR);
}

