/*
Programa que faz um LED piscar em uma KL25Z usando o transceiver a partir dos comandos
enviados de uma segunda KL25z

Pegar o nRF24L01P_hello_world, e substituir a main.cpp daquele programa por este código

Lembrar de renomear este código para main.cpp
*/

#include "mbed.h"
#include "nRF24L01P.h"

Serial pc(USBTX, USBRX); // tx, rx

nRF24L01P my_nrf24l01p(PTD2, PTD3, PTC5, PTD0, PTD5, PTA13);    // mosi, miso, sck, csn, ce, irq

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);

int main() {


    myled1 = 1;
    myled2 = 1;
    myled3 = 1;


// The nRF24L01+ supports transfers from 1 to 32 bytes, but Sparkfun's
//  "Nordic Serial Interface Board" (http://www.sparkfun.com/products/9019)
//  only handles 4 byte transfers in the ATMega code.
#define TRANSFER_SIZE   4

    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    my_nrf24l01p.powerUp();

    // Display the (default) setup of the nRF24L01+ chip
    pc.printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p.getRfFrequency() );
    pc.printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p.getRfOutputPower() );
    pc.printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p.getAirDataRate() );
    pc.printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress() );
    pc.printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress() );

    pc.printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );

    my_nrf24l01p.setTransferSize( TRANSFER_SIZE );

    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();

    while (1) {

        // If we've received anything over the host serial link...
        if ( pc.readable() ) {

            // ...add it to the transmit buffer
            txData[txDataCnt++] = pc.getc();

            // If the transmit buffer is full
            if ( txDataCnt >= sizeof( txData ) ) {

                // Send the transmitbuffer via the nRF24L01+
                my_nrf24l01p.write( NRF24L01P_PIPE_P0, txData, txDataCnt );

                txDataCnt = 0;
            }
        }

        // If we've received anything in the nRF24L01+...
        if ( my_nrf24l01p.readable() ) {

            // ...read the data into the receive buffer
            rxDataCnt = my_nrf24l01p.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
            if (rxData[3] == 'p')

                myled2 = !myled2;
            }
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {

                pc.putc( rxData[i] );
            }
        }
    }
}
