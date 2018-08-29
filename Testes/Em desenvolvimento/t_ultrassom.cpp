/*
Código para testar o funcionamento do sensor de distancia
*/
#include "mbed.h"

Serial pc(USBTX, USBRX); // tx, rx
// PwmOut trigger(PTA4);
DigitalOut trigger(PTA4);
InterruptIn echo(PTA5);
Timer t_ultrassom;
int delta_ultrassom;
int delta_ant;
Ticker t_print;
Ticker t_trigger;

void finaliza_calc_dist();

void inicia_calc_dist() {
    echo.disable_irq();
    t_ultrassom.start();
    echo.enable_irq();
}

void mostra_delta() {
    float distancia = delta_ultrassom/58.0;
    pc.printf("Distancia: %.2f\n", distancia);
}

void finaliza_calc_dist() {
    echo.disable_irq();
    t_ultrassom.stop();
    delta_ultrassom = t_ultrassom.read_us();
    mostra_delta();
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
    // t_print.attach(&mostra_delta, 0.5); // ordena o print a cada arg[2] seg
    t_trigger.attach(&dispara_trigger, 0.25);
    echo.enable_irq();
    echo.rise(inicia_calc_dist);
    echo.fall(finaliza_calc_dist);
    trigger = 0;
    while(1) {
    }
}
