#include "mbed.h"
#include "nRF24L01P.h"
#define TAMANHO_MSG  2
#define FRENTE 1
#define RE -1
#define ESQ 4
#define DIR 5
#define RAPID 0.5
#define MEDIO 0.4
#define DEVAG 0.3
#define TAMANHO_MSG  2

// Comunicações
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
Timer t;
InterruptIn encoder_esq(PTD4);
InterruptIn encoder_dir(PTA12);

Ticker t_cont_esq;
Ticker t_cont_dir;

volatile int furos_esq = 0;
volatile int furos_dir = 0;

// Sensor de distancia
DigitalOut trigger(PTA4);
InterruptIn echo(PTA5);
Timer t_ultrassom;
int delta_ultrassom;
int delta_ant;
float distancia;
Ticker t_print;
Ticker t_trigger;

/* ----- FUNÇÕES DE MOTORES E MOVIMENTAÇÃO ----- */
void setup_motores() {
    motor_E_1.period_ms(20); motor_E_1 = 0.0;
    motor_E_2.period_ms(20); motor_E_2 = 0.0;
    motor_D_1.period_ms(20); motor_D_1 = 0.0;
    motor_D_2.period_ms(20); motor_D_2 = 0.0;
}

void acionar_motor(int id_motor, int sentido, float vel) {
    pc.printf("aciona_motor chamada\n");
    if (id_motor == ESQ) {
        if (sentido == RE) {
            motor_E_1 = vel; motor_E_2 = 0.0;
        }
        else if (sentido == FRENTE) {
            motor_E_1 = 0.0; motor_E_2 = vel;
        }
    }
    if (id_motor == DIR) {
        vel = vel*1.2; // motor direito é ligeiramente mais lento
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
    furos_esq++;
    encoder_esq.enable_irq();
}

void cont_furos_dir() {
    encoder_dir.disable_irq();
    furos_dir++;
    encoder_dir.enable_irq();
}

void print_cont_esq() {
    pc.printf("furos_esq: %d\n", furos_esq);
}
void print_cont_dir() {
    pc.printf("furos_dir: %d\n", furos_dir);
}

/* ----- FUNÇÕES DE COMUNICAÇÃO ----- */
void interpretar_msg(char msg[]) {
    // Acionamento do motor esquerdo para a frente
    t.start();
    int tempo_mov;
    if (msg[0] == 'a') {
        pc.printf("msg[0] == 'a'\n");
        acionar_motor(ESQ, FRENTE, MEDIO);
        while(furos_esq < 100) {
        }
        pc.printf("frear_motor(ESQ)\n");
        frear_motor(ESQ);
        wait(0.6);
        furos_esq = 0;
    }

    // Acionamento dos dois motores para frente
    else if (msg[0] == 'w') {
        acionar_motor(ESQ, FRENTE, RAPID); acionar_motor(DIR, FRENTE, RAPID);
        while ((furos_dir < 100) || (furos_esq < 100)) {
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
    t.stop();
    tempo_mov = t.read_ms();
    pc.printf("\n\nTempo para executar movimento: %d [ms] \n", tempo_mov);
    t.reset();
}

void setup_transceiver(nRF24L01P transceiver) {
    transceiver.powerUp();
    transceiver.setRxAddress(0xE5E5E5E5E5);
    transceiver.setRfFrequency(2448);
    transceiver.setTxAddress(0xE5E5E5E5E5);
    transceiver.setTransferSize(TAMANHO_MSG);
    transceiver.setReceiveMode();
    transceiver.enable();
    pc.printf("nRF24L01+ Frequency    : %d MHz\r\n",    transceiver.getRfFrequency());
    pc.printf("nRF24L01+ Output power : %d dBm\r\n",    transceiver.getRfOutputPower());
    pc.printf("nRF24L01+ Data Rate    : %d kbps\r\n",   transceiver.getAirDataRate());
    pc.printf("nRF24L01+ TX Address   : 0x%010llX\r\n", transceiver.getTxAddress());
    pc.printf("nRF24L01+ RX Address   : 0x%010llX\r\n", transceiver.getRxAddress());
    pc.printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TAMANHO_MSG );
}

/* ----- FUNÇÕES DE DETECÇÃO DE OBSTÁCULOS ----- */

void inicia_calc_dist() {
    echo.disable_irq();
    t_ultrassom.start();
    echo.enable_irq();
}

void finaliza_calc_dist() {
    echo.disable_irq();
    t_ultrassom.stop();
    delta_ultrassom = t_ultrassom.read_us();
    distancia = delta_ultrassom/58.0;
    // mostra_delta();
    t_ultrassom.reset();
    echo.enable_irq();
}

void dispara_trigger() {
    echo.disable_irq();
    trigger = 1;
    wait_us(10);
    trigger = 0;
    echo.enable_irq();
}


int main() {
    char rxData[TAMANHO_MSG];
    int rxDataCnt = 0;
    t_cont_dir.attach(&print_cont_dir, 0.5);
    t_cont_esq.attach(&print_cont_esq, 0.5);


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
