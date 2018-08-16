/*
Teste de comunicacao usando transceiver nRF24L01p
*/
#include "mbed.h"
#include "nRF24L01P.h"
#define TRANSFER_SIZE  2 // Tamanho do vetor de caracteres transferidos entre os transceivers

Serial pc(USBTX, USBRX); // tx, rx
nRF24L01P my_nrf24l01p(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq
DigitalOut led_green(LED1);
DigitalOut led_red(LED2);

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
int transmitir_do_PC(char txData[], int txDataCnt) {
    if (txDataCnt >= TRANSFER_SIZE) {
       // ...enviamos seu conteúdo via nRF24L01+
       my_nrf24l01p.write(NRF24L01P_PIPE_P0, txData, txDataCnt);
       txDataCnt = 0;
    }
    return txDataCnt;
}

void print_no_PC(char msg[], int msgCnt) {
    for (int i = 0; msgCnt > 0; msgCnt--, i++) {
        pc.putc(msg[i]);
    }
}

void interpretar_msg(char msg[]) {
    // Açao 'a'
    if ((msg[0] == 'a')) {
        led_green = 0;
        wait(0.5);
        led_green = 1;
    }
    // Açao 'b'
    else if (msg[0] == 'b') {
        led_red = 0;
        wait(0.5);
        led_red = 1;
    }
    // Açao 'c'
    else if (msg[0] == 'c') {
        led_green = 0;
        led_red = 0;
        wait(0.5);
        led_green = 1;
        led_red = 1;
    }
}

int main() {
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    // LEDs iniciam desligados
    led_green = 1;
    led_red = 1;
    setup_transceiver(my_nrf24l01p);

    while (1) {
        if (pc.readable()) {
            // le dado e transfere para buffer tx
            txData[txDataCnt++] = pc.getc();
            txDataCnt = transmitir_do_PC(txData, txDataCnt);
        }

        if (my_nrf24l01p.readable()) {
            // le dado e transfere para buffer rx
            rxDataCnt = my_nrf24l01p.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));
            interpretar_msg(rxData);
            print_no_PC(rxData, rxDataCnt);
        }
    }
}
