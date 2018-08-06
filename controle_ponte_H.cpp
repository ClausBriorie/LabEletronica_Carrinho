/*
Programa que ativa e desativa saidas PWM de uma KL25Z usando o transceiver,
a partir de comandos enviados de uma segunda KL25z conectada a um computador

Pegar o nRF24L01P_hello_world, e substituir a main.cpp daquele programa por este código

Lembrar de renomear este código para main.cpp
*/

#include "mbed.h"
#include "nRF24L01P.h"

#define TRANSFER_SIZE   2 // Tamanho do vetor de caracteres transferidos entre os transceivers

Serial pc(USBTX, USBRX); // tx, rx

nRF24L01P my_nrf24l01p(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq

// TODO: Verificar se as direções estão corretas; Trocar se necessário
PwmOut motor_esq(PTB0);
PwmOut motor_dir(PTB2);

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);

int main() {
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    // LEDs iniciam desligados
    myled1 = 1;
    myled2 = 1;
    myled3 = 1;

    // Setup do PWM
    motor_dir.period_ms(10);
    motor_dir.pulsewidth_ms(2);
    motor_dir.write(0.0);
    motor_esq.period_ms(10);
    motor_esq.pulsewidth_ms(2);
    motor_esq.write(0.0);

    // Ligando o transceiver
    my_nrf24l01p.powerUp();

    //TODO: Mudar as configurações para que outros transceivers não afetem os nossos
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

            if ((rxData[0] == 'a') && (rxData[1] == 'd')) {
                motor_esq.period_ms(10);
                motor_dir.period_ms(10);
                motor_esq.pulsewidth_ms(1);
                motor_dir.pulsewidth_ms(1);
                wait(1);
                motor_esq.period_ms(10);
                motor_dir.period_ms(10);
                motor_esq.pulsewidth_ms(0);
                motor_dir.pulsewidth_ms(0);
            }

            // Acionamento do motor esquerdo:
            else if (rxData[0] == 'a') {
                motor_esq.period_ms(10);
                motor_esq.pulsewidth_ms(1);
                wait(1);
                motor_esq.period_ms(10);
                motor_esq.pulsewidth_ms(0);
            }

            // Acionamento do motor direito:
            else if (rxData[1] == 'd') {
                motor_dir.period_ms(10);
                motor_dir.pulsewidth_ms(1);
                wait(1);
                motor_dir.period_ms(10);
                motor_dir.pulsewidth_ms(0);
            }

            // Mostra os conteúdos do buffer de recepção via link serial do host
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {
                pc.putc(rxData[i]);
            }
        }
    }
}
