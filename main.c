#include "stm32f0xx.h"

#include "stm32f0_discovery.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>




#define COL1 (1 << 0)

#define COL2 (1 << 1)

#define COL3 (1 << 2)

#define COL4 (1 << 3)



#define ROW1 (1 << 4)

#define ROW2 (1 << 5)

#define ROW3 (1 << 6)

#define ROW4 (1 << 7)



#define SAMPLE_TIME_MS 10

#define SAMPLE_COUNT (SAMPLE_TIME_MS)



#define THRESHOLD_TIME_MS 2

#define THRESHOLD (THRESHOLD_TIME_MS)



#define KEY_PRESS_MASK  0b11000111

#define KEY_REL_MASK    0b11100011

void init_lcd(void);
//void display1(const char *);
//void display2(const char *);

// These are function pointers.  They can be called like functions
// after you set them to point to other functions.
// e.g.  cmd = bitbang_cmd;
// They will be set by the stepX() subroutines to point to the new
// subroutines you write below.
void (*cmd)(char b) = 0;
void (*data3)(char b) = 0;
void (*display1)(const char *) = 0;
void (*display2)(const char *) = 0;
void circdma_display1(const char *);
void circdma_display2(const char *);

// Prototypes for subroutines in support.c
void generic_lcd_startup(void);
void clock(void);
void step6(void);
volatile int portApin15Counter = 1;
volatile int portApin15Counter2 = 1;
void spi_cmd(char);
void spi_data(char);
void init_tim2(void);
void dma_spi_init_lcd(void);
void spi_init_lcd(void);
volatile uint32_t frequency;
volatile uint32_t angle;
void setup_dac(void);
void setup_timer3(void);
void setup_tim6(void);
void make_wavetable(void);
void init_keypad(void);
int16_t wavetable[256];
int flag = 0;
int modeNum = 0; // easy
char mode[] = "Easy";
static int tenths = 0;
static int seconds = 20;
static int minutes = 0;
static int hours = 0;
int lvl1 = 1;
int offset1 = 0;
int step1[] = {3.5 * (1<<16), 3.5 * (1<<16), 3.5 * (1<<16), 5.207 * (1<<16)};
int bpm[] = {1, 1, 1, 1};
int lose[] = {2.207 * (1<<16), 1.86 * (1<<16)};
int losebpm[] = {1, 1};
int buttonPress[] = {0.77 * (1<<16), 1.03 * (1<<16), 1.29 * (1<<16), 1.55 * (1<<16)};
int pressbpm[] = {1, 1, 1, 1};
int sound[] = {3 * (1<<16)};
int soundbpm[] = {1};
volatile int freq;
int countdownFlag = 0;
int gameoverFlag = 0;

// This array will be used with dma_display1() and dma_display2() to mix
// commands that set the cursor location at zero and 64 with characters.
//
uint16_t dispmem[34] = {
        0x080 + 0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x080 + 64,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};


int col = 0;
int row = -1;
int col2 = 0;
int row2 = -1;
int dontCheck = 0;
int mode2 = 0;
int soundCheck = 0;
int soundCounter = 0;

char char_array[4][4] = { {'1', '2', '3', 'A'},
                          {'4', '5', '6', 'B'},
                          {'7', '8', '9', 'C'},
                          {'*', '0', '#', 'D'} };

/*
char char_array[4][4] = { {'1', '4', '7', '*'},
                          {'2', '5', '8', '0'},
                          {'3', '6', '9', '#'},
                          {'A', 'B', 'C', 'D'} };
*/

int int_array[4][4] =   { {3,7,11,15},
                          {2,6,10,14},
                          {1,5,9,13},
                          {0,4,8,12} };
/*
int int_array2[4][4] =   { {3,2,1,0},
                          {7,6,5,4},
                          {11,10,9,8},
                          {15,14,13,12} };*/
/*
int int_array2[4][4] =   { {0,4,8,12},
                          {1,5,9,13},
                          {2,6,10,14},
                          {3,7,11,15} };
*/
int int_array2[4][4] =   { {0,3,2,1},
                          {4,7,6,5},
                          {8,11,10,9},
                          {12,15,14,13} };

uint8_t key_samples[4][4]  = { {0}, {0}, {0}, {0} };
uint8_t key_pressed[4][4]  = { {0}, {0}, {0}, {0} };
uint8_t key_released[4][4]  = { {0}, {0}, {0}, {0} };

uint8_t key_samples2[4][4]  = { {0}, {0}, {0}, {0} };
uint8_t key_pressed2[4][4]  = { {0}, {0}, {0}, {0} };
uint8_t key_released2[4][4]  = { {0}, {0}, {0}, {0} };

int data[20] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int easy1[20] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int data2[20] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int easy2[20] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int easy2a[20] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

char get_char_key() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(key_released[i][j] == 1 && key_pressed[i][j] == 1) {
                key_released[i][j] = 0;
                key_pressed[i][j] = 0;
                return char_array[i][j];
            }
        }
    }

    return '\0';
}

void setupPorts(void){

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

 //   GPIOC->MODER |= GPIO_MODER_MODER8_0;

 //   GPIOC->MODER |= GPIO_MODER_MODER9_0;

    GPIOC->MODER &= ~GPIO_MODER_MODER0;
    GPIOC->MODER &= ~GPIO_MODER_MODER1;
    GPIOC->MODER &= ~GPIO_MODER_MODER2;


    GPIOC->MODER |= GPIO_MODER_MODER0_0;
    GPIOC->MODER |= GPIO_MODER_MODER1_0;
    GPIOC->MODER |= GPIO_MODER_MODER2_0;


    //PB5,6,7
    GPIOB->MODER &= ~GPIO_MODER_MODER5;
    GPIOB->MODER &= ~GPIO_MODER_MODER6;
    GPIOB->MODER &= ~GPIO_MODER_MODER7;

    GPIOB->MODER |= GPIO_MODER_MODER5_0;
    GPIOB->MODER |= GPIO_MODER_MODER6_0;
    GPIOB->MODER |= GPIO_MODER_MODER7_0;

}

void clock_out(){
    GPIOC->ODR |= GPIO_ODR_0;
    nano_wait(200);
    GPIOC->ODR &= ~GPIO_ODR_0;
    nano_wait(200);
}

void clock_out2(){
    GPIOB->ODR |= GPIO_ODR_5;
    nano_wait(200);
    GPIOB->ODR &= ~GPIO_ODR_5;
    nano_wait(200);
}

