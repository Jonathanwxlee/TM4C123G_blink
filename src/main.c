#include <stdint.h>
/*

general notes:
|= is the bitwise OR operator
& is bitwise AND
<< is left shift, which means move the bits of a number to the left x spots. 
    - for example: 1U << 1 means move 00000001 over one spot to get -> 00000010. 

*/

/*
System control registers:
------------------------------------------
RCGCGPIO (found at 0x400FE608) is a 32 bit control register. But, only the first 6 bits are used. This register is a "Run Mode Clock Gating Control" register used specifically for the GPIO peripherals
6 bits control whether the different GPIO peripherals get clock signals for operation. 
The other 26 bits are unused (if you flip them it is undefined behavior)
All peripherals need clock signals in order to operate.
We control whether or not to send clock signals to peripherals to save power. 

Register: | 00000000 00000000 00000000 00[000000] |

When the bits are flipped, clock signals will be sent to the corresponding GPIO peripheral (A, B, C, D, E, F)
------------------------------------------
PRGPIO (found at 0x400FEA08) is a 32 bit status register. This register is a "Peripheral Ready Register" specifically for the GPIO peripherals. 
6 bits indicate when the 6 GPIO peripherals are ready to be written to, after a clock signal is sent.
This register shuodl continually be checked to ensure the GPIO peripherals are ready for writing. 

Register: | 00000000 00000000 00000000 00[000000] |

When the bits are flipped, it indicates which GPIO peripheral is ready for access (A, B, C, D, E, F)
------------------------------------------
*/
#define SYSCTL_RCGCGPIO_R  (*((volatile uint32_t *)0x400FE608)) // the volatile keyword is important here because it indicates that the variable may be changed even if there are no explicit instructions in the program that changed it. So, this keyword tells the compiler to always read directly from the memory address instead of performing compiler optimizations 
#define SYSCTL_PRGPIO_R    (*((volatile uint32_t *)0x400FEA08)) 

/*
GPIO Port F peripheral registers
------------------------------------------
| 0x40025000 :     |
|    ...     :     |
| 0x40025400 : DIR | -> this 32 bit register's purpose is to set pins as input or output
|    ...     :     |
| 0x4002551C : DEN | -> this 32 bit register's purpose is to enable digital functionality 
|            :     |
...
|            :     |

DATA addresses for all the pins of Port F are scattereda round the register address block. However, there is a formula for determining this
pin DATA address = [base address] + ([PIN mask] x 4)
------------------------------------------
DIR: This register determines the direction of each PIN on the GPIO port F peripheral.
In other words, it determines whether the PIN is used for input or output
Only 5 bits are used - one for each of the pins in the peripheral (PF0, PF1, PF2, PF3, PF4)

Register: | 00000000 00000000 00000000 000[00000] |

When the bit is 0, it means that corresponding pin is input. When the bit is 1, the corresponding pin is output
------------------------------------------
DEN: This register determines whether a GPIO pin's digital input/output functionality is enabled. 
Basically, we need DEN in order for DIR to work in the first place. 
Again, for the GPIO Port F peripheral, only 5 bits are used since there are only 5 pins in the Port F peripheral (PF0, PF1, PF2, PF3, PF4)

Register: | 00000000 00000000 00000000 000[00000] |

When the bit is 0, the corresponding pin's digital input/output functionality is disabled. When the bit is 1, the corresponding pin's digital input/output functionality is ENABLED
------------------------------------------
*/
#define GPIO_PORTF_BASE    0x40025000 // this is the starting memory address for GPIO port F peripheral's registers. 
#define GPIO_PORTF_DIR_R   (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x400)))
#define GPIO_PORTF_DEN_R   (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x51C)))

/*PF1 controls the red LED

Each pin of the GPIO port F peripheral corresponds to a bit. 
PF0 : 00000001 : 0x01
PF1 : 00000010 : 0x02
PF2 : 00000100 : 0x04
PF3 : 00001000 : 0x08
PF4 : 00010000 : 0x10

definig RED_LED below is just a mask we can use with an identifiable name whenver we want to set the PF1 pin. 
By setting this directive, we are saying that this pin will always be associated with the red led. 

*/ 
#define RED_LED            0x02

void delay(volatile uint32_t count)
{
    while (count > 0)
    {
        count--;
    }
}

int main(void)
{
    // 1. Enable the clock for GPIO Port F
    SYSCTL_RCGCGPIO_R |= (1U << 5);

    // 2. Wait until GPIO Port F is ready
    while ((SYSCTL_PRGPIO_R & (1U << 5)) == 0)
    {
        // Wait (I didn't know you could stall with empty while loops)
    }

    // 3. Configure PF1 as an output
    GPIO_PORTF_DIR_R |= RED_LED;

    // 4. Enable digital functionality on PF1
    GPIO_PORTF_DEN_R |= RED_LED;

    // GPIO masked data register for PF1. So this led uint32 represents the address of the DATA register for PF1
    volatile uint32_t *led = (volatile uint32_t *)(GPIO_PORTF_BASE + (RED_LED << 2));

    // 5. Continuously blink the red LED
    while (1)
    {
        *led = RED_LED;  // LED ON
        delay(1600000);

        *led = 0;        // LED OFF
        delay(1600000);
    }

    return 0;
}
