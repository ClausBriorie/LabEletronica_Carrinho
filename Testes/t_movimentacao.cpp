/*
Programa que ativa e desativa saidas PWM de uma KL25Z usando o transceiver,
a partir de comandos enviados de uma segunda KL25Z conectada a um computador
*/
#include "mbed.h"
#include "nRF24L01P.h"
#define TRANSFER_SIZE  2
#define FRENTE 1
#define RE -1
#define ESQ 4
#define DIR 5

DigitalOut led_green(LED1);
DigitalOut led_red(LED2);

PwmOut motor_E_1(PTB0);
PwmOut motor_E_2(PTB1);
PwmOut motor_D_1(PTB2);
PwmOut motor_D_2(PTB3);

// TODO: Test me
// se necessário, usar argumentos: PwmOut motor_E_1, PwmOut motor_E_2, PwmOut motor_D_1, PwmOut motor_D_2
void setup_PWMs() {
    motor_E_1.period_ms(20); motor_E_1 = 0.0;
    motor_E_2.period_ms(20); motor_E_2 = 0.0;
    motor_D_1.period_ms(20); motor_D_1 = 0.0;
    motor_D_2.period_ms(20); motor_D_2 = 0.0;
}

// TODO: Test me
void print_no_PC(char msg[TRANSFER_SIZE], int msgCnt) {
    for (int i = 0; msgCnt > 0; msgCnt--, i++) {
        pc.putc(msg[i]);
    }
}

// TODO: Test me
void interpretar_msg(char msg[TRANSFER_SIZE]) {
    // Açao 'a'
    if ((msg[0] == 'a')) {
        led_green = 0;
        wait(0.5);
        led_green = 1;
    }
    // Açao 'b'
    else if (msg[0] == 'b') {
        led_red = 0;
        wait(0.5);
        led_red = 1;
    }
    // Açao 'c'
    else if (msg[0] == 'c') {
        led_green = 0;
        led_red = 0;
        wait(0.5);
        led_green = 1;
        led_red = 1;
    }
}

void girar_roda(int id_motor, int sentido, float vel) {
    if (id_motor == ESQ) {
        if (sentido == FRENTE) {
            motor_E_1 = vel;
            motor_E_2 = 0.0;
        }
        else if (sentido == RE) {
            motor_E_1 = 0.0;
            motor_E_2 = vel;
        }
    }
    if (id_motor == DIR) {
        if (sentido == FRENTE) {
            motor_D_1 = vel;
            motor_D_2 = 0.0;
        }
        else if (sentido == RE) {
            motor_D_1 = 0.0;
            motor_D_2 = vel;
        }
    }
}

void frear_roda(int id_motor) {
    if (id_motor == ESQ) {
        motor_E_1 = 0.3;
        motor_E_2 = 0.3;
    }
    if (id_motor == DIR) {
        motor_D_1 = 0.3;
        motor_D_2 = 0.3;
    }
}

// TODO: implement me
/*
void manter_vel_alvo(int id_motor, float vel_alvo) {

}

void frear_roda(int id_motor) {

}
*/

int main() {
    // LEDs iniciam desligados
    led_green = 1;
    led_red = 1;

    char msg[TRANSFER_SIZE];
    int msgCnt = 0;

    setup_PWMs(motor_E_1, motor_E_2, motor_D_1, motor_D_2);

    while (1) {
        if (pc.readable()) {
        // le dado e transfere para buffer tx
        msg[msgCnt++] = pc.getc();
        }
        ////// CONTINUAR A PARTIR DAQUI!!!
    }
}