void send_data_row1(int data[]){

    int i;
    GPIOC->ODR &= ~GPIO_ODR_1;

    for(i = 4; i < 8; i++){
           if(data[i] == 0){
               GPIOC->ODR &= ~GPIO_ODR_2;
               clock_out();
               clock_out();
               clock_out();
           }
           else if(data[i] == 1){
               GPIOC->ODR |= GPIO_ODR_2;
               clock_out();
               GPIOC->ODR &= ~GPIO_ODR_2;

               clock_out();
               clock_out();
           }
           else if(data[i] == 2){
               GPIOC->ODR &= ~GPIO_ODR_2;
               clock_out();
               GPIOC->ODR |= GPIO_ODR_2;
               clock_out();
               GPIOC->ODR &= ~GPIO_ODR_2;
               clock_out();
         }
           else if(data[i] == 3){
               GPIOC->ODR &= ~GPIO_ODR_2;
               clock_out();
               clock_out();
               GPIOC->ODR |= GPIO_ODR_2;
               clock_out();
         }
    }

    GPIOC->ODR |= GPIO_ODR_2;
    clock_out();
    clock_out();
    clock_out();

    if(data[3] == 0){
                GPIOC->ODR |= GPIO_ODR_2;
            }
    else{
                GPIOC->ODR &= ~GPIO_ODR_2;
            }
    clock_out();

    GPIOC->ODR |= GPIO_ODR_1;
}

void send_data_row2(int data[]){
    GPIOC->ODR &= ~GPIO_ODR_1;
    int i;

    for(i = 8; i < 12; i++){
        if(data[i] == 0){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            clock_out();
            clock_out();
        }
        else if(data[i] == 1){
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
            GPIOC->ODR &= ~GPIO_ODR_2;

            clock_out();
            clock_out();
        }
        else if(data[i] == 2){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
      }
        else if(data[i] == 3){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            clock_out();
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
      }
    }

    GPIOC->ODR |= GPIO_ODR_2;
    clock_out();
    clock_out();

    if(data[2] == 0){
                GPIOC->ODR |= GPIO_ODR_2;
            }
    else{
                GPIOC->ODR &= ~GPIO_ODR_2;
            }
    clock_out();

    GPIOC->ODR |= GPIO_ODR_2;
    clock_out();
    GPIOC->ODR |= GPIO_ODR_1;
}

void send_data_row3(int data[]){

    int i;
    GPIOC->ODR &= ~GPIO_ODR_1;
    for(i = 12; i < 16; i++){
        if(data[i] == 0){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            clock_out();
            clock_out();
        }
        else if(data[i] == 1){
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
            GPIOC->ODR &= ~GPIO_ODR_2;

            clock_out();
            clock_out();
        }
        else if(data[i] == 2){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
      }
        else if(data[i] == 3){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            clock_out();
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
      }
    }

    GPIOC->ODR |= GPIO_ODR_2;
    clock_out();

    if(data[1] == 0){
                GPIOC->ODR |= GPIO_ODR_2;
            }
    else{
                GPIOC->ODR &= ~GPIO_ODR_2;
            }
    clock_out();

    GPIOC->ODR |= GPIO_ODR_2;
    clock_out();
    clock_out();

    GPIOC->ODR |= GPIO_ODR_1;
}

void send_data_row4(int data[]){

    int i;
    GPIOC->ODR &= ~GPIO_ODR_1;


    for(i = 16; i < 20; i++){
        if(data[i] == 0){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            clock_out();
            clock_out();
        }
        else if(data[i] == 1){
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
            GPIOC->ODR &= ~GPIO_ODR_2;

            clock_out();
            clock_out();
        }
        else if(data[i] == 2){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
      }
        else if(data[i] == 3){
            GPIOC->ODR &= ~GPIO_ODR_2;
            clock_out();
            clock_out();
            GPIOC->ODR |= GPIO_ODR_2;
            clock_out();
      }
    }


    if(data[0] == 0){
                GPIOC->ODR |= GPIO_ODR_2;
            }
    else{
                GPIOC->ODR &= ~GPIO_ODR_2;
            }
    clock_out();

    GPIOC->ODR |= GPIO_ODR_2;
    clock_out();
    clock_out();
    clock_out();
    GPIOC->ODR |= GPIO_ODR_1;
}

void send_data_row1b(int data2[]){

    int i;
    GPIOB->ODR &= ~GPIO_ODR_6;

    for(i = 4; i < 8; i++){
           if(data2[i] == 0){
               GPIOB->ODR &= ~GPIO_ODR_7;
               clock_out2();
               clock_out2();
               clock_out2();
           }
           else if(data2[i] == 1){
               GPIOB->ODR |= GPIO_ODR_7;
               clock_out2();
               GPIOB->ODR &= ~GPIO_ODR_7;

               clock_out2();
               clock_out2();
           }
           else if(data2[i] == 2){
               GPIOB->ODR &= ~GPIO_ODR_7;
               clock_out2();
               GPIOB->ODR |= GPIO_ODR_7;
               clock_out2();
               GPIOB->ODR &= ~GPIO_ODR_7;
               clock_out2();
         }
           else if(data2[i] == 3){
               GPIOB->ODR &= ~GPIO_ODR_7;
               clock_out2();
               clock_out2();
               GPIOB->ODR |= GPIO_ODR_7;
               clock_out2();
         }
    }

    GPIOB->ODR |= GPIO_ODR_7;
    clock_out2();
    clock_out2();
    clock_out2();

    if(data2[3] == 0){
                GPIOB->ODR |= GPIO_ODR_7;
            }
    else{
                GPIOB->ODR &= ~GPIO_ODR_7;
            }
    clock_out2();

    GPIOB->ODR |= GPIO_ODR_6;
}

void send_data_row2b(int data2[]){
    GPIOB->ODR &= ~GPIO_ODR_6;
    int i;

    for(i = 8; i < 12; i++){
        if(data2[i] == 0){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            clock_out2();
            clock_out2();
        }
        else if(data2[i] == 1){
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR &= ~GPIO_ODR_7;

            clock_out2();
            clock_out2();
        }
        else if(data2[i] == 2){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
      }
        else if(data2[i] == 3){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            clock_out2();
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
      }
    }

    GPIOB->ODR |= GPIO_ODR_7;
    clock_out2();
    clock_out2();

    if(data2[2] == 0){
                GPIOB->ODR |= GPIO_ODR_7;
            }
    else{
                GPIOB->ODR &= ~GPIO_ODR_7;
            }
    clock_out2();

    GPIOB->ODR |= GPIO_ODR_7;
    clock_out2();
    GPIOB->ODR |= GPIO_ODR_6;
}

