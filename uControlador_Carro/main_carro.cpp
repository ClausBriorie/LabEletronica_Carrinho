#include "mbed.h"
#include "nRF24L01P.h"
#define TAMANHO_MSG  2
#define FRENTE 1
#define RE -1
#define ESQ 4
#define DIR 5
#define RAPID 0.5
#define MEDIO 0.35
#define DEVAG 0.2
#define TAMANHO_MSG  2

nRF24L01P transceiver(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq

Serial pc(USBTX, USBRX); // tx, rx

// Controle de motores
PwmOut motor_E_1(PTB0);
PwmOut motor_E_2(PTB1);
PwmOut motor_D_1(PTB2);
PwmOut motor_D_2(PTB3);

// Encoders
Timer t_esq;
InterruptIn encoder_esq(PTD7);
volatile int furos_esq = 0;

int t_anterior = 0;

/* ----- FUNÇÕES DE MOTORES E MOVIMENTAÇÃO ----- */
void setup_motores() {
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

/* ----- FUNÇÕES DE ENCODERS ----- */
void cont_furos_esq() {
    encoder_esq.disable_irq();
    t_esq.stop();
    int t_atual;
    int delta_t;
    int limiar_debouncing = 10; // milissegundos

    t_atual = t_esq.read_ms();
    delta_t = t_atual - t_anterior;

    furos_esq++;
    // if (delta_t > limiar_debouncing) {
    //
    // }
    t_esq.reset();
    t_esq.start();
    encoder_esq.enable_irq();
}

/* ----- FUNÇÕES DE COMUNICAÇÃO ----- */
void interpretar_msg(char msg[]) {
    // Acionamento do motor esquerdo para a frente
    if (msg[0] == 'a') {
        acionar_motor(ESQ, FRENTE, DEVAG);
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

// TODO: verificar se argumento é necessário
void setup_transceiver(nRF24L01P transceiver) {
    transceiver.powerUp();
    transceiver.setRxAddress(0xE5E5E5E5E5);
    transceiver.setRfFrequency(2448);
    transceiver.setTxAddress(0xE5E5E5E5E5);
    transceiver.setTransferSize(TAMANHO_MSG);
    transceiver.setReceiveMode();
    transceiver.enable();
}

int main() {
    char rxData[TAMANHO_MSG];
    int rxDataCnt = 0;

    setup_transceiver(transceiver);
    setup_motores();

    // Inicializando timers para interrupções
    t_esq.start();
    encoder_esq.enable_irq();
    encoder_esq.rise(&cont_furos_esq);

    while (1) {
        pc.printf("furos: %d\n", furos_esq);
        if (transceiver.readable()) {
            encoder_esq.rise(&cont_furos_esq);

            // le dado e transfere para buffer rx
            rxDataCnt = transceiver.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));
            interpretar_msg(rxData);
        }
    }
}
