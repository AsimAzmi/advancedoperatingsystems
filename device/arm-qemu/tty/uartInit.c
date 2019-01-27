/**
 * @file uartInit.c
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */
#include <xinu.h>
#include <uart.h>

/**
 * @ingroup uartgeneric
 *
 * Global table of UART devices that are available in the current Embedded Xinu
 * configuration.
 */
struct uart_csreg uarttab[NUART];

/**
 * @ingroup uartgeneric
 *
 * Initialize a UART, including the entry in Xinu's UART table as well as the
 * hardware itself.
 *
 * @param devptr
 *      Pointer to the device table entry for the UART.
 *
 * @return OK on success; SYSERR on failure.
 */
devcall ttyinit(struct dentry *devptr)
{
    /* Initialize the entry for this UART in Xinu's UART table.  */

    struct uart_csreg *uartptr = &uarttab[devptr->dvminor];

    /* Initialize statistical counts. */
    uartptr->cout = 0;
    uartptr->cin = 0;
    uartptr->lserr = 0;
    uartptr->ovrrn = 0;
    uartptr->iirq = 0;
    uartptr->oirq = 0;

    /* Initialize the input buffer, including a semaphore for threads to wait
     * on.  */
    uartptr->isema = semcreate(0);
    uartptr->iflags = UART_IFLAG_ECHO | TTY_ICRNL;
    uartptr->istart = 0;
    uartptr->icount = 0;
    if (isbadsem(uartptr->isema))
    {
        return SYSERR;
    }

    /* Initialize the output buffer, including a semaphore for threads to wait
     * on.  */
    uartptr->osema = semcreate(UART_OBLEN);
    uartptr->oflags = 0;
    uartptr->ostart = 0;
    uartptr->ocount = 0;
    uartptr->oidle = 1;
    if (isbadsem(uartptr->osema))
    {
        semdelete(uartptr->isema);
        return SYSERR;
    }

    /* Initialize the actual hardware.  */
    if (OK != uartHwInit(devptr))
    {
        semdelete(uartptr->isema);
        semdelete(uartptr->osema);
        return SYSERR;
    }
    /* Save pointers to the device and the hardware registers in the UART
     * structure.  */
    uartptr->dev = devptr;
    uartptr->csr = devptr->dvcsr;
    return OK;
}
