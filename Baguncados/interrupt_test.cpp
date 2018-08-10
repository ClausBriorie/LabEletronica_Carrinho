#include "mbed.h"
#define TRANSFER_SIZE   2 // Tamanho do vetor de caracteres transferidos entre os transceivers

// Comunicações entre placas e computador
Serial pc(USBTX, USBRX); // tx, rx
DigitalIn sensorDeVelocidade1(PTD7);
InterruptIn interrupcao(PTD7);
PwmOut motor(PTB0);
volatile int contador = 0;

void funcao_chamada_pela_interrupcao() {
    contador++;
    pc.printf(" Interrupcao foi chamada\n");
}

void funcao_2() {
    //gpio_irq_disable(*PTD7);
    pc.printf("funcao_2() chamada; valor do contador: %d/n/n", contador);
    //gpio_irq_enable(*PTD7);
}

int main() {
    char Data[TRANSFER_SIZE];
    int DataCnt = 0;
    motor.period_ms(20);
    motor = 0.5;
    interrupcao.enable_irq();
    interrupcao.rise(&funcao_chamada_pela_interrupcao);
    while (1) {
        interrupcao.rise(&funcao_chamada_pela_interrupcao);
         // Se algo for recebido do host de link serial...
        if (pc.readable()) {
            // ...adicionamos isso ao buffer
            Data[DataCnt++] = pc.getc();
        }

        // Se o buffer está cheio...
        if (DataCnt >= TRANSFER_SIZE) {
            if ((Data[0] == 'a') && (Data[1] == 'a')) {
                pc.printf("Contador: %d\n", contador);
            }
            DataCnt = 0;
        }
    }
}