void send_data_row3b(int data2[]){

    int i;
    GPIOB->ODR &= ~GPIO_ODR_6;
    for(i = 12; i < 16; i++){
        if(data2[i] == 0){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            clock_out2();
            clock_out2();
        }
        else if(data2[i] == 1){
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR &= ~GPIO_ODR_7;

            clock_out2();
            clock_out2();
        }
        else if(data2[i] == 2){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
      }
        else if(data2[i] == 3){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            clock_out2();
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
      }
    }

    GPIOB->ODR |= GPIO_ODR_7;
    clock_out2();

    if(data2[1] == 0){
                GPIOB->ODR |= GPIO_ODR_7;
            }
    else{
                GPIOB->ODR &= ~GPIO_ODR_7;
            }
    clock_out2();

    GPIOB->ODR |= GPIO_ODR_7;
    clock_out2();
    clock_out2();

    GPIOB->ODR |= GPIO_ODR_6;
}

void send_data_row4b(int data2[]){

    int i;
    GPIOB->ODR &= ~GPIO_ODR_6;


    for(i = 16; i < 20; i++){
        if(data2[i] == 0){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            clock_out2();
            clock_out2();
        }
        else if(data2[i] == 1){
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR &= ~GPIO_ODR_7;

            clock_out2();
            clock_out2();
        }
        else if(data2[i] == 2){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
      }
        else if(data2[i] == 3){
            GPIOB->ODR &= ~GPIO_ODR_7;
            clock_out2();
            clock_out2();
            GPIOB->ODR |= GPIO_ODR_7;
            clock_out2();
      }
    }


    if(data2[0] == 0){
                GPIOB->ODR |= GPIO_ODR_7;
            }
    else{
                GPIOB->ODR &= ~GPIO_ODR_7;
            }
    clock_out2();

    GPIOB->ODR |= GPIO_ODR_7;
    clock_out2();
    clock_out2();
    clock_out2();
    GPIOB->ODR |= GPIO_ODR_6;
}

void reset(void){
    for(int i = 4; i < 20; i++){
        data[i] = 0;
    }
}

void reset2(void){
    for(int i = 4; i < 20; i++){
        data2[i] = 0;
    }
    for(int i = 4; i < 20; i++){
        data[i] = 0;
    }
}



void update_key_press() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if ((key_samples[i][j] & KEY_PRESS_MASK) == 0b00000111) {
                key_pressed[i][j] = 1;
                key_samples[i][j] = 0xFF;
            }

            if ((key_samples[i][j] & KEY_REL_MASK) == 0b11100000) {
                key_released[i][j] = 1;
                key_samples[i][j] = 0x00;
            }
        }
    }
}

void update_key_press2() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if ((key_samples2[i][j] & KEY_PRESS_MASK) == 0b00000111) {
                key_pressed2[i][j] = 1;
                key_samples2[i][j] = 0xFF;
            }

            if ((key_samples2[i][j] & KEY_REL_MASK) == 0b11100000) {
                key_released2[i][j] = 1;
                key_samples2[i][j] = 0x00;
            }
        }
    }
}

int get_key_pressed() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(key_released[i][j] == 1 && key_pressed[i][j] == 1) {
                key_released[i][j] = 0;
                key_pressed[i][j] = 0;
                return int_array[i][j];
            }
        }
    }

    return -1;
}

int get_key_pressed2() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(key_released2[i][j] == 1 && key_pressed2[i][j] == 1) {
                key_released2[i][j] = 0;
                key_pressed2[i][j] = 0;
                return int_array2[i][j];
            }
        }
    }

    return -1;
}

void update_samples(int row) {
    // Update the current column with new values
    for(int i = 0; i < 4; i++) {
        uint8_t curr_history = key_samples[i][col];
        curr_history = curr_history << 1;

        if(row == i)
            curr_history |= 1;

        key_samples[i][col] = curr_history;
    }
}

void update_samples2(int row) {
    // Update the current column with new values
    for(int i = 0; i < 4; i++) {
        uint8_t curr_history = key_samples2[i][col];
        curr_history = curr_history << 1;

        if(row == i)
            curr_history |= 1;

        key_samples2[i][col] = curr_history;
    }
}

// Use the global variable ‘col’ provided in the skeleton file,
// increment it by 1. Check if ‘col’ exceeds 3, if so, reset it to 0.
// This is because we have 4 columns on the keypad and ‘col’ dictates which
// column is currently being driven. Set the output data register
// of the port A to (1 << col). This drives the corresponding column to ‘logic 1’.
void TIM3_IRQHandler()
{
    //if(1){
    if(!dontCheck){
    int current_row = -1;

    int check = GPIOA->IDR;

    if((GPIOC->IDR & 16) != 0)
        current_row = 0;
    else if((check & 32) != 0)
        current_row = 1;
    else if((check & 64) != 0)
        current_row = 2;
    else if((check & 128) != 0)
        current_row = 3;

    update_samples(current_row);

    update_key_press();

    col++;
    if(col > 3)
        col = 0;

    //GPIOA->ODR = 1 << col;

    //other board
    int current_row2 = -1;

        int check2 = GPIOA->IDR;

        if((check2 & GPIO_IDR_12) != 0)
            current_row2 = 0;
        else if((check2 & GPIO_IDR_13) != 0)
            current_row2 = 1;
        else if((check2 & GPIO_IDR_14) != 0)
            current_row2 = 2;
        else if((check2 & GPIO_IDR_15) != 0)
            current_row2 = 3;

        update_samples2(current_row2);

        update_key_press2();

        col2++;
        if(col2 > 3)
            col2 = 0;

        GPIOA->ODR = (1 << (col2 + 8)) | (1 << col);
    }

    //Acknowledge interrupt
    TIM3->SR &= ~(1);
}

//PA8-15
void TIM2_IRQHandler(){

    clock();

    TIM2 -> SR &= ~(1<<0);
}


// This function should enable clock to timer 3, setup the timer
// so that it triggers TIM3_IRQHandler every 1ms.
void setup_timer3() {
    // Enable the system clock for timer 3
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

        //freq = 1MHZ = 48MHZ/48
        TIM3->PSC = 48 - 1;
        // Auto-reload 1000 so reset every 1ms
        TIM3->ARR = 1000 - 1;

        TIM3->DIER |= TIM_DIER_UIE;
        // Enable timer 3
        TIM3->CR1 |= TIM_CR1_CEN;

        //Enable Interupt
        NVIC->ISER[0] = 1 << TIM3_IRQn;
}

void init_tim2(void) {
    // Your code goes here.

    // enable clock to timer 2
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

    // trigger interrupt 10x per sec
    TIM2 -> PSC = 999;
    TIM2 -> ARR = 4799;

    TIM2 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM2_IRQn;
    //NVIC_SetPriority(TIM2_IRQn, 1);

    // start timer2
    TIM2 -> CR1 |= TIM_CR1_CEN;
}

