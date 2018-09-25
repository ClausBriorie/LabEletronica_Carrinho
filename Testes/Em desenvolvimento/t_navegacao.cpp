#include "mbed.h"
#include "nRF24L01P.h"
#include <string>
#define TAMANHO_MSG  8

// Constantes de direçôes
#define FRENTE 1
#define RE -1
#define ESQ 4
#define DIR 5
#define AMBOS 6

// Constantes de velocidades
#define RAPID 0.60
#define MEDIO 0.45
#define DEVAG 0.40

// Constantes de alinhamento com os eixos
#define X_POS 10
#define X_NEG -10
#define Y_POS 20
#define Y_NEG -20

// LEDs
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

// Comunicações
nRF24L01P transceiver(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq
Serial pc(USBTX, USBRX); // tx, rx

// Controle de motores
PwmOut motor_D_1(PTB2); //   MOTOR A - IN1
PwmOut motor_D_2(PTB3); //   MOTOR A - IN2
PwmOut motor_E_1(PTB0); //   MOTOR B - IN3
PwmOut motor_E_2(PTB1); //   MOTOR B - IN4

// Encoders
Timer t_esq;
Timer t_dir;
Timer t;
InterruptIn encoder_esq(PTD4);
InterruptIn encoder_dir(PTA12);
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

// NAVEGAÇÃO
volatile int posicao_atual_x = 0;
volatile int posicao_atual_y = 0;
int alinhamento;
bool andando_para_frente;
bool terminou_x;
bool terminou_y;
int posicao_alvo_x = 0;
int posicao_alvo_y = 0;



/* ----- FUNÇÕES DE MOTORES E MOVIMENTAÇÃO ----- */
void setup_motores() {
    motor_E_1.period_ms(5); motor_E_1 = 0.0;
    motor_E_2.period_ms(5); motor_E_2 = 0.0;
    motor_D_1.period_ms(5); motor_D_1 = 0.0;
    motor_D_2.period_ms(5); motor_D_2 = 0.0;
}

void acionar_motor(int id_motor, int sentido, float vel) {
    if (id_motor == ESQ) {
        if (sentido == RE) {
            motor_E_1 = vel*1.3; motor_E_2 = 0.0; // Tentativa de minimizar o efeito
            wait(0.07);                       // da inércia de arranque do motor esquerdo
            motor_E_1 = vel; motor_E_2 = 0.0;
        }
        else if (sentido == FRENTE) {
            motor_E_1 = 0.0; motor_E_2 = vel*1.3;
            wait(0.07);
            motor_E_1 = 0.0; motor_E_2 = vel;
        }
    }
    else if (id_motor == DIR) {
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

void ponto_morto_motor(int id_motor) {
    if (id_motor == ESQ) {
        motor_E_1 = 0.0; motor_E_2 = 0.0;
    }
    else if (id_motor == DIR) {
        motor_D_1 = 0.0; motor_D_2 = 0.0;
    }
}

void realizar_movimento(char mov[]) {
    // CARRO ANDA PARA A FRENTE
    if (mov[0] == 'w') {
        //  Acionamento dos dois motores para frente
        acionar_motor(ESQ, FRENTE, RAPID); acionar_motor(DIR, FRENTE, MEDIO);
        while ((furos_dir < 2000) || (furos_esq < 2000)) {
            // Condição parada do movimento
            if (furos_dir >= 2000) {ponto_morto_motor(DIR);}
            if (furos_esq >= 2000) {ponto_morto_motor(ESQ);}


            if ( (distancia < 20.0) ||
                 ((terminou_x == false) && (posicao_atual_x >=posicao_alvo_x)) ||
                 ((terminou_y == false) && (posicao_atual_y >=posicao_alvo_y)) ) {
                break;
            }
        }
        furos_esq = 0                    ; furos_dir = 0;
    }

    // CARRO GIRA PARA A ESQUERDA
    else if (mov[0] == 'a') {
        // Acionamento do motor direito para frente
        frear_motor(ESQ);
        acionar_motor(DIR, FRENTE, MEDIO);
        while (furos_dir < 22) {
        }
        furos_dir = 0;
    }

    // CARRO ANDA PARA TRÁS
    else if (mov[0] == 's') {
        // Acionamento dos dois motores para trás
        acionar_motor(ESQ, RE, RAPID); acionar_motor(DIR, RE, MEDIO);
        while ((furos_dir < 100) || (furos_esq < 100)) {
            if (furos_dir >= 100) {ponto_morto_motor(DIR);}
            if (furos_esq >= 100) {ponto_morto_motor(ESQ);}
        }

        furos_esq = 0                ; furos_dir = 0;
    }

    // CARRO GIRA PARA A DIREITA
    else if (mov[0] == 'd') {
        // Acionamento do motor esquerdo para a frente
        frear_motor(DIR);
        acionar_motor(ESQ, FRENTE, MEDIO);
        while(furos_esq < 22) {
        }
        furos_esq = 0;
    }

    // GIRAR PARADO 90 GRAUS À ESQUERDA
    else if (mov[0] == 'j') {
        acionar_motor(ESQ, RE, RAPID); acionar_motor(DIR, FRENTE, MEDIO);
        while ((furos_dir < 22) || (furos_esq < 22)) {
            if (furos_dir >= 22) {ponto_morto_motor(DIR);}
            if (furos_esq >= 22) {ponto_morto_motor(ESQ);}
        }
        furos_esq = 0                ; furos_dir = 0;
    }

    // GIRAR PARADO 90 GRAUS À DIREITA
    else if (mov[0] == 'l') {
        acionar_motor(ESQ, FRENTE, RAPID); acionar_motor(DIR, RE, MEDIO);
        while ((furos_dir < 22) || (furos_esq < 22)) {
            if (furos_dir >= 22) {ponto_morto_motor(DIR);}
            if (furos_esq >= 22) {ponto_morto_motor(ESQ);}
        }
        furos_esq = 0                ; furos_dir = 0;
    }

    ponto_morto_motor(ESQ)           ; ponto_morto_motor(DIR);
    wait(1);
}

/* ----- FUNÇÕES DE ENCODERS ----- */
void cont_furos_esq() {
    encoder_esq.disable_irq();
    int delta;
    t_esq.stop();
    delta = t_esq.read_ms();
    if (delta > 14) {
        furos_esq++;  // não conta efeitos de bouncing
    }
    t_esq.reset();
    t_esq.start();
    if ((alinhamento == Y_POS) && (andando_para_frente)) {
        posicao_atual_y++;
    }
    if ((alinhamento == X_POS) && (andando_para_frente)) {
        posicao_atual_x++;
    }
    encoder_esq.enable_irq();
}

void cont_furos_dir() {
    encoder_dir.disable_irq();
    int delta;
    t_dir.stop();
    delta = t_dir.read_ms();
    if (delta > 14) {
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

void finaliza_calc_dist() {
    echo.disable_irq();
    t_ultrassom.stop();
    delta_ultrassom = t_ultrassom.read_us();
    distancia = delta_ultrassom/58.0;
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
int ajustar_alinhamento(int alinhamento_atual, int direcao_da_curva) {
    if (direcao_da_curva == ESQ) {
        if (alinhamento == X_POS) {
            alinhamento = Y_POS;
            led1 = 1; led2 = 0;
        }
        else if (alinhamento == X_NEG) {
            alinhamento = Y_NEG;
        }
        else if (alinhamento == Y_POS) {
            alinhamento = X_NEG;
        }
        else if (alinhamento == Y_NEG) {
            alinhamento = X_POS;
            led1 = 0; led2 = 1;
        }
    }

    if (direcao_da_curva == DIR) {
        if (alinhamento == X_POS) {
            alinhamento = Y_NEG;
        }
        else if (alinhamento == Y_NEG) {
            alinhamento = X_NEG;
        }
        else if (alinhamento == X_NEG) {
            alinhamento = Y_POS;
            led1 = 1; led2 = 0;
        }
        else if (alinhamento == Y_POS) {
            alinhamento = X_POS;
            led1 = 0; led2 = 1;
        }
    }

    return alinhamento;
}

void print_condicoes_de_navegacao(int posicao_atual_x, int posicao_atual_y, int alinhamento, float distancia, bool andando_para_frente) {
    /*
    Função usada apenas para debbug
    */
    pc.printf("\n\n----------------------------------\n");
    pc.printf("posicao_atual_x:     %d\n", posicao_atual_x);
    pc.printf("posicao_atual_y:     %d\n", posicao_atual_y);
    pc.printf("alinhamento:         %d\n", alinhamento);
    pc.printf("distancia:           %f\n", distancia);
    pc.printf("andando_para_frente: %d\n", andando_para_frente);
    pc.printf("----------------------------------\n\n");
}


int main() {
    char rxData[TAMANHO_MSG];
    int rxDataCnt = 0;
    led1 = 0;
    led2 = 1;
    led3 = 1;

    // Ultrassom
    t_trigger.attach(&dispara_trigger, 0.25);
    echo.enable_irq();
    echo.rise(inicia_calc_dist);
    echo.fall(finaliza_calc_dist);
    trigger = 0;

    setup_transceiver(transceiver);
    setup_motores();

    // Encoder
    encoder_esq.enable_irq();
    encoder_esq.rise(&cont_furos_esq);
    encoder_dir.enable_irq();
    encoder_dir.rise(&cont_furos_dir);

    // Navegação
    alinhamento = X_POS;
    terminou_x = false;
    terminou_y = false;


    // Loop de aquisição da posição alvo
    while (1) {
        if (transceiver.readable()) {
            pc.printf("posicao_alvo_x: %d\n", posicao_alvo_x);
            pc.printf("posicao_alvo_y: %d\n", posicao_alvo_y);

            rxDataCnt = transceiver.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));

            char *token;
            int a,b;

            token = strtok(rxData, ",;");
            pc.printf("token = %s\n", token);

            token = strtok(NULL, ",;");
            a = atoi(token);

            token = strtok(NULL, ",;");
            b = atoi(token);

            pc.printf("a: %d\n", a);
            pc.printf("b: %d\n", b);

            posicao_alvo_x = a;
            posicao_alvo_y = b;
            pc.printf("modo de aquisicao\n\n\n");

            pc.printf("posicao_alvo_x: %d\n", posicao_alvo_x);
            pc.printf("posicao_alvo_y: %d\n", posicao_alvo_y);
            break;
        }
    }

    pc.printf("************ aquisicao feita ***************\n\n\n");

    pc.printf("posicao_alvo_x: %d\n", posicao_alvo_x);
    pc.printf("posicao_alvo_y: %d\n", posicao_alvo_y);

    // Loop de execução do trajeto
    wait(4);
    while ((posicao_atual_x < posicao_alvo_x) || (posicao_atual_y < posicao_alvo_y)) {
        andando_para_frente = true;
        realizar_movimento("ww");
        // print_condicoes_de_navegacao(posicao_atual_x, posicao_atual_y, alinhamento, distancia, andando_para_frente);
        andando_para_frente = false;

        // Se chegou na posicao alvo nas duas coordenadas, parar e avisar
        if ((posicao_atual_x >= posicao_alvo_x) && (posicao_atual_y >= posicao_alvo_y)) {
            frear_motor(ESQ);
            frear_motor(DIR);
            wait(1);
            while(1) {
                led3 = !led3;
                wait(0.1);
                led2 = !led2;
                wait(0.1);
                led1 = !led1;
                wait(0.1);
            }
        }

        // Se   : (encontrar obstáculo enquanto andava no eixo X e ainda tem que andar mais em Y) OU (já andou o que precisava em X)
        // Então: vira 90 graus à esquerda
        if (((distancia < 20.0) && (alinhamento == X_POS) && (posicao_atual_y < posicao_alvo_y))
            ||
            ((posicao_atual_x >= posicao_alvo_x) && (alinhamento == X_POS)) ) {
            wait(1);
            realizar_movimento("jj");
            alinhamento = ajustar_alinhamento(alinhamento, ESQ);
            // print_condicoes_de_navegacao(posicao_atual_x, posicao_atual_y, alinhamento, distancia, andando_para_frente);
            terminou_x = true;
            wait(1);
        }

        // Se   : (encontrar um obstáculo enquanto estava andando no eixo Y) E 9ainda tem que andar mais em X)
        // Então: vira 90 graus à direita
        if (((distancia < 20.0) && (alinhamento == Y_POS)) // && (posicao_atual_x < posicao_alvo_x)
            ||
            ((posicao_atual_y >= posicao_alvo_y) && (alinhamento == Y_POS)) ) {
            wait(1);
            realizar_movimento("ll");
            alinhamento = ajustar_alinhamento(alinhamento, DIR);
            // print_condicoes_de_navegacao(posicao_atual_x, posicao_atual_y, alinhamento, distancia, andando_para_frente);
            terminou_y = true;
            wait(1);
        }
    }
}
