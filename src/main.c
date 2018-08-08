#include "uartdrv.h"
#include "string.h"

// Define receive/transmit operation queues
DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_RX_BUFS, rxBufferQueue);
DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_TX_BUFS, txBufferQueue);

// Configuration for USART0, location 0
#define MY_UART                                     \
{                                                   \
  USART0,                                           \
  115200,                                           \
  _USART_ROUTE_LOCATION_LOC5,                       \
  usartStopbits1,                                   \
  usartNoParity,                                    \
  usartOVS16,                                       \
  false,                                            \
  uartdrvFlowControlNone,                           \
  gpioPortE,                                        \
  12,                                               \
  gpioPortE,                                        \
  13,                                               \
  (UARTDRV_Buffer_FifoQueue_t *)&rxBufferQueue,     \
  (UARTDRV_Buffer_FifoQueue_t *)&txBufferQueue      \
}

UARTDRV_HandleData_t handleData;
UARTDRV_Handle_t handle = &handleData;

uint8_t buffer_tx[64];
uint8_t buffer_rx[64];

/*
 * Switch statements only work with integers/individual characters,
 * so need to convert char array into uint32
 */
uint32_t cmd_code = '****';
char* cmd_bufferptr = (char*)&cmd_code;

int i = 0;
int cmd_flag = 0;

void callback_tx(UARTDRV_Handle_t handle,
              Ecode_t transferStatus,
              uint8_t *data,
              UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)transferStatus;
  (void)data;
  (void)transferCount;
}

void callback_rx(UARTDRV_Handle_t handle,
              Ecode_t transferStatus,
              uint8_t *data,
              UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)transferStatus;
  (void)data;
  (void)transferCount;

  // Echo user input
  UARTDRV_Transmit(handle, data, 1, callback_tx);

  // Check for complete command
  if(buffer_rx[0] != '\n' && buffer_rx[0] != '\r'){
	// Check for backspace
	if(buffer_rx[0] == '\b'){
	  i--;
	}
	else {
      cmd_bufferptr[i%4] = buffer_rx[0];
      i++;
	}
  }
  else {
	i = 0;

	// Parse commands
	switch(cmd_code) {
	  case 'tiuq':    // Commands are reversed because of little endian-ness
		cmd_flag = 1;
	    break;

	  case 'llop':
		cmd_flag = 2;
		break;

	  default:
	    break;
    }

	UARTDRV_Transmit(handle, buffer_tx, strlen(buffer_tx), callback_tx);
  }

  UARTDRV_Receive(handle, buffer_rx, 1, callback_rx);
}

int main(void) {
  UARTDRV_InitUart_t initData = MY_UART;
  UARTDRV_InitUart(handle, &initData);

  // For testing purposes only, can remove later
  sprintf(&buffer_tx[0], "Start program? (y/n): ");
  UARTDRV_Transmit(handle, buffer_tx, strlen(buffer_tx), callback_tx);
  UARTDRV_ReceiveB(handle, buffer_rx, 1);
  UARTDRV_Transmit(handle, buffer_rx, 1, callback_tx);    // Echo user input


  if(buffer_rx[0] == 'y'){
    sprintf(&buffer_tx[0], "Enter Command: ");
    UARTDRV_Transmit(handle, buffer_tx, strlen(buffer_tx), callback_tx);

	// Need this line at least once in main() to kick off the receive/callback chain
	UARTDRV_Receive(handle, buffer_rx, 1, callback_rx);
  }

  while(1){

    // Check for flags here with switch, or with interrupts?
    switch(cmd_flag) {
	  case 1:
		// call some function
		break;

	  case 2:
		// call some other function
	    break;

	  default:
		break;
	}

  }

}
