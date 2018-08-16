/*
Programa que coordena os dois motores
a partir de comandos enviados de um computador
*/
#include "mbed.h"
#define FRENTE 1
#define RE -1
#define ESQ 4
#define DIR 5
#define RAPID 0.5
#define MEDIO 0.35
#define DEVAG 0.2
#define TAMANHO_MSG  2

Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led_green(LED1);
DigitalOut led_red(LED2);

PwmOut motor_E_1(PTB0);
PwmOut motor_E_2(PTB1);
PwmOut motor_D_1(PTB2);
PwmOut motor_D_2(PTB3);

void setup_PWMs() {
    motor_E_1.period_ms(20); motor_E_1 = 0.0;
    motor_E_2.period_ms(20); motor_E_2 = 0.0;
    motor_D_1.period_ms(20); motor_D_1 = 0.0;
    motor_D_2.period_ms(20); motor_D_2 = 0.0;
}

void acionar_motor(int id_motor, int sentido, float vel) {
    if (id_motor == ESQ) {
        if (sentido == RE) {
            motor_E_1 = vel; motor_E_2 = 0.0;
        }
        else if (sentido == FRENTE) {
            motor_E_1 = 0.0; motor_E_2 = vel;
        }
    }
    if (id_motor == DIR) {
        if (sentido == RE) {
            motor_D_1 = vel; motor_D_2 = 0.0;
        }
        else if (sentido == FRENTE) {
            motor_D_1 = 0.0; motor_D_2 = vel;
        }
    }
}

void frear_motor(int id_motor) {
    if (id_motor == ESQ) {
        motor_E_1 = 0.3; motor_E_2 = 0.3;
    }
    else if (id_motor == DIR) {
        motor_D_1 = 0.3; motor_D_2 = 0.3;
    }
}

void interpretar_msg(char msg[]) {
    // Acionamento do motor esquerdo para a frente
    if (msg[0] == 'a') {
        acionar_motor(ESQ, FRENTE, MEDIO);
        wait(0.5);
        frear_motor(ESQ);
        wait(0.1);
    }

    // Acionamento dos dois motores para frente
    else if (msg[0] == 'w') {
        acionar_motor(ESQ, FRENTE, MEDIO); acionar_motor(DIR, FRENTE, MEDIO);
        wait(0.5);
        frear_motor(ESQ)                 ; frear_motor(DIR);
        wait(0.1);
    }

    // Acionamento dos dois motores para trás
    else if (msg[0] == 's') {
        acionar_motor(ESQ, RE, RAPID); acionar_motor(DIR, RE, RAPID);
        wait(0.5);
        frear_motor(ESQ)             ; frear_motor(DIR);
        wait(0.1);
    }

    // Acionamento do motor direito para frente
    else if (msg[0] == 'd') {
        acionar_motor(DIR, FRENTE, MEDIO);
        wait(0.5);
        frear_motor(DIR);
        wait(0.1);
    }
}

int main() {
    char msg[TAMANHO_MSG];
    int counter_msg = 0;

    // LEDs iniciam desligados
    led_green = 1;
    led_red = 1;

    setup_PWMs();

    while (1) {
        if (pc.readable()) {
            msg[counter_msg++] = pc.getc();
            interpretar_msg(msg);
            for (int i = 0; counter_msg > 0; counter_msg--, i++ ) {
                pc.putc(msg[i]);
            }

            if (counter_msg >= sizeof(msg)) {
                counter_msg = 0;
            }
        }
    }
}
