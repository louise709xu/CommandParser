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

uint8_t txBuffer[64];
uint8_t rxBuffer[64];

uint32_t cmmd_code = 'tiuq';
char* commandBuffer_ptr = (char*)&cmmd_code;
int i = 0;

void callback_TX(UARTDRV_Handle_t handle,
              Ecode_t transferStatus,
              uint8_t *data,
              UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)transferStatus;
  (void)data;
  (void)transferCount;
}

void callback_RX(UARTDRV_Handle_t handle,
              Ecode_t transferStatus,
              uint8_t *data,
              UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)transferStatus;
  (void)data;
  (void)transferCount;

  UARTDRV_Transmit(handle, data, 1, callback_TX);

  if(rxBuffer[0] != '\n' && rxBuffer[0] != '\r'){
    commandBuffer_ptr[i%4] = rxBuffer[0];
    i++;
  }
  else {
	i = 0;

	switch(cmmd_code) {
	  case 'tiuq':
	    UARTDRV_Transmit(handle, txBuffer, strlen(txBuffer), callback_TX);
	    break;

	  default:
	    break;
    }
  }

  UARTDRV_Receive(handle, rxBuffer, 1, callback_RX);
}

int main(void) {
  UARTDRV_InitUart_t initData = MY_UART;
  UARTDRV_InitUart(handle, &initData);

  sprintf(&txBuffer[0], "Start program? (y/n): ");
  UARTDRV_Transmit(handle, txBuffer, strlen(txBuffer), callback_TX);

  UARTDRV_ReceiveB(handle, rxBuffer, 1);
  UARTDRV_Transmit(handle, rxBuffer, 1, callback_TX);


  if(rxBuffer[0] == 'y'){
    sprintf(&txBuffer[0], "Enter Command: ");
    UARTDRV_Transmit(handle, txBuffer, strlen(txBuffer), callback_TX);
	UARTDRV_Receive(handle, rxBuffer, 1, callback_RX);
  }

  while(1){
    //UARTDRV_ReceiveB(handle, rxBuffer, 1);
	//UARTDRV_Transmit(handle, rxBuffer, 1, callback_TX);
  }

}