// This function should enable the clock to port A, configure pins 0, 1, 2 and
// 3 as outputs (we will use these to drive the columns of the keypad).
// Configure pins 4, 5, 6 and 7 to have a pull down resistor
// (these four pins connected to the rows will being scanned
// to determine the row of a button press).
void init_keypad() {
    // Enable Port A
           RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
           RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
        // Set the mode for PA0 for output
        GPIOA->MODER &= ~(3<<0);
        GPIOA->MODER |= 1<<0;

        //PA1
        GPIOA->MODER &= ~(3<<2);
        GPIOA->MODER |= 1<<2;

        //PA2
        GPIOA->MODER &= ~(3<<4);
        GPIOA->MODER |= 1<<4;

        //PA3
        GPIOA->MODER &= ~(3<<6);
        GPIOA->MODER |= 1<<6;

        GPIOC->MODER &= ~(3<<8);
        GPIOA->MODER &= ~(3<<10);
        GPIOA->MODER &= ~(3<<12);
        GPIOA->MODER &= ~(3<<14);
        //PA4 Pull down cross this out
        //PC4
        //GPIOA->PUPDR &= ~(3<<8);
        //GPIOA->PUPDR |= 2<<8;
        GPIOC->PUPDR &= ~(3<<8);
        GPIOC->PUPDR |= 2<<8;

        //PA5 Pull down
        GPIOA->PUPDR &= ~(3<<10);
        GPIOA->PUPDR |= 2<<10;

        //PA6 Pull down
        GPIOA->PUPDR &= ~(3<<12);
        GPIOA->PUPDR |= 2<<12;

        //PA7 Pull down
        GPIOA->PUPDR &= ~(3<<14);
        GPIOA->PUPDR |= 2<<14;
}

void init_keypad2() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
     GPIOA->MODER &= ~(3<<16);
     GPIOA->MODER |= 1<<16;
     GPIOA->MODER &= ~(3<<18);
     GPIOA->MODER |= 1<<18;
     GPIOA->MODER &= ~(3<<20);
     GPIOA->MODER |= 1<<20;
     GPIOA->MODER &= ~(3<<22);
     GPIOA->MODER |= 1<<22;

     GPIOA->MODER &= ~(3<<24);
     GPIOA->MODER &= ~(3<<26);
     GPIOA->MODER &= ~(3<<28);
     GPIOA->MODER &= ~(3<<30);

     GPIOA->PUPDR &= ~(3<<24);
     GPIOA->PUPDR |= 2<<24;
     GPIOA->PUPDR &= ~(3<<26);
     GPIOA->PUPDR |= 2<<26;
     GPIOA->PUPDR &= ~(3<<28);
     GPIOA->PUPDR |= 2<<28;
     GPIOA->PUPDR &= ~(3<<30);
     GPIOA->PUPDR |= 2<<30;
}
/*
void dispShort(void){
    send_data_row1(data);
    send_data_row2(data);
    send_data_row3(data);
    send_data_row4(data);
}
*/
void dispShort2(void){
    send_data_row1b(data2);
    send_data_row2b(data2);
    send_data_row3b(data2);
    send_data_row4b(data2);
    send_data_row1(data);
    send_data_row2(data);
    send_data_row3(data);
    send_data_row4(data);
}

void toggle(int k){
    DAC -> CR &= ~DAC_CR_EN1;
    soundCheck = 1;
    soundCounter = 0;
    if(data[k+4])
        data[k+4] = 0;
    else
        data[k+4] = 3;

}

void toggle2(int k){
    DAC -> CR &= ~DAC_CR_EN1;
    soundCheck = 1;
    soundCounter = 0;
    if(data2[k+4])
        data2[k+4] = 0;
    else
        data2[k+4] = 3;

}

void prob3(void) {
    reset();
    //dispShort();
    dontCheck = 0;
    while(1) {
        dispShort();

        int k = get_key_pressed();
        if(k >= 0){

            toggle(k);
            int correct = 1;
            for(int i = 0; i < 20; i++){
                if(data[i] != easy1[i]){
                    correct = 0;
                    break;
                }
            }

            if(correct){
                dontCheck = 1;
                for(int i = 4; i < 20; i++){
                    data[i] = 2;
                }
                return;
            }
        }
    }
}

void prob3b(void) {
    char line1[20];
    reset2();
    //dispShort();
    dontCheck = 0;
    soundCheck = 0;
    soundCounter = 0;
    while(1) {
        dispShort2();

        if ((GPIOB -> IDR & GPIO_IDR_11) == GPIO_IDR_11){

            micro_wait(1000);
            if ((GPIOB -> IDR & GPIO_IDR_11) != GPIO_IDR_11){

                NVIC_SystemReset();

            }

        }

            int k = get_key_pressed2();
        int k2 = get_key_pressed();
        soundCounter++;
        if(soundCounter >= 200){
            soundCounter = 0;
            soundCheck = 0;
            DAC -> CR &= ~DAC_CR_EN1;
        }
        if((k >= 0) | (k2 >= 0)){

            if(k >= 0)
                toggle2(k);
            if(k2 >= 0)
                toggle(k2);

            int correct = 1;
            for(int i = 0; i < 20; i++){
                if((data2[i] != easy2[i]) | (data[i] != easy2a[i])){
                    correct = 0;
                    break;
                }
            }
            if(soundCheck == 1){
                makeSound();
            }
            if(correct){
                dontCheck = 1;
                soundCounter = 0;
                soundCheck = 0;

                TIM2 -> DIER &= ~(TIM_DIER_UIE);

                tenths = 0;
                seconds = 20;
                minutes = 0;
                hours = 0;

                portApin15Counter += 1;

                if(portApin15Counter <= 5){
                sprintf(line1, "Level %d - %s", portApin15Counter, mode);
                display1(line1);
                }

                //TIM2 -> DIER |= TIM_DIER_UIE;


                for(int i = 4; i < 20; i++){
                    data2[i] = 2;
                    data[i] = 2;
                }
                return;
            }
        }
    }
}

void disp(void){
    int counter = 0;
    while(counter < 5000){
        send_data_row1(data);
        send_data_row2(data);
        send_data_row3(data);
        send_data_row4(data);
        counter++;
    }
}

void disp2(void){
    int counter = 0;
    if (mode2){
        while(counter < 7000){
            send_data_row1b(data2);
            send_data_row2b(data2);
            send_data_row3b(data2);
            send_data_row4b(data2);
            send_data_row1(data);
            send_data_row2(data);
            send_data_row3(data);
            send_data_row4(data);
            counter++;
        }
    }
    else{
        while(counter < 4000){
            send_data_row1b(data2);
            send_data_row2b(data2);
            send_data_row3b(data2);
            send_data_row4b(data2);
            send_data_row1(data);
            send_data_row2(data);
            send_data_row3(data);
            send_data_row4(data);
            counter++;
        }
    }
}

void dispMed2(void){
    int counter = 0;

        while(counter < 1500){
            send_data_row1b(data2);
            send_data_row2b(data2);
            send_data_row3b(data2);
            send_data_row4b(data2);
            send_data_row1(data);
            send_data_row2(data);
            send_data_row3(data);
            send_data_row4(data);
            counter++;
        }

}

