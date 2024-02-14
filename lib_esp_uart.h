/*******************************************************************************
* lib_esp_uart is a light and easy to use library for ESP32 / esp-idf devices,
* it implimets a simple function set to communicate with UART devices and ports
*
* This program is distributed in the hope that it will be useful, but 
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE.
* You should have received a copy of the GNU General Public License along with
* this program; if not, see www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*
* ADBeta (c) 2024    Ver 0.4.11    14 Feb 2024
*******************************************************************************/
#ifndef LIB_ESP_UART
#define LIB_ESP_UART

#include "driver/uart.h"

/*** Structs ******************************************************************/
//UART Handler struct used for hardware level control
typedef struct {
	uart_port_t port;
	int tx_io;
	int rx_io;
	uint32_t baudrate;
	int rx_buff;
	int tx_buff;
	uint16_t timeout;        //Timeout before giving up reading (milliseconds)
} uart_handler_t;

/*** Functions ****************************************************************/
//Initialises a UART port, returns error if not successful
esp_err_t UART_Init(const uart_handler_t *uart);

//Prints a null terminated string to the UART port.
//PrintNewline sends \n\r after the string.
//Returns total number of bytes sent
int UART_Print(const uart_handler_t *uart, const char *str);
int UART_PrintNewline(const uart_handler_t *uart, const char *str);

//Reads a string from a UART Port into a buffer string
//Takes a UART Handler, a pointer to a string buffer, and a max length
void UART_ReceiveString(const uart_handler_t *uart, char *str, const size_t len);

//Sends a request string to the UART, and will wait until it responds for
//[waits] number of cycles. Each cycle waits for uart->timeout length.
//Will exit when the buffer is full or a response is given.
//NOTE: DO not use, this is in testing and development
void UART_WaitForResponse(const uart_handler_t *uart, const char *req, 
                            char *resp, const size_t len, const size_t waits);

#endif
