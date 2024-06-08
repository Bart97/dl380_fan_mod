#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>

enum Timer0Prescaler : uint8_t
{
    timer0Prescaler1x = (1 << CS00),
    timer0Prescaler8x = (1 << CS01),
    timer0Prescaler64x = (1 << CS01) | (1 << CS00),
};

void setupPB2Interrupt(void)
{
    const uint8_t int0TriggerOnFallingEdge = (1 << ISC00);
    EICRA = int0TriggerOnFallingEdge;
    EIMSK = (1 << INT0);

//    PCMSK = (1 <<PCINT2);
//    PCICR = (1 <<PCIE0);
}

void initGpio(void)
{
    const uint8_t PB1output = 1 << DDB1;
    const uint8_t PB2input = 0;
    DDRB = PB1output | PB2input;
    PORTB = 0;
    PINB = 0;
    setupPB2Interrupt();
}

static inline bool isFailureIndicated(void)
{
    const bool isPB1High = PORTB & (1 << PORTB1);
    return !isPB1High;
}

void initClock(void)
{
    const uint8_t overflowInterruptEnable = (1 << TOIE0);
    TIMSK0 |= overflowInterruptEnable;
    TCCR0B |= timer0Prescaler8x;
}

static inline void resetClock(void)
{
    TCNT0 = 0;
}

int main(void)
{
    initGpio();
    initClock();
    sei();
    while (true)
    {
        asm("NOP");
    }
}

const uint8_t requiredRecoverPulseCount = 40;
uint8_t recoverPulseCount = 0;

const uint8_t requiredMissedPulseCount = 2;
uint8_t missedPulseCount = 0;

ISR(TIM0_OVF_vect)
{
    if (!isFailureIndicated())
    {
        if (++missedPulseCount >= requiredMissedPulseCount)
        {
            PORTB = 0;
        }
    }
    recoverPulseCount = 0;
}

ISR(INT0_vect)
{
    if (isFailureIndicated())
    {
        if (++recoverPulseCount >= requiredRecoverPulseCount)
        {
            PORTB = 1 << PORTB1;
        }
    }
    missedPulseCount = 0;
    resetClock();
}

//ISR(PCINT0_vect)
//{
//    PORTB = 1 << PORTB1;
//}
