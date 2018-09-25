#include "mbed.h"
#include "nRF24L01P.h"
#define TAMANHO_MSG  2
#define FRENTE 1
#define RE -1
#define ESQ 4
#define DIR 5
#define RAPID 0.6
#define MEDIO 0.5
#define DEVAG 0.4
#define TAMANHO_MSG  4
#define ALINHADO_EM_X 0
#define ALINHADO_EM_Y 1

// Comunicações        OK    OK    OK    OK    OK    OK  - FIOS VERDE/AZUL/ROXO
nRF24L01P transceiver(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq
Serial pc(USBTX, USBRX); // tx, rx

// Controle de motores       FIOS CINZA/ROXO/PRETO/BRANCO
PwmOut motor_D_1(PTB2); //   aka MOTOR A - IN1 - FIO ROXO     OK
PwmOut motor_D_2(PTB3); //   aka MOTOR A - IN2 - FIO CINZA    OK
PwmOut motor_E_1(PTB0); //   aka MOTOR B - IN3 - FIO BRANCO   OK
PwmOut motor_E_2(PTB1); //   aka MOTOR B - IN4 - FIO PRETO    OK

// Encoders
Timer t_esq;
Timer t_dir;
Timer t;
InterruptIn encoder_esq(PTD4);  // OK - FIO LARANJA
InterruptIn encoder_dir(PTA12); // OK - FIO ROXO
// Ticker t_cont_esq;
// Ticker t_cont_dir;
volatile int furos_esq = 0;
volatile int furos_dir = 0;

// Sensor de distancia
DigitalOut trigger(PTA4); // OK
InterruptIn echo(PTA5);   // OK
Timer t_ultrassom;
int delta_ultrassom;
int delta_ant;
float distancia;
Ticker t_print;
Ticker t_trigger;

// NAVEGAÇÃO
int posicao_alvo_x = 300; // posição medida em número de furos enquanto FRENTE e orientação X
int posicao_alvo_y = 200; // posição medida em número de furos enquanto FRENTE e orientação Y
int posicao_atual_x = 0;
int posicao_atual_y = 0;
int alinhamento = ALINHADO_EM_X;


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

void realizar_movimento(char msg[]) {
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
        while ((furos_dir < 20) || (furos_esq < 20)) {
            // Evitar que um motor gire muito mais rapidamente do que o outro
            // if (furos_dir > furos_esq + 3) {acionar_motor(DIR, FRENTE, RAPID*0.99);}
            // if (furos_esq > furos_dir + 3) {acionar_motor(ESQ, FRENTE, RAPID*0.99);}

            // Condição parada do movimento
            if (furos_dir >= 20) {frear_motor(DIR);}
            if (furos_esq >= 20) {frear_motor(ESQ);}
        }
        // TODO: atualizar posicao_atual correspondente ao alinhamento
        frear_motor(ESQ)                 ; frear_motor(DIR);
        wait(0.3);
        if (alinhamento == ALINHADO_EM_X) {
            posicao_atual_x = posicao_atual_x + furos_dir;
        }
        if (alinhamento == ALINHADO_EM_Y) {
            posicao_atual_y = posicao_atual_x + furos_esq;
        }

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
        while ((furos_dir < 12) || (furos_esq < 12)) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
            if (furos_dir >= 12) {frear_motor(DIR);}
            if (furos_esq >= 12) {frear_motor(ESQ);}
        }
        frear_motor(ESQ)             ; frear_motor(DIR);
        wait(0.6);
        furos_esq = 0                ; furos_dir = 0;
    }

    // Virar parado 90 graus à direita
    else if (msg[0] == 'l') {
        acionar_motor(ESQ, FRENTE, MEDIO); acionar_motor(DIR, RE, MEDIO);
        while ((furos_dir < 12) || (furos_esq < 12)) {
            // TODO: remover linha abaixo após testar
            // pc.printf("furos: \t esq: %d \t dir: %d\n", furos_esq, furos_dir);
            if (furos_dir >= 12) {frear_motor(DIR);}
            if (furos_esq >= 12) {frear_motor(ESQ);}
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

/* ----- FUNÇÕES DE ENCODERS ----- */
void cont_furos_esq() {
    encoder_esq.disable_irq();
    int delta;
    t_esq.stop();
    delta = t_esq.read_ms();
    if (delta > 20) {
        furos_esq++;  // não conta efeitos de bouncing
    }
    t_esq.reset();
    t_esq.start();
    encoder_esq.enable_irq();
}

void cont_furos_dir() {
    encoder_dir.disable_irq();
    int delta;
    t_dir.stop();
    delta = t_dir.read_ms();
    if (delta > 20) {
        furos_dir++; // não conta efeitos de bouncing
    }
    t_dir.reset();
    t_dir.start();
    encoder_dir.enable_irq();
}

void print_cont_esq() {
    pc.printf("furos_esq: %d\n", furos_esq);
}

void print_cont_dir() {
    pc.printf("furos_dir: %d\n", furos_dir);
    pc.printf("posicao_atual_x: %d\n", posicao_atual_x);
    pc.printf("posicao_atual_y: %d\n", posicao_atual_y);
}

/* ----- FUNÇÕES DE COMUNICAÇÃO ----- */

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

void mostra_distancia() {
    pc.printf("distancia: %.2f  [cm]\n", distancia);
}

void finaliza_calc_dist() {
    echo.disable_irq();
    t_ultrassom.stop();
    delta_ultrassom = t_ultrassom.read_us();
    distancia = delta_ultrassom/58.0;
    mostra_distancia();
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

/* ----- FUNÇÕES DE NAVEGAÇÃO ----- */

void caminhar_em_y();

void caminhar_em_x() {
    while (posicao_atual_x < posicao_alvo_x) {
        realizar_movimento("ww");
        // Se encontrar um obstáculo, vira 90 graus à esquerda e continua a andar
        if ((distancia < 15) && (posicao_atual_y < posicao_alvo_y)) {
            realizar_movimento("jj");
            caminhar_em_y();
        }
    }
    // Se terminar de andar o necessário em X, continua a andar em Y
    if (posicao_atual_y < posicao_alvo_y) {
        caminhar_em_y();
    }
    // Se chegou na posicao alvo nas duas coordenadas, faz dança da vitória
    if ((posicao_atual_x >= posicao_alvo_x) && (posicao_atual_y >= posicao_alvo_y)) {
        while(1) {
            realizar_movimento("jj");
        }
    }
}

void caminhar_em_y() {
    while (posicao_atual_y < posicao_alvo_y) {
        realizar_movimento("ww");
        // Se encontrar um obstáculo, vira 90 graus à direita e continua a andar
        if ((distancia < 15) && (posicao_atual_x < posicao_alvo_x)) {
            realizar_movimento("ll");
            caminhar_em_x();
        }
    }
    // Se terminar de andar o necessário em Y, continua a andar em X
    if (posicao_atual_x < posicao_alvo_x) {
        caminhar_em_x();
    }
    // Se chegou na posicao alvo nas duas coordenadas, faz dança da vitória
    if ((posicao_atual_y >= posicao_alvo_y) && (posicao_atual_x >= posicao_alvo_x)) {
        while(1) {
            realizar_movimento("jj");
        }
    }
}


int main() {
    char rxData[TAMANHO_MSG];
    int rxDataCnt = 0;

    // ultrassom
    t_trigger.attach(&dispara_trigger, 0.25);
    echo.enable_irq();
    echo.rise(inicia_calc_dist);
    echo.fall(finaliza_calc_dist);
    trigger = 0;

    setup_transceiver(transceiver);
    setup_motores();

    // encoder
    encoder_esq.enable_irq();
    encoder_esq.rise(&cont_furos_esq);
    encoder_dir.enable_irq();
    encoder_dir.rise(&cont_furos_dir);

    while (1) {
        // if (distancia <= 15.0) {
        //   realizar_movimento("jj");
        // }
        caminhar_em_x();
        if (transceiver.readable()) {
            // le dado e transfere para buffer rx
            rxDataCnt = transceiver.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));
            realizar_movimento(rxData);
        }
    }
}
