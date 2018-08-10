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

// The nRF24L01+ supports transfers from 1 to 32 bytes, but Sparkfun's
//  "Nordic Serial Interface Board" (http://www.sparkfun.com/products/9019)
//  only handles 4 byte transfers in the ATMega code.
#define TRANSFER_SIZE  4 // Tamanho do vetor de caracteres transferidos entre os transceivers

// TODO: Verificar se as direções estão corretas; Trocar se necessário
PwmOut IN1(PTB0);
PwmOut IN2(PTB1);
PwmOut IN3(PTB2);
PwmOut IN4(PTB3);

int main() {

    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    // LEDs iniciam desligados
    led_green = 1;
    led_red = 1;

    // Setup do PWM
    IN1.period_ms(20); IN1 = 0.0;
    IN2.period_ms(20); IN2 = 0.0;
    IN3.period_ms(20); IN3 = 0.0;
    IN4.period_ms(20); IN4 = 0.0;

    // Ligando o transceiver
    my_nrf24l01p.powerUp();

    //TODO: Mudar as configurações para que outros transceivers não afetem os nossos
    // Apresenta a configuração (default) do chip nRF24L01+

    my_nrf24l01p.setTransferSize(TRANSFER_SIZE);

    ///////////////////////////////////////////////////////////////////////////////////////////
    my_nrf24l01p.setRxAddress(0xE5E5E5E5E5);
    my_nrf24l01p.setRfFrequency(2448);
    my_nrf24l01p.setTxAddress(0xE5E5E5E5E5);
    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();

    pc.printf("nRF24L01+ Frequency    : %d MHz\r\n",    my_nrf24l01p.getRfFrequency());
    pc.printf("nRF24L01+ Output power : %d dBm\r\n",    my_nrf24l01p.getRfOutputPower());
    pc.printf("nRF24L01+ Data Rate    : %d kbps\r\n",   my_nrf24l01p.getAirDataRate());
    pc.printf("nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress());
    pc.printf("nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress());
    pc.printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
    pc.printf( "Comandos \t a_ : aciona motor A \n\t _b : aciona motor B \n\t ab : aciona os dois motores \n\n\n" );

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
            pc.printf("\n txDataCnt: %s \n", txData[txData]);
        }

        // Se recebermos algo pela nRF24L01+...
        if ( my_nrf24l01p.readable() ) {
            // ...lemos os dados e os transferimos para o buffer de recepção
            rxDataCnt = my_nrf24l01p.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));
            pc.printf("\n rxData[rxData]: %s \n", rxData[rxDataCnt]);

            if ((rxData[0] == 'a') && (rxData[1] == 'b')) {
                IN1 = 0.3; IN2 = 0.0;
                IN3 = 0.3; IN4 = 0.0;
                wait(2);
                IN1 = 0.3; IN2 = 0.3;
                IN3 = 0.3; IN4 = 0.3;
                wait(1);
            }

            // Acionamento do motor A:
            else if (rxData[0] == 'a') {
                IN1 = 0.3; IN2 = 0.0;
                wait(2);
                IN1 = 0.3; IN2 = 0.3;
                wait(1);
            }

            // Acionamento do motor B:
            else if (rxData[1] == 'b') {

                IN3 = 0.3; IN4 = 0.0;
                wait(2);
                IN3 = 0.3; IN4 = 0.3;
                wait(1);
            }

            // Mostra os conteúdos do buffer de recepção via link serial do host
            for (int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {
                pc.putc(rxData[i]);
            }
        }
    }
}
