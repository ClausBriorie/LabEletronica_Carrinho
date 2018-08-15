/*
Programa que ativa e desativa saidas PWM de uma KL25Z
a partir de comandos enviados de um computador
*/

#include "mbed.h"

Serial pc(USBTX, USBRX); // tx, rx
DigitalOut led_green(LED1);
DigitalOut led_red(LED2);

#define TRANSFER_SIZE  2 // Tamanho do vetor de caracteres transferidos entre os transceivers

// TODO: Verificar se as direções estão corretas; Trocar se necessário
PwmOut motor_A_1(PTB0);
PwmOut motor_A_2(PTB1);

// TODO: Test me
void setup_PWMs(PwmOut motor_A_1, PwmOut motor_A_2) {
    motor_A_1.period_ms(20); motor_A_1 = 0.0;
    motor_A_2.period_ms(20); motor_A_2 = 0.0;
}

// TODO: Test me
void acionar_motor(PwmOut motor_A_1, PwmOut motor_A_2) {
    pc.printf("acionar_motor() chamada\n");
    //motor_A_1 = 0.40; motor_A_2 = 0.0;
    //wait(0.04);
    motor_A_1 = 0.3; motor_A_2 = 0.0;
}

// TODO: Test me
void frear_motor(PwmOut motor_A_1, PwmOut motor_A_2) {
    pc.printf("frear_motor() chamada\n");
    motor_A_1 = 0.3; motor_A_2 = 0.3;
}

int main() {
    char Data[TRANSFER_SIZE];
    int DataCnt = 0;

    // LEDs iniciam desligados
    led_green = 1;
    led_red = 1;

    setup_PWMs(motor_A_1, motor_A_2);

    while (1) {
        if (pc.readable()) {
            Data[DataCnt++] = pc.getc();

            // Acionamento do motor A:
            if (Data[0] == 'a') {
                acionar_motor(motor_A_1, motor_A_2);
                wait(0.5);
                frear_motor(motor_A_1, motor_A_2);
                wait(0.1);
            }

            for (int i = 0; DataCnt > 0; DataCnt--, i++ ) {
                pc.putc(Data[i]);
            }

            if (DataCnt >= sizeof(Data)) {
                DataCnt = 0;
            }
        }
    }
}
