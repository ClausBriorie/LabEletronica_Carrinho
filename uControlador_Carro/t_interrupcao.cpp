#include "mbed.h"
#define TRANSFER_SIZE   2

Serial pc(USBTX, USBRX); // tx, rx

// TODO: Ver se a linha abaixo é ou não essencial
//DigitalIn sensorDeVelocidade1(PTD7);

Timer t;
InterruptIn interrupcao(PTD7);
PwmOut motor(PTB0);
volatile int contador = 0;
int t_anterior = 0;

void conta_furos() {
    interrupcao.disable_irq();
    t.stop();
    int t_atual;
    int delta_t;
    int limiar_debouncing = 12; // milissegundos

    t_atual = t.read_ms();
    delta_t = t_atual - t_anterior;

    if (delta_t > limiar_debouncing) {
        contador++;
    }
    t.reset();
    t.start();
    interrupcao.enable_irq();
}

int main() {
    // inicia contagem do tempo
    t.start();

    char Data[TRANSFER_SIZE];
    int DataCnt = 0;
    motor.period_ms(20);
    motor = 0.5;
    interrupcao.enable_irq();
    interrupcao.rise(&conta_furos);
    while (1) {
        interrupcao.rise(&conta_furos);
         // Se algo for recebido do host de link serial...
        if (pc.readable()) {
            // ...adicionamos isso ao buffer
            Data[DataCnt++] = pc.getc();
        }
        // Se o buffer está cheio...
        if (DataCnt >= TRANSFER_SIZE) {
            if ((Data[0] == 'a') && (Data[1] == 'a')) {
                pc.printf("Contador: %d\n", contador);
                contador = 0;
            }
            DataCnt = 0;
        }
    }
}