void dispOver(void){
    int counter = 0;

        while(counter < 8000){
            send_data_row1b(data2);
            send_data_row2b(data2);
            send_data_row3b(data2);
            send_data_row4b(data2);
            send_data_row1(data);
            send_data_row2(data);
            send_data_row3(data);
            send_data_row4(data);
            counter++;
        }

}
/*
void setupEasy(int x){
    int easy = x;
       for(int i = 0; i < easy; i++){
           int a = rand() % 16 + 4;
           data[a] = 3;
       }
}
*/
void setupEasy2(int x){
    int easy = x;
       for(int i = 0; i < easy; i++){
           int a = rand() % 16 + 4;
           int b = rand() % 2 + 1;
           if(b == 1){
               data2[a] = 3;
           }
           else if(b == 2){
               data[a] = 3;
           }
       }
}

void newLevelEasy2(int x){
    setupEasy2(x);
    dontCheck = 1;
    for(int i = 0; i < 20; i++){
        easy2[i] = data2[i];
        easy2a[i] = data[i];
    }
    disp2();
    reset2();
    dispShort2();
    TIM2 -> DIER |= (TIM_DIER_UIE);
    prob3b();
    //DAC -> CR |= DAC_CR_EN1;
    makeSound();
    dispMed2();
    reset2();
    dispShort2();
    DAC -> CR &= ~DAC_CR_EN1;
}

void newLevelEasy(int x){
    setupEasy(x);
    dontCheck = 1;
    for(int i = 0; i < 20; i++){
        easy1[i] = data[i];
    }
    disp();
    reset();
    dispShort();
    prob3();
    disp();
    reset();
    //disp();
}

void toggleHard(int k){
    DAC -> CR &= ~DAC_CR_EN1;
    soundCheck = 1;
    soundCounter = 0;
    if(data[k+4] == 3)
        data[k+4] = 0;
    else
        data[k+4] = data[k+4] + 1;

}

void toggleHard2(int k){
    DAC -> CR &= ~DAC_CR_EN1;
    soundCheck = 1;
    soundCounter = 0;
    //makeSound();

    if(data2[k+4] == 3)
        data2[k+4] = 0;
    else
        data2[k+4] = data2[k+4] + 1;

}

void hardPress(void) {
    reset();
    dontCheck = 0;
    while(1) {
        dispShort();

        int k = get_key_pressed();
        if(k >= 0){

            toggleHard(k);
            int correct = 1;
            for(int i = 0; i < 20; i++){
                if(data[i] != easy1[i]){
                    correct = 0;
                    break;
                }
            }

            if(correct){
                dontCheck = 1;
                for(int i = 4; i < 20; i++){
                    data[i] = 2;
                }
                return;
            }
        }
    }
}
/*
void prob3b(void) {
    reset2();
    //dispShort();
    dontCheck = 0;
    while(1) {
        dispShort2();

        int k = get_key_pressed2();
        int k2 = get_key_pressed();
        if((k >= 0) | (k2 >= 0)){

            if(k >= 0)
                toggle2(k);
            if(k2 >= 0)
                toggle(k2);
            int correct = 1;
            for(int i = 0; i < 20; i++){
                if((data2[i] != easy2[i]) | (data[i] != easy2a[i])){
                    correct = 0;
                    break;
                }
            }

            if(correct){
                dontCheck = 1;
                for(int i = 4; i < 20; i++){
                    data2[i] = 2;
                    data[i] = 2;
                }
                return;
            }
        }
    }
}
*/
/*
void prob3b(void) {
    reset2();
    //dispShort();
    dontCheck = 0;
    soundCheck = 0;
    soundCounter = 0;
    while(1) {
        dispShort2();

        int k = get_key_pressed2();
        int k2 = get_key_pressed();
        soundCounter++;
        if(soundCounter >= 200){
            soundCounter = 0;
            soundCheck = 0;
            DAC -> CR &= ~DAC_CR_EN1;
        }
        if((k >= 0) | (k2 >= 0)){

            if(k >= 0)
                toggle2(k);
            if(k2 >= 0)
                toggle(k2);

            int correct = 1;
            for(int i = 0; i < 20; i++){
                if((data2[i] != easy2[i]) | (data[i] != easy2a[i])){
                    correct = 0;
                    break;
                }
            }
            if(soundCheck == 1){
                makeSound();
            }
            if(correct){
                dontCheck = 1;
                soundCounter = 0;
                soundCheck = 0;


                for(int i = 4; i < 20; i++){
                    data2[i] = 2;
                    data[i] = 2;
                }
                return;
            }
        }
    }
}*/

void hardPress2(void) {
    char line1[20];
    reset2();
    dontCheck = 0;
    soundCheck = 0;
    soundCounter = 0;
    while(1) {
        dispShort2();

        if ((GPIOB -> IDR & GPIO_IDR_11) == GPIO_IDR_11){

            micro_wait(1000);
            if ((GPIOB -> IDR & GPIO_IDR_11) != GPIO_IDR_11){

                NVIC_SystemReset();

            }

        }

        int k = get_key_pressed2();
        int k2 = get_key_pressed();
        soundCounter++;
        if(soundCounter >= 200){
            soundCounter = 0;
            soundCheck = 0;
            DAC -> CR &= ~DAC_CR_EN1;
        }
        if((k >= 0) | (k2 >= 0)){
        //if(k >= 0){

            if(k >= 0)
                toggleHard2(k);
            if(k2 >= 0)
                toggleHard(k2);
            int correct = 1;
            for(int i = 0; i < 20; i++){
                if((data2[i] != easy2[i]) | (data[i] != easy2a[i])){
                //if(data2[i] != easy2[i]){
                    correct = 0;
                    break;
                }
            }
            if(soundCheck == 1){
                makeSound();
            }
            if(correct){
                //DAC -> CR &= ~DAC_CR_EN1;
                dontCheck = 1;
                soundCounter = 0;
                soundCheck = 0;

                TIM2 -> DIER &= ~(TIM_DIER_UIE);

               tenths = 0;
               seconds = 20;
               minutes = 0;
               hours = 0;

               portApin15Counter += 1;

               if(portApin15Counter <= 5){
               sprintf(line1, "Level %d - %s", portApin15Counter, mode);
               display1(line1);
               }

                for(int i = 4; i < 20; i++){
                    data2[i] = 2;
                    data[i] = 2;
                }
                return;
            }
        }
    }
}
/*
void setupHard(int x){
    int easy = x;
       for(int i = 0; i < easy; i++){
           int a = rand() % 16 + 4;
           int b = rand() % 3 + 1;
           data[a] = b;
       }
}
*/
void setupHard2(int x){
    int easy = x;
       for(int i = 0; i < easy; i++){
           int a = rand() % 16 + 4;
           int b = rand() % 3 + 1;
           int c = rand() % 2;
           //int b = rand() % 2 + 1;
            if(c == 1){
              data2[a] = b;
            }
            else{
              data[a] = b;
            }
           //data2[a] = b;
       }
}
/*
void newLevelHard(int x){
    setupHard(x);
    dontCheck = 1;
    for(int i = 0; i < 20; i++){
        easy1[i] = data[i];
    }
    disp();
    reset();
    dispShort();
    hardPress();
    disp();
    reset();
    //disp();
}
*/


