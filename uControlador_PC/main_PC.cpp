#include "mbed.h"
#include "nRF24L01P.h"
#define TAMANHO_MSG  4

Serial pc(USBTX, USBRX); // tx, rx
nRF24L01P transceiver(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq

void setup_transceiver(nRF24L01P transceiver) {
    transceiver.powerUp();
    transceiver.setRxAddress(0xE5E5E5E5E5);
    transceiver.setRfFrequency(2448);
    transceiver.setTxAddress(0xE5E5E5E5E5);
    // Apresenta a configuração (default) do chip nRF24L01+
    pc.printf("nRF24L01+ Frequency    : %d MHz\r\n",    transceiver.getRfFrequency());
    pc.printf("nRF24L01+ Output power : %d dBm\r\n",    transceiver.getRfOutputPower());
    pc.printf("nRF24L01+ Data Rate    : %d kbps\r\n",   transceiver.getAirDataRate());
    pc.printf("nRF24L01+ TX Address   : 0x%010llX\r\n", transceiver.getTxAddress());
    pc.printf("nRF24L01+ RX Address   : 0x%010llX\r\n", transceiver.getRxAddress());
    pc.printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TAMANHO_MSG );
    transceiver.setTransferSize(TAMANHO_MSG);
    transceiver.setReceiveMode();
    transceiver.enable();
}

int transmitir_do_PC(char txData[], int txDataCnt) {
    if (txDataCnt >= TAMANHO_MSG) {
       // ...enviamos seu conteúdo via nRF24L01+
       transceiver.write(NRF24L01P_PIPE_P0, txData, txDataCnt);
       txDataCnt = 0;
    }
    return txDataCnt;
}

void print_no_PC(char msg[], int msgCnt) {
    for (int i = 0; msgCnt > 0; msgCnt--, i++) {
        pc.putc(msg[i]);
    }
    pc.printf("\n");
}

int main() {
    char txData[TAMANHO_MSG];
    int txDataCnt = 0;

    setup_transceiver(transceiver);

    while (1) {
        if (pc.readable()) {
            // le dado e transfere para buffer tx
            txData[txDataCnt++] = pc.getc();
            txDataCnt = transmitir_do_PC(txData, txDataCnt);
            print_no_PC(txData, txDataCnt);

        }
    }
}
