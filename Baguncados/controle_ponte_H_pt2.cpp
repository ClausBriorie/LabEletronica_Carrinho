/*
Programa que ativa e desativa saidas PWM de uma KL25Z usando o transceiver,
a partir de comandos enviados de uma segunda KL25Z conectada a um computador
*/

#include "mbed.h"
#include "nRF24L01P.h"

Serial pc(USBTX, USBRX); // tx, rx

nRF24L01P my_nrf24l01p(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq
DigitalOut led_green(LED1);
DigitalOut led_red(LED2);

#define TRANSFER_SIZE  2 // Tamanho do vetor de caracteres transferidos entre os transceivers

// TODO: Verificar se as direções estão corretas; Trocar se necessário
PwmOut motor_A_1(PTB0);
PwmOut motor_A_2(PTB1);
PwmOut motor_B_1(PTB2);
PwmOut motor_B_2(PTB3);

void setup_transceiver(nRF24L01P my_nrf24l01p) {
    my_nrf24l01p.powerUp();
    my_nrf24l01p.setRxAddress(0xE5E5E5E5E5);
    my_nrf24l01p.setRfFrequency(2448);
    my_nrf24l01p.setTxAddress(0xE5E5E5E5E5);
    // Apresenta a configuração (default) do chip nRF24L01+
    pc.printf("nRF24L01+ Frequency    : %d MHz\r\n",    my_nrf24l01p.getRfFrequency());
    pc.printf("nRF24L01+ Output power : %d dBm\r\n",    my_nrf24l01p.getRfOutputPower());
    pc.printf("nRF24L01+ Data Rate    : %d kbps\r\n",   my_nrf24l01p.getAirDataRate());
    pc.printf("nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress());
    pc.printf("nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress());
    pc.printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
    pc.printf( "Comandos \t _d : aciona motor direito \n\t a_ : aciona motor esquerdo \n\t ad : aciona os dois motores \n\n\n" );
    my_nrf24l01p.setTransferSize(TRANSFER_SIZE);
    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();
}

// TODO: Test me
void setup_PWMs(PwmOut motor_A_1, PwmOut motor_A_2, PwmOut motor_B_1, PwmOut motor_A_2) {
    motor_A_1.period_ms(20); motor_A_1 = 0.0;
    motor_A_2.period_ms(20); motor_A_2 = 0.0;
    motor_B_1.period_ms(20); motor_B_1 = 0.0;
    motor_B_2.period_ms(20); motor_B_2 = 0.0;
}

// TODO: complete me
/*
void setup_PWM_controls(PwmOut motor_1, PwmOut motor_2) {

}

void acionar_motor(PwmOut motor) {

}

void desligar_motor(PwmOut motor) {

}
*/

int main() {

    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    // LEDs iniciam desligados
    led_green = 1;
    led_red = 1;

    setup_PWSs(motor_A_1, motor_A_2, motor_B_1, motor_A_2);
    setup_transceiver(my_nrf24l01p);

    while (1) {

        // Se algo for recebido do host de link serial...
        if (pc.readable()) {

            // ...adicionamos isso ao buffer de transmissão
            txData[txDataCnt++] = pc.getc();

            // Se o buffer de transmissão está cheio...
            if (txDataCnt >= sizeof(txData)) {
                // ...enviamos seu conteúdo via nRF24L01+
                my_nrf24l01p.write(NRF24L01P_PIPE_P0, txData, txDataCnt);
                txDataCnt = 0;
            }
        }

        // Se recebermos algo pela nRF24L01+...
        if ( my_nrf24l01p.readable() ) {
            // ...lemos os dados e os transferimos para o buffer de recepção
            rxDataCnt = my_nrf24l01p.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));

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

            // Acionamento do motor A:
            else if (rxData[0] == 'a') {
                motor_A_1 = 0.3; motor_A_2 = 0.0;
                wait(1);
                motor_A_1 = 0.3; motor_A_2 = 0.3;
                wait(0.1);
            }

            // Acionamento do motor B:
            else if (rxData[0] == 'd') {

                motor_B_1 = 0.3; motor_B_2 = 0.0;
                wait(1);
                motor_B_1 = 0.3; motor_B_2 = 0.3;
                wait(0.1);
            }

            // Mostra os conteúdos do buffer de recepção via link serial do host
            for (int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {
                pc.putc(rxData[i]);
            }
        }
    }
}