void newLevelHard2(int x){
    setupHard2(x);
    dontCheck = 1;
    for(int i = 0; i < 20; i++){
        easy2[i] = data2[i];
        easy2a[i] = data[i];
    }
    disp2();
    reset2();
    dispShort2();
    TIM2 -> DIER |= (TIM_DIER_UIE);
    hardPress2();
    makeSound();
    dispMed2();
    //DAC -> CR &= ~DAC_CR_EN1;

    reset2();
    dispShort2();
    //disp();
    DAC -> CR &= ~DAC_CR_EN1;
}


int gameStart(void){
    int count = 0;
    data[7] = 2;
    data[11] = 1;
    int mode = -1;
    while(data[7] == 2 && data[11] == 1){
        dispShort();

        int k = get_key_pressed();
        if(k == 3){
            data[7] = 0;
            mode = 0;
        }
        else if(k == 7){
            data[11] = 0;
            mode = 1;
        }

        count++;
    }
    reset();
    srand(count);

    return mode;
}

int gameStart2(void){
    int count = 0;
    data2[7] = 2;
    data2[11] = 1;
    int mode = -1;
    while(data2[7] == 2 && data2[11] == 1){
        dispShort2();

        int k = get_key_pressed2();
        if(k == 3){
            data2[7] = 0;
            mode = 0;
        }
        else if(k == 7){
            data2[11] = 0;
            mode = 1;
        }

        count++;
    }
    reset2();
    srand(count);

    return mode;
}

void easyMode(void){
    newLevelEasy(2);
    newLevelEasy(3);
    newLevelEasy(4);
    newLevelEasy(6);
    newLevelEasy(7);
}

void easyMode2(void){

    countdown();
    newLevelEasy2(4);
    countdown();
    newLevelEasy2(6);
    countdown();
    newLevelEasy2(8);
    countdown();
    newLevelEasy2(11);
    countdown();
    newLevelEasy2(13);
    /*
    countdown();
     newLevelEasy2(1);
     countdown();
     newLevelEasy2(1);
     countdown();
     newLevelEasy2(1);
     countdown();
     newLevelEasy2(1);
     countdown();
     newLevelEasy2(1);
*/
     //NVIC_SystemReset;
}

void hardMode(void){
    newLevelHard(2);
    newLevelHard(3);
    newLevelHard(4);
    newLevelHard(6);
    newLevelHard(7);
}

void hardMode2(void){
    //development
    /*
    countdown();
    newLevelHard2(1);
    countdown();
    newLevelHard2(1);
    countdown();
    newLevelHard2(1);
    countdown();
    newLevelHard2(1);
    countdown();
    newLevelHard2(1);*/

    //production
    countdown();
    newLevelHard2(4);
    countdown();
    newLevelHard2(5);
    countdown();
    newLevelHard2(7);
    countdown();
    newLevelHard2(8);
    countdown();
    newLevelHard2(10);
}

void step6(void) {
    // Configure the function pointers.
    char line1[20];
    char line2[20];
    cmd = spi_cmd;
    data3 = spi_data;
    display1 = circdma_display1;
    display2 = circdma_display2;
    // Initialize the display.
    dma_spi_init_lcd();
    // Initialize timer 2.
    init_tim2();
    setup_tim6();
    //setup_timer3();
    make_wavetable();
    setup_dac();
    DAC -> CR &= ~DAC_CR_EN1;
    //init_keypad();

    // code for button interrupt
    // use cmd to increment number

    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    //GPIOA -> MODER &= ~GPIO_MODER_MODER15;
    //GPIOA -> PUPDR &= ~GPIO_PUPDR_PUPDR15;
    GPIOB -> MODER &= ~GPIO_MODER_MODER11;
    GPIOB -> PUPDR &= ~GPIO_PUPDR_PUPDR11;

    //SYSCFG -> EXTICR[3] = SYSCFG_EXTICR2_EXTI5_PA;

    //EXTI -> IMR |= EXTI_IMR_MR15;
    //EXTI -> RTSR |= EXTI_RTSR_TR15;
    //EXTI -> IMR |= EXTI_IMR_MR11;
    //EXTI -> RTSR |= EXTI_RTSR_TR11;

    //NVIC_EnableIRQ(EXTI4_15_IRQn);
    //NVIC_SetPriority(EXTI4_15_IRQn, 0);

    TIM2 -> DIER &= ~(TIM_DIER_UIE);


    if ((RCC -> CSR & RCC_CSR_PINRSTF) == RCC_CSR_PINRSTF){

        //sprintf(line1, "Level %d - %s", portApin15Counter, mode);
        sprintf(line1, "Green: Easy");
        display1(line1);
        sprintf(line2, "Red: Hard");
        display2(line2);
    }
/*
        while(1){

            if ((GPIOA -> IDR & GPIO_IDR_15) == GPIO_IDR_15){

                micro_wait(50000);
                if ((GPIOA -> IDR & GPIO_IDR_15) != GPIO_IDR_15){

                    if ((EXTI -> PR & EXTI_PR_PR15) == EXTI_PR_PR15){
                    break;
                }

           }

          }
       }*/    // <-----------  *REWRITE THIS* getting rid of this messes with GAME OVER number
  //  }

    //EXTI -> IMR &= ~EXTI_IMR_MR15;
    //EXTI -> RTSR &= ~EXTI_RTSR_TR15;
    //EXTI -> IMR &= ~EXTI_IMR_MR11;
    //EXTI -> RTSR &= ~EXTI_RTSR_TR11;

    //EXTI -> IMR |= EXTI_IMR_MR15;
    //EXTI -> RTSR |= EXTI_RTSR_TR15;
    //EXTI -> IMR |= EXTI_IMR_MR11;
    //EXTI -> RTSR |= EXTI_RTSR_TR11;

/*
    int i, duration;
    int current_row = -1;
    int current_col = -1;
    int testRow;
    int testCol;
    int z = 0; // int i
    int a;
    int pin1;
    int pin2;
    int y;
  */
/*
    for(;;){

        if ((GPIOA -> IDR & GPIO_IDR_15) == GPIO_IDR_15){

            micro_wait(10000);
            if ((GPIOA -> IDR & GPIO_IDR_15) != GPIO_IDR_15){

                    if (portApin15Counter == 6){
                        portApin15Counter = 5;
                    }

                    char line1[20];
                    sprintf(line1, "Level %d - %s", portApin15Counter, mode);
                    display1(line1);

                    if (portApin15Counter > 5){
                        portApin15Counter = 5;
                    }
                    else{
                        portApin15Counter += 1;
                    }

                    TIM2 -> DIER &= ~(TIM_DIER_UIE);
                    countdownFlag = 1;
                    countdown();
                    TIM2 -> DIER |= (TIM_DIER_UIE);

                    tenths = 0;
                    seconds = 5;
                    minutes = 0;
                    hours = 0;

                DAC -> CR &= ~DAC_CR_EN1;
            }
    }

    if((GPIOA -> IDR & GPIO_IDR_11) == GPIO_IDR_11){

        micro_wait(10000);
       if ((GPIOA -> IDR & GPIO_IDR_11) != GPIO_IDR_11){

            portApin15Counter -= 1;

            if (portApin15Counter == 0){
                NVIC_SystemReset();
            }

            if (modeNum == 0){
                modeNum = 1;
                strcpy(mode, "Hard");
                char line1[20];
                sprintf(line1, "Level %d - %s", portApin15Counter, mode);
                display1(line1);
            }
            else if (modeNum == 1){
                modeNum = 0;
                strcpy(mode, "Easy");
                char line1[20];
                sprintf(line1, "Level %d - %s", portApin15Counter, mode);
                display1(line1);
            }

            portApin15Counter += 1;

            TIM2 -> DIER &= ~(TIM_DIER_UIE);
            countdownFlag = 1;
            countdown();
            TIM2 -> DIER |= (TIM_DIER_UIE);

            tenths = 0;
            seconds = 5;
            minutes = 0;
            hours = 0;

            DAC -> CR &= ~DAC_CR_EN1;

        }

    }

    if ((GPIOF -> MODER & (1 << 14)) == (1 << 14)){

        gameoverFlag = 0;
        GPIOF -> MODER &= ~(3 << 14);
        loser();
    }

    testRow = GPIOA -> IDR & (0xfff);
    testRow >>= 5;

    testCol = GPIOA -> IDR & (0xf);

    for (z = 0; z < 4; z++)
    {

        pin1 = testRow & 1;

        if (pin1 == 1)
        {
            current_row = z;

            DAC -> CR |= DAC_CR_EN1;
            freq = buttonPress[z];
            duration = pressbpm[z];
            micro_wait(duration * 50000);
            DAC -> CR &= ~DAC_CR_EN1;
        }

        testRow >>= 1;

    }

    update_samples(current_row);

    update_key_press();

    col += 1;

    if (col > 3)
    {
        col = 0;
    }

    GPIOA -> ODR = (1 << col);

    }
    */
}

