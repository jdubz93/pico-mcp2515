#include <stdio.h>
#include "mcp2515/mcp2515.h"
#include "pico/stdlib.h"

#define SPI_SCK_PIN 18
#define SPI_MOSI_PIN 19
#define SPI_MISO_PIN 16
#define SPI_CS_PIN 20
#define CAN_INT_PIN 21

typedef struct {
    spi_inst_t *spi;
    uint8_t cs_pin;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint8_t sclk_pin;
    uint32_t clock_speed; 
} mcp2515_config_t;

/* Initialize MCP2515 + Initialize SPI */
mcp2515_config_t config = {
    .spi        = spi0,
    .cs_pin     = SPI_CS_PIN,
    .tx_pin     = SPI_MOSI_PIN,
    .rx_pin     = SPI_MISO_PIN,
    .sclk_pin   = SPI_SCK_PIN,
    .clock_speed = 1 * 1000 * 1000
};
MCP2515 can0(config.spi, config.cs_pin, config.tx_pin, config.rx_pin, config.sclk_pin, config.clock_speed);

struct can_frame rx;
struct can_frame tx = {
    .can_id = 0x123,
    .can_dlc = 8,
    .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06} /* remaining two bytes are padded with 0's */ 
};

/* CAN frame format (standard) */
/* Identifier: 11 bits */
/* Remote transmission request (RTR): 1 bit */
/* Identifier extension bit (IDE): 1 bit */
/* Reserved bit (r0): 1 bit */
/* Data length code (DLC): 4 bits */
/* Data field: 0 to 64 bits (8 bytes max, so 8 * DLC) */
/* CRC field: 15 bits */
/* CRC delimiter: 1 bit */
/* Acknowledge slot: 1 bit */
/* Acknowledge delimiter: 1 bit */
/* End of frame (EOF): 7 bits */
/* Intermission: 3 bits */
/***************************/
/* +---+------------+---+---+---+----+-----------------+----------+---+---+---+-------+---+  */
/* |SOF| Identifier |RTR|IDE|r0 |DLC |     Data        |   CRC    |ACK|ACK|EOF|Inter- | IFS| */
/* |   |   (11 bit) |   |   |   |(4b)| (0 to 64 bits)  |  (15 bit)|del|sl.|   |mission|    | */
/* +---+------------+---+---+---+----+-----------------+----------+---+---+---+-------+---+  */


void setup() {
    /* Initialize the UART for printf */
    stdio_init_all();

    /* Initialize interface */
    can0.reset();
    can0.setBitrate(CAN_250KBPS, MCP_16MHZ);

    /* can0.setListenOnlyMode(); */
    can0.setNormalMode();
}

int main() {
    setup();

    while (1) {
         MCP2515::ERROR err = can0.readMessage(&rx);
        if(err == MCP2515::ERROR_OK) {
            printf("New frame recv'd...\n");
            printf("ID:    %x\n", rx.can_id);
            printf("DLC:   %d\n", rx.can_dlc);
            printf("Data:  ");
            for (int i = 0; i < rx.can_dlc; i++) {
                printf("%02x ", rx.data[i]);
            }
            printf("\n");
            printf("Sending response\n");
            can0.sendMessage(&tx);
        }
    }
}

