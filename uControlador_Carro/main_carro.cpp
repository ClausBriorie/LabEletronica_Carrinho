#include "mbed.h"
#include "nRF24L01P.h"
#define TAMANHO_MSG  2
#define FRENTE 1
#define RE -1
#define ESQ 4
#define DIR 5
#define RAPID 0.6
#define MEDIO 0.45
#define DEVAG 0.3
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
Timer t_dir;
Timer t_generico;
InterruptIn encoder_esq(PTA12);
InterruptIn encoder_dir(PTD4);
volatile int furos_esq = 0;
volatile int furos_dir = 0;

/* ----- FUNÇÕES DE MOTORES E MOVIMENTAÇÃO ----- */
void setup_motores() {
    motor_E_1.period_ms(20); motor_E_1 = 0.0;
    motor_E_2.period_ms(20); motor_E_2 = 0.0;
    motor_D_1.period_ms(20); motor_D_1 = 0.0;
    motor_D_2.period_ms(20); motor_D_2 = 0.0;
}

void acionar_motor(int id_motor, int sentido, float vel) {
    float partida = 0.2;
    if (id_motor == ESQ) {
        if (sentido == RE) {
            motor_E_1 = vel + partida; motor_E_2 = 0.0;
            wait(0.05);
            motor_E_1 = vel; motor_E_2 = 0.0;
        }
        else if (sentido == FRENTE) {
            motor_E_1 = 0.0; motor_E_2 = vel + partida;
            wait(0.05);
            motor_E_1 = 0.0; motor_E_2 = vel;
        }
    }
    if (id_motor == DIR) {
        vel = vel*1.1; // motor direito é ligeiramente mais lento
        if (sentido == RE) {
            motor_D_1 = vel + partida; motor_D_2 = 0.0;
            wait(0.05);
            motor_D_1 = vel; motor_D_2 = 0.0;
        }
        else if (sentido == FRENTE) {
            motor_D_1 = 0.0; motor_D_2 = vel + partida;
            wait(0.05);
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
    int tempo;
    t_esq.stop();
    tempo = t_esq.read_ms();
    if (tempo > 10) {furos_esq++;} // evita debouncing
    t_esq.reset();
    t_esq.start();
    encoder_esq.enable_irq();
}

void cont_furos_dir() {
    encoder_dir.disable_irq();
    int tempo;
    t_dir.stop();
    tempo = t_dir.read_ms();
    if (tempo > 10) {furos_dir++;} // evita debouncing
    t_dir.reset();
    t_dir.start();
    encoder_dir.enable_irq();
}

/* ----- FUNÇÕES DE COMUNICAÇÃO ----- */
void interpretar_msg(char msg[]) {
    // Acionamento do motor esquerdo para a frente
    t_generico.start();
    int tempo_mov;
    if (msg[0] == 'a') {
        acionar_motor(ESQ, FRENTE, MEDIO);
        while(furos_esq < 100) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
        }
        frear_motor(ESQ);
        wait(0.6);
        furos_esq = 0;
    }

    // Acionamento dos dois motores para frente
    else if (msg[0] == 'w') {
        acionar_motor(ESQ, FRENTE, RAPID); acionar_motor(DIR, FRENTE, RAPID);
        while ((furos_dir < 100) || (furos_esq < 100)) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
            if (furos_dir >= 100) {frear_motor(DIR);}
            if (furos_esq >= 100) {frear_motor(ESQ);}
        }
        frear_motor(ESQ)                 ; frear_motor(DIR);
        wait(0.6);
        furos_esq = 0                    ; furos_dir = 0;
    }

    // Acionamento dos dois motores para trás
    else if (msg[0] == 's') {
        acionar_motor(ESQ, RE, RAPID); acionar_motor(DIR, RE, RAPID);
        while ((furos_dir < 100) || (furos_esq < 100)) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
            if (furos_dir >= 100) {frear_motor(DIR);}
            if (furos_esq >= 100) {frear_motor(ESQ);}
        }
        frear_motor(ESQ)             ; frear_motor(DIR);
        wait(0.6);
        furos_esq = 0                ; furos_dir = 0;
    }

    // Acionamento do motor direito para frente
    else if (msg[0] == 'd') {
        acionar_motor(DIR, FRENTE, MEDIO);
        while (furos_dir < 100) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
        }
        furos_dir = 0;
        frear_motor(DIR);
        wait(0.6);
        furos_dir = 0;
    }

    // Virar parado 90 graus à esquerda
    else if (msg[0] == 'j') {
        acionar_motor(ESQ, RE, MEDIO); acionar_motor(DIR, FRENTE, MEDIO);
        while ((furos_dir < 10) || (furos_esq < 10)) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
            if (furos_dir >= 10) {frear_motor(DIR);}
            if (furos_esq >= 10) {frear_motor(ESQ);}
        }
        frear_motor(ESQ)             ; frear_motor(DIR);
        wait(0.6);
        furos_esq = 0                ; furos_dir = 0;
    }

    // Virar parado 90 graus à direita
    else if (msg[0] == 'l') {
        acionar_motor(ESQ, FRENTE, MEDIO); acionar_motor(DIR, RE, MEDIO);
        while ((furos_dir < 10) || (furos_esq < 10)) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
            if (furos_dir >= 10) {frear_motor(DIR);}
            if (furos_esq >= 10) {frear_motor(ESQ);}
        }
        frear_motor(ESQ)             ; frear_motor(DIR);
        wait(0.6);
        furos_esq = 0                ; furos_dir = 0;
    }
    t_generico.stop();
    tempo_mov = t_generico.read_ms();
    pc.printf("\n\nTempo para executar movimento: %d [ms] \n", tempo_mov);
    t_generico.reset();
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

    encoder_esq.enable_irq();
    encoder_esq.rise(&cont_furos_esq);

    encoder_dir.enable_irq();
    encoder_dir.rise(&cont_furos_dir);

    while (1) {
        if (transceiver.readable()) {
            // le dado e transfere para buffer rx
            rxDataCnt = transceiver.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));
            interpretar_msg(rxData);
        }
    }
}