void countdown(void){
    char line2a[20];
    char line1a[20];
    int num = 3;
    int i, duration;

    sprintf(line1a, "Level %d - %s", portApin15Counter, mode);
    display1(line1a);

    for (i = 0; i < 4; i++){

        if (num == 0){
            sprintf(line2a, "START");
        }
        else{
            sprintf(line2a, "%d", num);
        }

         DAC -> CR |= DAC_CR_EN1;
         freq = step1[i];
         duration = bpm[i];
         display2(line2a);
         micro_wait(duration * 500000);
         DAC -> CR &= ~DAC_CR_EN1;
         display2(" ");
         if (i != 3){
             micro_wait(duration * 500000);
         }
        num--;
    }
}

void loser(void){

    int i, duration;
    char line1[20];
    char line2[20];

    for (i = 0; i < 2; i++){
        DAC -> CR |= DAC_CR_EN1;
        freq = lose[i];
        duration = losebpm[i];
        micro_wait(duration * 1000000);
        DAC -> CR &= ~DAC_CR_EN1;
    }

    micro_wait(2000000);

    portApin15Counter = 1;
    strcpy(mode, "Easy");
    sprintf(line1, "Level %d - %s", portApin15Counter, mode);
    display1(line1);
    sprintf(line2, " ");
    display2(line2);

}

/*
void EXTI4_15_IRQHandler(void){
    if (EXTI -> PR & EXTI_PR_PR15){
        if ((GPIOA -> IDR & GPIO_IDR_15) == GPIO_IDR_15){

                EXTI -> PR |= EXTI_PR_PR15;

        }
        else{
            micro_wait(100000);
        }

    }
    if (EXTI -> PR & EXTI_PR_PR11){
        if ((GPIOA -> IDR & GPIO_IDR_11) == GPIO_IDR_11){


                EXTI -> PR |= EXTI_PR_PR11;
          }

        }
        else{
            micro_wait(100000);
        }
    }
*/

void clock(void) {

    char line1[20];
    char line2[20];
    tenths -= 1;

    if (tenths < 0){
        tenths = 9;
        seconds -= 1;
    }
    if (seconds < 0){
        seconds = 59;
        minutes -= 1;
    }
    if (minutes < 0){
        minutes = 59;
        hours -= 1;
    }

    sprintf(line2, "%02d:%02d:%02d.%d", hours, minutes, seconds, tenths);
    display2(line2);

    if ((tenths == 0) && (seconds == 0) && (minutes == 0) && (hours == 0)){

        RCC -> AHBENR |= RCC_AHBENR_GPIOFEN;
        GPIOF -> MODER &= ~(3 << 14);
        GPIOF -> MODER |= (1 << 14);

        TIM2 -> DIER &= ~(TIM_DIER_UIE);

        sprintf(line1, "GAME OVER");
        display1(line1);

        //portApin15Counter -= 1;

        sprintf(line2, "Level %d - %s", portApin15Counter, mode);
        display2(line2);

        for(int i = 4; i < 20; i++){
            data[i] = 1;
            data2[i] = 1;
        }
        dispOver();

        //portApin15Counter += 1;
        NVIC_SystemReset();
    }

}

void TIM6_DAC_IRQHandler(void){

    TIM6 -> SR &= ~(1 << 0);

    int sample = 0;
    offset1 += freq;

    DAC -> SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

    if ((offset1>>16) >= (sizeof wavetable / sizeof wavetable[0])){

        offset1 -= ((sizeof wavetable / sizeof wavetable[0])<<16);
    }

    sample = (wavetable[offset1>>16]);
    sample = sample / 16 + 2048;

    DAC -> DHR12R1 = sample;
}

void spi_cmd(char b) {
    // Your code goes here.
    while ((SPI2 -> SR & SPI_SR_TXE) == 0){

        ;
    }

    SPI2 -> DR = b;

}

