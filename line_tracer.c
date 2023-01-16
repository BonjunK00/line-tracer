#include "msp.h"
#include "Clock.h"
#include <stdio.h>

void led_init() {
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;

    P2->DIR |= 0x07;

    P2->OUT &= ~0x07;
}
void switch_init() {
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;

    P1->DIR &= ~0x12;

    P1->REN |= 0x12;

    P1->OUT |= 0x12;
}
void systick_init(void) {
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL = 0x00000005;
}
void IRsensor_init() {
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR |= 0xFF;

}
void pwm_init34(uint16_t period, uint16_t duty3, uint16_t duty4) {
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;

    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;
}
void motor_init(void) {
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;

    pwm_init34(7500, 0, 0);
}
void move(uint16_t leftDuty, uint16_t rightDuty) {
    P3->OUT |= 0xC0;
    TIMER_A0->CCR[3] = leftDuty;
    TIMER_A0->CCR[4] = rightDuty;
}
void left_forward() {
    P5->OUT &= ~0x10;
}
void left_backward() {
    P5->OUT |= 0x10;
}
void right_forward() {
    P5->OUT &= ~0x20;
}
void right_backward() {
    P5->OUT |= 0x20;
}
void systick_wait1ms() {
    SysTick->LOAD = 0x0000BB80;
    SysTick->VAL = 0;
    while((SysTick->CTRL & 0x00010000) == 0) {};

}
void systick_wait_ms(int ms){
    int i;

    for (i = 0; i < ms; i++) {
        systick_wait1ms();
    }
}
void turn_on_led(int color) {
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}
void turn_off_led() {
    P2->OUT &= ~0x07;
}
void timer_A3_capture_init() {
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;

    TIMER_A3->CCTL[0] = 0x4910;
    TIMER_A3->CCTL[1] = 0x4910;
    TIMER_A3->EX0 &= ~0x0007;

    NVIC->IP[3] = (NVIC->IP[3]&0x0000FFFF) | 0x40400000;
    NVIC->ISER[0] = 0x0000C000;
    TIMER_A3->CTL |= 0x0024;
}
uint32_t left_count;
uint32_t right_count;
void TA3_0_IRQHandler(void) {
    TIMER_A3->CCTL[0] &= ~0x0001;
    left_count++;
}
void TA3_N_IRQHandler (void) {
    TIMER_A3->CCTL[1] &= ~0x0001;
    right_count++;
}
void push(int sec) {
    left_forward();
    right_forward();
    move(3000, 3000);
    systick_wait_ms(sec);
}
void turn_right(int degree) {
    left_count = 0;
    right_count = 0;

    left_forward();
    right_backward();
    move(2500, 2500);
    while(1) {
        if (left_count > degree * 2 && right_count > degree * 2)
            break;
    }
    push(120);
}
void turn_left(int degree) {
    left_count = 0;
    right_count = 0;

    left_backward();
    right_forward();
    move(2500, 2500);
    while(1) {
        if (left_count > degree * 2 && right_count > degree * 2)
            break;
    }
    push(120);
}
void trace_line(int until) {
    while (1) {
        int sensor;

        P5->OUT |= 0x08;
        P9->OUT |= 0x04;

        P7->DIR = 0xFF;

        P7->OUT = 0xFF;

        Clock_Delay1us(10);

        P7->DIR = 0x00;

        Clock_Delay1us(1000);

        sensor = P7->IN;

        left_forward();
        right_forward();

        if (sensor & until) {
            break;
        }
        else if (sensor & 0x40) {
            left_backward();
            move(2000, 2000);
        }
        else if (sensor & 0x02){
            right_backward();
            move(2000, 2000);
        }
        else if (sensor & 0x20){
            left_backward();
            move(1500, 1500);
        }
        else if (sensor & 0x04){
            right_backward();
            move(1500, 1500);

        }
        else {
            move(2000, 2200);
        }
        systick_wait_ms(10);

        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;

        Clock_Delay1ms(5);
    }
    push(70);
}
void trace_line_fast(int until) {
    while (1) {
        int sensor;

        P5->OUT |= 0x08;
        P9->OUT |= 0x04;

        P7->DIR = 0xFF;

        P7->OUT = 0xFF;

        Clock_Delay1us(10);

        P7->DIR = 0x00;

        Clock_Delay1us(1000);

        sensor = P7->IN;

        left_forward();
        right_forward();

        if (sensor & until) {
            break;
        }
        else if (sensor & 0x40) {
            left_backward();
            move(2000, 1000);
         }
        else if (sensor & 0x02){
            right_backward();
            move(1000, 2000);
        }
        else if (sensor & 0x20){
            left_backward();
            move(1500, 1500);
        }
        else if (sensor & 0x04){
            right_backward();
            move(1500, 1500);

        }
        else {
            move(3500, 3700);
        }
        systick_wait_ms(5);

        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;

        Clock_Delay1ms(5);
    }
    push(40);
}
void main(void)
{
    Clock_Init48MHz();
    led_init();
    switch_init();
    systick_init();
    IRsensor_init();
    motor_init();
    timer_A3_capture_init();

    int i = 2;
    while(i--) {
    // First course
    trace_line_fast(0b00000001);
    turn_right(90);
    trace_line_fast(0b00000001);
    turn_right(90);
    trace_line_fast(0b00000001);

    push(200);
    trace_line(0b00000001);
    turn_right(85);
    trace_line(0b10000000);
    turn_left(70);
    trace_line(0b00000001);
    turn_right(85);
    trace_line(0b10000000);
    turn_left(70);
    trace_line(0b00000001);
    push(200);
    trace_line_fast(0b10000000);
    turn_left(75);
    trace_line_fast(0b01000000);
    turn_left(85);

    // Second course
    trace_line(0b00000010);
    turn_right(75);
    trace_line(0b00000001);
    turn_right(85);
    trace_line(0b10000000);
    turn_left(75);
    trace_line(0b10000000);
    turn_left(80);
    trace_line(0b10000000);
    turn_left(85);
    trace_line(0b10000000);
    turn_left(90);
    //Check!

    // Third course
    trace_line(0b10000000);
    turn_left(70);
    trace_line(0b10000000);
    turn_left(70);
    trace_line(0b10000000);
    turn_left(70);
    trace_line(0b10000000);
    turn_left(70);
    trace_line(0b10000000);
    turn_left(70);
    trace_line_fast(0b00000001);
    turn_right(85);

    // Final course
    trace_line_fast(0b00000010);
    turn_right(125);
    push(200);
    trace_line_fast(0b10000000);
    turn_left(75);
    trace_line_fast(0b00000001);
    turn_right(95);
    trace_line_fast(0b00000001);
    turn_right(85);
    trace_line_fast(0b10000000);
    turn_left(75);
    trace_line_fast(0b10000000);
    // bold
    push(200);
    trace_line_fast(0b10000000);
    turn_left(70);
    trace_line_fast(0b10000000);
    turn_left(70);
    trace_line_fast(0b10000000);
    push(200);
    //bold
    trace_line_fast(0b00000001);
    turn_right(85);
    trace_line_fast(0b00000001);
    turn_right(90);
    trace_line(0b10000000);
    turn_left(85);
    trace_line(0b00000001);
    turn_right(85);
    trace_line(0b00000001);
    turn_right(85);
    trace_line(0b10000000);
    turn_left(70);
    trace_line(0b00000001);
    turn_right(85);
    trace_line_fast(0b01000000);
    if(i == 1)
        push(300);
    else
        move(0, 0);
    }
}
