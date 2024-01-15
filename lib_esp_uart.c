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
	
	//Make sure RX and TX Buffer are greater than UART_HW_FIFO_LEN. TX Buffer 
	//is allowed to be 0 if specified
	const int min_size = UART_HW_FIFO_LEN(uart->port) + 1;
	int rx_buff_cpy, tx_buff_cpy;
	
	rx_buff_cpy = (uart->rx_buff < min_size) ? min_size : uart->rx_buff;
	tx_buff_cpy = (uart->tx_buff && uart->tx_buff < min_size) ? min_size 
	                                                            : uart->tx_buff;
	
	//If the driver is already installed return OK
	if(uart_is_driver_installed(uart->port)) return ESP_OK;
	
	//Init the driver & config
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
	//Terminate the string
	str[bytes_read] = '\0';
}