void spi_data(char b) {
    // Your code goes here.
    while ((SPI2 -> SR & SPI_SR_TXE) == 0){

        ;
    }

    SPI2 -> DR = 0x200 + b;

}

void circdma_display2(const char *s) {
    // Your code goes here.

    //cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            dispmem[x+18] = s[x] | 0x200;
            //data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        dispmem[x+18] = 0x220;
        //data(' ');
}

void circdma_display1(const char *s) {
    // Your code goes here.

    //cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            dispmem[x+1] = s[x] | 0x200;
            //data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        dispmem[x+1] = 0x220;
        //data(' ');

}

void dma_spi_init_lcd(void) {
    // Your code goes here.

    spi_init_lcd();

    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel5 -> CCR &= ~(DMA_CCR_EN);

    // copy from address in CMAR
    DMA1_Channel5 -> CMAR = (uint32_t) (dispmem);

    // copy to address in CPAR
    DMA1_Channel5 -> CPAR = (uint32_t) (&(SPI2 -> DR));

    // set peripheral and mem transf. size to 16 bit
    DMA1_Channel5 -> CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel5 -> CCR |= DMA_CCR_MSIZE_0;

    // transfer 17 16-bit values
    DMA1_Channel5 -> CNDTR = (sizeof(char)) * 34;

    // set drxn as mem to periph
    DMA1_Channel5 -> CCR |= DMA_CCR_DIR;

    // increment mem addr. after each transfer
    DMA1_Channel5 -> CCR |= DMA_CCR_MINC;

    // set priority to low
    DMA1_Channel5 -> CCR &= ~(DMA_CCR_PL);

    // modify spi ch2 so DMA request made when buffer is empty
    SPI2 -> CR2 |= SPI_CR2_TXDMAEN;

    // choose circular mode
    DMA1_Channel5 -> CCR |= DMA_CCR_CIRC;

    // enable DMA channel
    DMA1_Channel5 -> CCR |= DMA_CCR_EN;


}

void spi_init_lcd(void) {
    // Your code goes here.

    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;

    // configure pb12 (SS), pb13 (SCK), pb15 (MOSI) as alt fxns
    // for SPI2

    GPIOB -> MODER &= ~(3<<24);
    GPIOB -> MODER &= ~(3<<26);
    GPIOB -> MODER &= ~(3<<30);

    GPIOB -> MODER |= 2<<24;
    GPIOB -> MODER |= 2<<26;
    GPIOB -> MODER |= 2<<30;

    // use alternate function 0
    GPIOB -> AFR[1] &= ~((0xf)<<16);
    GPIOB -> AFR[1] &= ~((0xf)<<20);
    GPIOB -> AFR[1] &= ~((0xf)<<28);

    // use bidrxnal mode, bidrxnal OE (set master?)
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2 -> CR1 |= SPI_CR1_BIDIMODE;
    SPI2 -> CR1 |= SPI_CR1_BIDIOE;

    SPI2 -> CR1 |= SPI_CR1_MSTR;

    // use slowest baud rate
    SPI2 -> CR1 |= SPI_CR1_BR;

    // BR value at which display still works
    //SPI2 -> CR1 |= (SPI_CR1_BR_1 | SPI_CR1_BR_0);

    // set clock = 0 when idle
    SPI2 -> CR1 &= ~(SPI_CR1_CPOL);

    // use 1st clock trans as capture edge
    SPI2 -> CR1 &= ~(SPI_CR1_CPHA);

    // use 10 bit word size
    SPI2 -> CR2 = (SPI_CR2_DS_0 | SPI_CR2_DS_3);

    // use slave select OE
    SPI2 -> CR2 |= SPI_CR2_SSOE;

    // set auto NSS gen
    SPI2 -> CR2 |= SPI_CR2_NSSP;

    // enable SPI2 with SPE bit
    SPI2 -> CR1 |= SPI_CR1_SPE;

    generic_lcd_startup();
}

void generic_lcd_startup(void) {
    nano_wait(100000000); // Give it 100ms to initialize
    cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
    cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
    cmd(0x01);  // clear entire display
    nano_wait(6200000); // clear takes 6.2ms to complete
    cmd(0x02);  // put the cursor in the home position
    cmd(0x06);  // 0000 01IS: set display to increment
}

/*
void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}
*/

void setup_dac(void){
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;

    GPIOA -> MODER &= ~(3 << 8);
    GPIOA -> MODER |= (3 << 8);

    DAC -> CR |= (DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0);
    DAC -> CR |= DAC_CR_TEN1;

    DAC -> CR |= DAC_CR_EN1;
}

void make_wavetable(void){
    int x;
    for (x = 0; x < sizeof wavetable / sizeof wavetable[0]; x += 1){

        wavetable[x] = 32767 * sin(x * 2 * M_PI / 256);

    }
}

void setup_tim6(void){

    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;

    TIM6 -> ARR = 1;
    TIM6 -> PSC = 374;

    TIM6 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] |= 1 << TIM6_DAC_IRQn;

    TIM6 -> CR1 |= TIM_CR1_CEN;
}

void makeSound(void){

    //int sound[] = {3.5 * (1<<16)};
    //int soundbpm[] = {1};
    //int i, duration;
    //for (i = 0; i < 1; i++){
        DAC -> CR |= DAC_CR_EN1;
        freq = sound[0];
        //duration = soundbpm[i];
        //micro_wait(duration * 1000000);
        //DAC -> CR &= ~DAC_CR_EN1;
    //}

}

//void countdown(void){

//}
/*
void EXTI4_15_IRQHandler(void){

    if (EXTI -> PR & EXTI_PR_PR11){
        if((GPIOB -> IDR & GPIO_IDR_11) == GPIO_IDR_11){

            micro_wait(10000);
            if ((GPIOB -> IDR & GPIO_IDR_11) != GPIO_IDR_11){

                NVIC_SystemReset();

                EXTI -> PR |= EXTI_PR_PR11;

            }
        }
    }

}
*/
int main(void)
{
    char line1[20];
    char line2[20];
    init_keypad();
    setup_timer3();

    init_keypad2();
    //setup_timer2();

    setupPorts();
    step6();


   /* col = 0;
    row = -1;
    col2 = 0;
    row2 = -1;
    dontCheck = 0;
    mode2 = 0;
    soundCheck = 0;
    soundCounter = 0;*/
    //int mode = gameStart();
    mode2 = gameStart2();

    /*
    if(mode == 0)
        easyMode();
    else
        hardMode();
*/

    if(mode2 == 0){
        easyMode2();
    }
    else{
        strcpy(mode, "Hard");
        hardMode2();
    }

    //reset();
    //disp();

    reset2();
    dispShort2();

    sprintf(line1, "YOU WIN!");
    display1(line1);
    sprintf(line2, " ");
    display2(line2);

    micro_wait(3000000);

    NVIC_SystemReset();
}















