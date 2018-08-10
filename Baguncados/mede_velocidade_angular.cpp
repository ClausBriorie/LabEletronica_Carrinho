#include "mbed.h"
#define PULSOS_POR_VOLTA 20
#define TRANSFER_SIZE 2

// LEDs para facilitar debbug e verificar funcionamento
DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);

// Comunicações entre placas e computador
Serial pc(USBTX, USBRX); // tx, rx

// Portas do sensor de velocidade
DigitalIn sensorDeVelocidade1(PTA1);
// DigitalIn sensorDeVelocidade2(PTA2);

// Interrupções dos sensores de velocidade
InterruptIn mede_vel_angular_1(PTA1);
// InterruptIn mede_vel_angular_2(PTA2);

// Comandos de rotação do motor
PwmOut motor_A_1(PTB0);
PwmOut motor_A_2(PTB1);
// PwmOut motor_B_1(PTB2);
// PwmOut motor_B_2(PTB3);

int contador = 0;

// TODO: encontrar funcao apropriada para pegar o tempo atual, para substituir time.GET_TIME()
// void calcula_RPM(unsigned long tempo_anterior, unsigned int pulsos) {
void calcula_RPM() {
   /*unsigned long delta_t;
   rpm = (60 * 1000/PULSOS_POR_VOLTA) / (time.GET_TIME() - tempo_anterior) * pulsos;
   pc.printf("\nRPM: %d\n", rpm);
   */
    contador++;
    pc.printf("\n\nVariacao no sinal do sensor de velocidade. Contador: %d \n\n", contador);
    wait(0.1);
}

int main() {
    // LEDs
    myled1 = 1; // iniciam desligados
    myled2 = 1;
    myled3 = 1;
    int contador = 0;

    mede_vel_angular_1.rise(&calcula_RPM);
    // Medidas de rotação
    int rpm_1 = 0;
    int rpm_2 = 0;
    unsigned long tempo_anterior = 0;
    unsigned long tempo_atual = 0;

    // Setup do PWM
    motor_A_1.period_ms(10);
    motor_A_1.pulsewidth_ms(2);
    motor_A_2.write(0.0);
    motor_A_2.period_ms(10);
    // motor_B_1.period_ms(10);
    // motor_B_1.pulsewidth_ms(2);
    // motor_B_2.write(0.0);
    // motor_B_2.period_ms(10);

    char Data[TRANSFER_SIZE];
    int DataCnt = 0;

    while (1) {

//        // Atualizar contador a cada segundo
//        //tempo_atual = time.GET_TIME();
//        //if (tempo_atual - tempo_anterior >= 1000) {
//        //
//        //}
//
//        // Se algo for recebido do host de link serial...
//        if (pc.readable()) {
//            // ...adicionamos isso ao buffer
//            Data[DataCnt++] = pc.getc();
//        }

        // Se o buffer está cheio...
        if (DataCnt >= TRANSFER_SIZE) {

            // Lemos Data e tomamos ações com o motor

            // Acionamento dos dois motores
            /*
            if ((rxData[0] == 'w')) {
                motor_B_1 = 0.5; motor_B_2 = 0.0;
                led_green = 0;
                motor_A_1 = 0.5; motor_A_2 = 0.0;
                led_red = 0;
                wait(1);
                motor_B_1 = 0.5; motor_B_2 = 0.5;
                led_green = 1;
                led_red = 1;
                motor_A_1 = 0.5; motor_A_2 = 0.5;
                wait(0.1);
            }
            */

            // Acionamento do motor A:
            if (Data[0] == 'a') {
                motor_A_1 = 0.3; motor_A_2 = 0.0;
                wait(1);
                motor_A_1 = 0.3; motor_A_2 = 0.3;
                wait(0.1);
            }

//
//            // Acionamento do motor B:
//            /*
//            else if (rxData[0] == 'd') {
//                motor_B_1 = 0.3; motor_B_2 = 0.0;
//                wait(1);
//                motor_B_1 = 0.3; motor_B_2 = 0.3;
//                wait(0.1);
//            }
//            */
//
//            // Mostra os conteúdos do buffer de recepção via link serial do host
//            for ( int i = 0; DataCnt > 0; DataCnt--, i++ ) {
//                pc.putc(Data[i]);
//            }
        }
    }
}
