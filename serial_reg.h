/*
 * Streamlined header file for the 16550A, which is the one supported
 *  by the vbox, based on:
 *  
 * include/linux/serial_reg.h
 *
 * Copyright (C) 1992, 1994 by Theodore Ts'o.
 * 
 * Redistribution of this file is permitted under the terms of the GNU 
 * Public License (GPL)
 * 
 * These are the UART port assignments, expressed as offsets from the base
 * register.  These assignments should hold for any serial port based on
 * a 8250, 16450, or 16550(A).
 */

#ifndef _SERIAL_REG_H
#define _SERIAL_REG_H

/*
 * DLAB=0
 */
#define UART_RX		0	/* In:  Receive buffer */
#define UART_TX		0	/* Out: Transmit buffer */

#define BASE        0x3F8

#define UART_IER	1	/* Out: Interrupt Enable Register */
#define UART_IER_MSI		0x08 /* Enable Modem status interrupt */
#define UART_IER_RLSI		0x04 /* Enable receiver line status interrupt */
#define UART_IER_THRI		0x02 /* Enable Transmitter holding register int. */
#define UART_IER_RDI		0x01 /* Enable receiver data interrupt */

#define UART_IIR	2	/* In:  Interrupt ID Register */
/* Check Table IV on pg 17 of the PC16550D data sheet */
#define UART_IIR_NO_INT		0x01 /* No interrupts pending */
#define UART_IIR_ID		0x06 /* Mask for the interrupt ID */
#define UART_IIR_MSI		0x00 /* Modem status interrupt */
#define UART_IIR_THRI		0x02 /* Transmitter holding register empty */
#define UART_IIR_RDI		0x04 /* Receiver data interrupt */
#define UART_IIR_RLSI		0x06 /* Receiver line status interrupt */

#define UART_FCR	2	/* Out: FIFO Control Register */
/* Check Section 8.5 on pg 17 of the PC16550D data sheet */
#define UART_FCR_ENABLE_FIFO	0x01 /* Enable the FIFO */
#define UART_FCR_CLEAR_RCVR	0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04 /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT	0x08 /* For DMA applications */

#define UART_FCR_TRIGGER_MASK	0xC0 /* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1	0x00 /* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4	0x40 /* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8	0x80 /* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14	0xC0 /* Mask for trigger set at 14 */

#define UART_LCR	3	/* Out: Line Control Register */
/*
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting 
 * UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */

/* Check Table II on pg 14 of the PC16550D data sheet, and 
 *  table in first column of pg 15, for the word length */

#define UART_LCR_DLAB		0x80 /* Divisor latch access bit */
#define UART_LCR_SBC		0x40 /* Set break control */
#define UART_LCR_SPAR		0x20 /* Stick parity (?) */
#define UART_LCR_EPAR		0x10 /* Even parity select */
#define UART_LCR_PARITY		0x08 /* Parity Enable */
#define UART_LCR_STOP		0x04 /* Stop bits: 0=1 bit, 1=2 bits */
#define UART_LCR_WLEN5		0x00 /* Wordlength: 5 bits */
#define UART_LCR_WLEN6		0x01 /* Wordlength: 6 bits */
#define UART_LCR_WLEN7		0x02 /* Wordlength: 7 bits */
#define UART_LCR_WLEN8		0x03 /* Wordlength: 8 bits */

#define UART_MCR	4	/* Out: Modem Control Register */

/* Check Table II on pg 14 of the PC16550D data sheet */
/* Not relevant: we are not controlling a modem */

#define UART_LSR	5	/* In:  Line Status Register */

/* Check Table II on pg 14 of the PC16550D data sheet */

#define UART_LSR_TEMT		0x40 /* Transmitter empty */
#define UART_LSR_THRE		0x20 /* Transmit-hold-register empty */
#define UART_LSR_BI		0x10 /* Break interrupt indicator */
#define UART_LSR_FE		0x08 /* Frame error indicator */
#define UART_LSR_PE		0x04 /* Parity error indicator */
#define UART_LSR_OE		0x02 /* Overrun error indicator */
#define UART_LSR_DR		0x01 /* Receiver data ready */

#define UART_MSR	6	/* In:  Modem Status Register */

/* Check Table II on pg 14 of the PC16550D data sheet */
/* Not relevant: we are not interfacing a modem */

#define UART_SCR	7	/* I/O: Scratch Register */

/*
 * DLAB=1
 */
#define UART_DLL	0	/* Out: Divisor Latch Low */
#define UART_DLM	1	/* Out: Divisor Latch High */

/* Check 2nd column of Table III on pg 15 of the PC16550D data sheet */

#define UART_DIV_1200   96	/* Divisor for 1200 bps */
#define UART_DIV_9600   12	/* Divisor for 9600 bps */

#endif /* _SERIAL_REG_H */

