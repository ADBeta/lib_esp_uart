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
* ADBeta (c) 2024
*******************************************************************************/
#include "lib_esp_uart.h"

#include "driver/uart.h"
#include <string.h>

/*** UART Functions ***********************************************************/
esp_err_t UART_Init(const uart_handler_t *uart)
{
	//Create a config struct
	const uart_config_t config = {
		.baud_rate = uart->baudrate,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};
	
	// Make sure RX and TX Buffer are greater than UART_HW_FIFO_LEN. TX Buffer 
	// is allowed to be 0 if specified
	const int min_size = UART_HW_FIFO_LEN(uart->port) + 1;
	int rx_buff_cpy, tx_buff_cpy;
	
	rx_buff_cpy = (uart->rx_buff < min_size) ? min_size : uart->rx_buff;
	tx_buff_cpy = (uart->tx_buff && uart->tx_buff < min_size) ? min_size 
	                                                            : uart->tx_buff;
	
	// If the driver is already installed return OK
	if(uart_is_driver_installed(uart->port)) return ESP_OK;
	
	// Init the driver & config
	esp_err_t sta = uart_driver_install(uart->port, rx_buff_cpy, tx_buff_cpy,
	                                                                0, NULL, 0);

	if(sta == ESP_OK) sta = uart_param_config(uart->port, &config);
	if(sta == ESP_OK) sta = uart_set_pin(uart->port, uart->tx_io, uart->rx_io, 
	                                    UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	
	return sta;
}

int UART_Print(const uart_handler_t *uart, const char *str)
{
	return uart_write_bytes(uart->port, str, strlen(str));
}

int UART_PrintNewline(const uart_handler_t *uart, const char *str)
{
	int bytes_sent = uart_write_bytes(uart->port, str, strlen(str));
	bytes_sent += uart_write_bytes(uart->port, "\r\n", 2);
	return bytes_sent;
}

void UART_ReceiveString(const uart_handler_t *uart, char *str, const size_t len)
{
	const size_t bytes_read = uart_read_bytes(uart->port, str, len - 1, 
	                                        uart->timeout / portTICK_PERIOD_MS);	
	// Terminate the string
	str[bytes_read] = '\0';
}

// NOTE: This is unfinished and shouldn't be used. Testing & development
void UART_WaitForResponse(const uart_handler_t *uart, const char *req, 
						     char *resp, const size_t len, const size_t waits)
{
	// Write the request string out, and wait for the TX Buffer to empty
	UART_PrintNewline(uart, req);
	uart_wait_tx_done(uart->port, uart->timeout);

	// Offset position in the string to copy response to
	size_t offset = 0;
	
	// Pointer to the next position in the string to put chars, and keep track
	// of how many free chars are left in the output string
	char *resp_ptr = resp;
	size_t chars_free = len -1;

	// Keep adding chars to the output string until it is full, or the timeout
	// is reached.
	size_t count = 0;
	while(count < waits && chars_free) {
		// Get a response from the UART, only 
		size_t resp_chars = uart_read_bytes(uart->port, resp_ptr, chars_free, 
									     uart->timeout / portTICK_PERIOD_MS);
		
		// If any chars were read, move the offset position by [n], and remove
		// [n] chars from the amount left free in the output.
		if(resp_chars) {
			offset += resp_chars;
			chars_free -= rest_chars;
			resp_ptr += sizeof(char) * offset;
		}

		printf("Count: %u\t\tBytes: %u\t\tOffset: %u\n", count, resp_bytes, offset);
		
		// Move the offset along by how many byutes were read
		++count;		
	}

	//Terminate the string
	resp[offset] = '\0';
}
