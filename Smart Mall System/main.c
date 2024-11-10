#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include <time.h>

#define F_CPU 8000000UL
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

// LCD Control Pins on PORTH
#define LCD_RS_PIN 0  // PH0
#define LCD_RW_PIN 1  // PH1
#define LCD_EN_PIN 2  // PH2

#define KEYPAD_PORT PORTK
#define KEYPAD_DDR  DDRK
#define KEYPAD_PIN  PINK

// LCD Commands
#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_4BIT_MODE 0x28
#define LCD_DISPLAY_ON 0x0C
#define LCD_ENTRY_MODE 0x06
#define LCD_ROW1 0x80
#define LCD_ROW2 0xC0

// EEPROM Addresses
#define FIRST_FLOOR_ADDRESS 0
#define SECOND_FLOOR_ADDRESS 4
#define THIRD_FLOOR_ADDRESS 8
#define WASH_ROOM_ADDRESS 12
#define CUSTOMER_PASSWORD_ADDRESS 16
#define WASHROOM_OCCUPIED_ADDRESS 32
#define RENT_DUE_ADDRESS 40


// Additional EEPROM addresses
#define TEMP_ACCESS_CODE_ADDRESS 50
#define TEMP_ACCESS_EXPIRY_ADDRESS 54
#define GROUND_FLOOR_BASE_RATE_ADDRESS 58
#define TENANT_RENT_STATUS_ADDRESS 62
// EEPROM Addresses
#define CUSTOMER_PASSWORD_ADDRESS 16
#define TEMP_ACCESS_TIMEOUT_ADDRESS 48


int rentDue = 0;
uint8_t washroomStatus = 0;



// Variables
uint32_t firstFloor = 2000000;
uint32_t secondFloor = 1500000;
uint32_t thirdFloor = 1000000;
uint32_t accessStartTime;
unsigned char washroomPassword[] = "1234";
unsigned char customerPassword[] = "256";
uint16_t people_count = 0;
uint8_t motion_detected = 0;
char count_str[16];
// Global variables for interrupt handling
volatile uint8_t keypad_scan_required = 0;
volatile uint8_t key_pressed = 0;
volatile uint8_t process_key = 0;
unsigned char tempAccessCode[5] = "";
uint8_t tempAccessTimeout = 0; // Timeout in seconds
volatile uint8_t timeoutActive = 0;

// USART Initialization
void USART_Init() {
	UBRR1H = (unsigned char)(BAUD_PRESCALE >> 8);
	UBRR1L = (unsigned char)(BAUD_PRESCALE);
	UCSR1B = (1 << RXEN1) | (1 << TXEN1);
	UCSR1C = (1 << UCSZ10) | (1 << UCSZ11);
}

void USART_Transmit(const char *data) {
	while (*data) {
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = *data++;
	}
}

unsigned char USART_ReceiveChar() {
	while (!(UCSR1A & (1 << RXC1)));
	return UDR1;
}

int USART_ReceiveInt() {
	char buffer[10];
	int i = 0;
	while (1) {
		char c = USART_ReceiveChar();
		if (c == '\r' || c == '\n') {
			buffer[i] = '\0';
			break;
		}
		buffer[i++] = c;
	}
	return atoi(buffer);
}

void set_configurations() {
	eeprom_update_block(&firstFloor, (void*)FIRST_FLOOR_ADDRESS, sizeof(firstFloor));
	eeprom_update_block(&secondFloor, (void*)SECOND_FLOOR_ADDRESS, sizeof(secondFloor));
	eeprom_update_block(&thirdFloor, (void*)THIRD_FLOOR_ADDRESS, sizeof(thirdFloor));
	eeprom_update_block(washroomPassword, (void*)WASH_ROOM_ADDRESS, strlen((char*)washroomPassword) + 1);
	eeprom_update_block(customerPassword, (void*)CUSTOMER_PASSWORD_ADDRESS, strlen((char*)customerPassword) + 1);
	eeprom_update_byte((uint8_t*)WASHROOM_OCCUPIED_ADDRESS, washroomStatus);
	eeprom_update_byte((uint8_t*)RENT_DUE_ADDRESS, rentDue);
}

// LCD Functions
void lcd_command(unsigned char cmd) {
	PORTH &= ~(1 << LCD_RS_PIN);
	PORTH &= ~(1 << LCD_RW_PIN);
	PORTJ = cmd;
	PORTH |= (1 << LCD_EN_PIN);
	_delay_us(1);
	PORTH &= ~(1 << LCD_EN_PIN);
	_delay_ms(2);
}

void lcd_data(unsigned char data) {
	PORTH |= (1 << LCD_RS_PIN);
	PORTH &= ~(1 << LCD_RW_PIN);
	PORTJ = data;
	PORTH |= (1 << LCD_EN_PIN);
	_delay_us(1);
	PORTH &= ~(1 << LCD_EN_PIN);
	_delay_ms(2);
}

void lcd_init(void) {
	DDRH |= (1 << LCD_RS_PIN) | (1 << LCD_RW_PIN) | (1 << LCD_EN_PIN);
	DDRJ = 0xFF;
	_delay_ms(20);
	lcd_command(0x38);
	_delay_ms(5);
	lcd_command(0x38);
	_delay_ms(1);
	lcd_command(0x38);
	lcd_command(LCD_DISPLAY_ON);
	lcd_command(LCD_CLEAR);
	_delay_ms(2);
	lcd_command(LCD_ENTRY_MODE);
}

void lcd_string(const char *str) {
	while (*str) {
		lcd_data(*str++);
	}
}

void lcd_clear(void) {
	lcd_command(LCD_CLEAR);
	_delay_ms(2);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
	uint8_t address;
	if (row == 0) {
		address = LCD_ROW1;
		} else {
		address = LCD_ROW2;
	}
	address += col;
	lcd_command(address);
}

// ISR for PIR Sensor (External Interrupt)
// ISR for PIR Sensor (External Interrupt)
ISR(INT0_vect) {
	if (PIND & (1 << PD0)) {
		if (!motion_detected) {
			people_count++;
			motion_detected = 1;
			
			// Set up motor pins as outputs
			DDRL |= (1 << PL0) | (1 << PL1);  // Set PL0 and PL1 as outputs
			
			// Turn on both motors
			PORTL |= (1 << PL0) | (1 << PL1);  // Set PL0 and PL1 high
			
			lcd_clear();
			lcd_set_cursor(0, 0);
			lcd_string(" Motion Detected!");
			lcd_set_cursor(1, 0);
			sprintf(count_str, "Total Count: %d", people_count);
			lcd_string(count_str);
			
			// Optional: Add delay for motor rotation
			_delay_ms(5000);  // Motors run for 1 second
			
			// Turn off motors
			PORTL &= ~((1 << PL0) | (1 << PL1));  // Set PL0 and PL1 low
		}
		} else {
		motion_detected = 0;
	}
}

// Timer1 initialization for 1-second intervals
void Timer1_Init() {
	TCCR1B |= (1 << WGM12); // CTC mode
	TIMSK1 |= (1 << OCIE1A); // Enable Timer1 Compare Match A interrupt
	OCR1A = 7812; // 1-second interrupt at 8MHz with 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10); // Set 1024 prescaler
	sei(); // Enable global interrupts
}

// Timer1 ISR for timeout handling
ISR(TIMER1_COMPA_vect) {
	if (timeoutActive && tempAccessTimeout > 0) {
		tempAccessTimeout--;
		if (tempAccessTimeout == 0) {
			timeoutActive = 0; // Stop the timeout
			tempAccessCode[0] = '\0'; // Clear the access code
			USART_Transmit("\nTemporary access expired. Access denied.\r\n");
		}
	}
}

// Motor control functions
void Motor_Start() {
	PORTE |= (1 << PE7); // Set PE7 high to start the motor
}

void Motor_Stop() {
	PORTE &= ~(1 << PE7); // Set PE7 low to stop the motor
}


#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

// LCD Commands for the second LCD
#define LCD2_CLEAR 0x01
#define LCD2_HOME 0x02
#define LCD2_ENTRY_MODE 0x06
#define LCD2_DISPLAY_ON 0x0C
#define LCD2_8BIT_MODE 0x38

// Function prototypes for the second LCD
void lcd2_init();
void lcd2_command(unsigned char command);
void lcd2_display(char *str);
void lcd2_clear();

//Main tenant management function
// void tenant_management_system() {
// 	unsigned char enteredPass[5] = {0};
// 
// 	// Configure PE7 as output for motor control
// 	DDRE |= (1 << PE7); // Set PE7 as output
// 
// 	// Initialize the second LCD
// 	lcd2_init();
// 
// 	while (1) {
// 		USART_Transmit("\nTenant Management System Menu\r\n");
// 		USART_Transmit("1. Pay Rent\r\n2. Access Washroom\r\n3. Assign Temporary Access\r\n4. Check Rent Status\r\n5. View Floor Rates\r\n7. Exit System\r\n");
// 		USART_Transmit("Enter your choice: ");
// 		int choice = USART_ReceiveChar() - '0';
// 
// 		switch (choice) {
// 			case 1:
// 			USART_Transmit("\nRENT PAYMENT SECTION\r\nEnter amount to pay: ");
// 			uint32_t rentAmount = USART_ReceiveInt();
// 			if (rentAmount > 0) {
// 				rentDue = 0;
// 				eeprom_update_byte((uint8_t*)RENT_DUE_ADDRESS, rentDue);
// 				USART_Transmit("Rent payment processed successfully.\r\n");
// 				} else {
// 				USART_Transmit("Invalid amount entered.\r\n");
// 			}
// 			break;
// 
// 			case 2:
// 			USART_Transmit("\nWASHROOM ACCESS\r\nEnter washroom password: ");
// 			for (int i = 0; i < 4; i++) {
// 				enteredPass[i] = USART_ReceiveChar();
// 				USART_Transmit("*");
// 			}
// 			enteredPass[4] = '\0';
// 
// 			if (strcmp((char*)enteredPass, (char*)washroomPassword) == 0 ||
// 			(timeoutActive && strcmp((char*)enteredPass, (char*)tempAccessCode) == 0)) {
// 				
// 				if (timeoutActive && strcmp((char*)enteredPass, (char*)tempAccessCode) == 0) {
// 					USART_Transmit("\nClient access granted.\r\n");
// 					} else {
// 					USART_Transmit("\nAccess granted.\r\n");
// 				}
// 
// 				// Display message on the second LCD and activate motor
// 				lcd2_clear();
// 				lcd2_display(" Access Washroom");
// 
// 				// Start the motor for washroom access
// 				PORTE |= (1 << PE7); // Motor ON
// 				_delay_ms(6000);     // Motor runs for 3 seconds (adjust as needed)
// 				PORTE &= ~(1 << PE7); // Motor OFF
// 				USART_Transmit("\nMotor stopped.\r\n");
// 
// 				} else {
// 				USART_Transmit("\nInvalid access code.\r\n");
// 			}
// 			break;
// 
// 			case 3:
// 			USART_Transmit("\nASSIGN TEMPORARY ACCESS\r\nEnter a 4-digit access code: ");
// 			for (int i = 0; i < 4; i++) {
// 				tempAccessCode[i] = USART_ReceiveChar();
// 				USART_Transmit("*");
// 			}
// 			tempAccessCode[4] = '\0';
// 			USART_Transmit("\nEnter timeout in seconds: ");
// 			char timeoutStr[4];
// 			int i = 0;
// 			while ((timeoutStr[i] = USART_ReceiveChar()) != '\r') {
// 				USART_Transmit(timeoutStr[i]);
// 				i++;
// 			}
// 			timeoutStr[i] = '\0';
// 			tempAccessTimeout = atoi(timeoutStr);
// 			timeoutActive = 1;
// 			USART_Transmit("\nTemporary access assigned.\r\n");
// 			break;
// 
// 			case 4:
// 			if (rentDue) {
// 				USART_Transmit("Rent is due.\r\n");
// 				} else {
// 				USART_Transmit("Rent is paid.\r\n");
// 			}
// 			break;
// 
// 			case 5:
// 			USART_Transmit("Viewing Floor Rates...\r\n");
// 			// Implement logic for floor rates display as needed
// 			break;
// 
// 			case 7:
// 			USART_Transmit("Exiting system...\r\n");
// 			return;
// 
// 			default:
// 			USART_Transmit("Invalid option.\r\n");
// 			break;
// 		}
// 		USART_Transmit("\r\nPress any key to continue...");
// 		USART_ReceiveChar();
// 	}
// }

// Second LCD Initialization
void lcd2_init() {
	DDRB = 0xFF;         // Configure PORTB as output for data pins
	DDRA |= (1 << PA0) | (1 << PA1) | (1 << PA2); // Configure control pins as output
	_delay_ms(20);       // Wait for LCD to power up
	lcd2_command(LCD2_8BIT_MODE);  // 8-bit mode, 2 lines, 5x7 font
	lcd2_command(LCD2_DISPLAY_ON); // Display ON, cursor OFF
	lcd2_command(LCD2_CLEAR);      // Clear display
	lcd2_command(LCD2_ENTRY_MODE); // Set entry mode
}

// Function to send command to the second LCD
void lcd2_command(unsigned char command) {
	PORTB = command;      // Place command on data pins
	PORTA &= ~(1 << PA0); // RS = 0 for command
	PORTA &= ~(1 << PA1); // RW = 0 for write
	PORTA |= (1 << PA2);  // Enable pulse
	_delay_ms(1);
	PORTA &= ~(1 << PA2);
	_delay_ms(1);
}

// Function to display string on the second LCD
void lcd2_display(char *str) {
	while (*str) {
		PORTB = *str++;         // Place character on data pins
		PORTA |= (1 << PA0);    // RS = 1 for data
		PORTA &= ~(1 << PA1);   // RW = 0 for write
		PORTA |= (1 << PA2);    // Enable pulse
		_delay_ms(1);
		PORTA &= ~(1 << PA2);
		_delay_ms(1);
	}
}

// Function to clear the second LCD screen
void lcd2_clear() {
	lcd2_command(LCD2_CLEAR);
	_delay_ms(2); // Wait for command to complete
}


#include <avr/eeprom.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>


// EEPROM Addresses
#define FIRST_FLOOR_ADDRESS 0
#define SECOND_FLOOR_ADDRESS 4
#define THIRD_FLOOR_ADDRESS 8
#define RENT_DUE_ADDRESS 40



// Function to update EEPROM with floor rents
void updateFloorRatesEEPROM() {
	eeprom_update_dword((uint32_t*)FIRST_FLOOR_ADDRESS, firstFloor);
	eeprom_update_dword((uint32_t*)SECOND_FLOOR_ADDRESS, secondFloor);
	eeprom_update_dword((uint32_t*)THIRD_FLOOR_ADDRESS, thirdFloor);
}

// Transmit integer via USART
void USART_TransmitInt(int num) {
	char buffer[10]; // Buffer to hold the string representation of the integer
	itoa(num, buffer, 10); // Convert integer to string (base 10)
	USART_Transmit(buffer); // Transmit the string
}

void displayFloorRates() {
	USART_Transmit("First Floor Rate: ");
	USART_TransmitInt(firstFloor);
	USART_Transmit("\r\nSecond Floor Rate: ");
	USART_TransmitInt(secondFloor);
	USART_Transmit("\r\nThird Floor Rate: ");
	USART_TransmitInt(thirdFloor);
	USART_Transmit("\r\n");
}

// Main tenant management function
void tenant_management_system() {
	unsigned char enteredPass[5] = {0};
	uint8_t rentPaid = 0; // 0 = Not Paid, 1 = Paid

	// Configure PE7 as output for motor control
	DDRE |= (1 << PE7); // Set PE7 as output

	// Initialize the second LCD
	lcd2_init();
	
	// Initialize EEPROM with floor rates
	updateFloorRatesEEPROM();

	while (1) {
		USART_Transmit("\nTenant Management System Menu\r\n");
		USART_Transmit("1. Pay Rent\r\n2. Access Washroom\r\n3. Assign Temporary Access\r\n4. Check Rent Status\r\n5. View Floor Rates\r\n6. Set Floor Rates (Admin)\r\n7. Exit System\r\n");
		USART_Transmit("Enter your choice: ");
		int choice = USART_ReceiveChar() - '0';

		switch (choice) {
			case 1:
			USART_Transmit("\nRENT PAYMENT SECTION\r\nSelect Floor (1 for First Floor, 2 for Second Floor, 3 for Third Floor): ");
			int floorChoice = USART_ReceiveChar() - '0';
			uint32_t expectedAmount = 0;

			// Determine expected amount based on selected floor
			switch (floorChoice) {
				case 1: expectedAmount = firstFloor; break;
				case 2: expectedAmount = secondFloor; break;
				case 3: expectedAmount = thirdFloor; break;
				default:
				USART_Transmit("Invalid floor selection.\r\n");
				continue;
			}

			USART_Transmit("\nEnter amount to pay: ");
			uint32_t rentAmount = USART_ReceiveInt();
			
			if (rentAmount == expectedAmount) {
				rentPaid = 1; // Mark rent as paid
				eeprom_update_byte((uint8_t*)RENT_DUE_ADDRESS, 0); // Update rent due status in EEPROM
				USART_Transmit("Rent payment processed successfully.\r\n");
				} else {
				USART_Transmit("Incorrect amount entered. Rent payment failed.\r\n");
			}
			break;

			case 2:
			if (!rentPaid) {
				USART_Transmit("Access denied. Rent is due.\r\n");
				break;
			}
			
			USART_Transmit("\nWASHROOM ACCESS\r\nEnter washroom password: ");
			for (int i = 0; i < 4; i++) {
				enteredPass[i] = USART_ReceiveChar();
				USART_Transmit("*");
			}
			enteredPass[4] = '\0';

			if (strcmp((char*)enteredPass, (char*)washroomPassword) == 0 ||
			(timeoutActive && strcmp((char*)enteredPass, (char*)tempAccessCode) == 0)) {
				USART_Transmit("\nAccess granted.\r\n");

				// Display message on the second LCD and activate motor
				lcd2_clear();
				lcd2_display(" Access Washroom");

				// Start the motor for washroom access
				PORTE |= (1 << PE7); // Motor ON
				_delay_ms(6000);     // Motor runs for 3 seconds (adjust as needed)
				PORTE &= ~(1 << PE7); // Motor OFF
				USART_Transmit("\nMotor stopped.\r\n");

				} else {
				USART_Transmit("\nInvalid access code.\r\n");
			}
			break;

			case 3:
			USART_Transmit("\nASSIGN TEMPORARY ACCESS\r\nEnter a 4-digit access code: ");
			for (int i = 0; i < 4; i++) {
				tempAccessCode[i] = USART_ReceiveChar();
				USART_Transmit("*");
			}
			tempAccessCode[4] = '\0';
			USART_Transmit("\nEnter timeout in seconds: ");
			char timeoutStr[4];
			int i = 0;
			while ((timeoutStr[i] = USART_ReceiveChar()) != '\r') {
				USART_Transmit(timeoutStr[i]);
				i++;
			}
			timeoutStr[i] = '\0';
			tempAccessTimeout = atoi(timeoutStr);
			timeoutActive = 1;
			USART_Transmit("\nTemporary access assigned.\r\n");
			break;

			case 4:
			if (rentPaid) {
				USART_Transmit("Rent is paid.\r\n");
				} else {
				USART_Transmit("Rent is due.\r\n");
			}
			break;

			case 5:
			displayFloorRates();
			break;

			case 6: // Set Floor Rates (Admin)
			USART_Transmit("\nADMIN: SET FLOOR RATES\r\n");
			USART_Transmit("Enter First Floor Rate: ");
			firstFloor = USART_ReceiveInt();
			USART_Transmit("Enter Second Floor Rate: ");
			secondFloor = USART_ReceiveInt();
			USART_Transmit("Enter Third Floor Rate: ");
			thirdFloor = USART_ReceiveInt();
			updateFloorRatesEEPROM();
			USART_Transmit("Floor rates updated.\r\n");
			break;

			case 7:
			USART_Transmit("Exiting system...\r\n");
			return;

			default:
			USART_Transmit("Invalid option.\r\n");
			break;
		}
		USART_Transmit("\r\nPress any key to continue...");
		USART_ReceiveChar();
	}
}




int main(void) {
	lcd_init();
	USART_Init();
	Timer1_Init();
	srand(time(NULL));
	lcd2_init();
	

	
	set_configurations();
	DDRD &= ~(1 << PD0);  // Set PD0 as input
	EICRA |= (1 << ISC00);  // Trigger INT0 on any logical change
	EIMSK |= (1 << INT0);  // Enable INT0 interrupt
	sei();  // Enable global interrupts
	tenant_management_system();
	DDRL |= (1 << PL0) | (1 << PL1);  //
	
	
	
	
	//new code
	// Initialize ports
	DDRB = 0xFF;   // LCD data lines as output
	DDRA = 0xFF;   // LCD control lines as output
	DDRK = 0x0F;
	//DDRE = 0xFF;
	DDRE = DDRE |=(1<<2);
	 // Initialize ports with proper configuration
	 DDRB = 0xFF;   // LCD data lines as output
	 DDRA = 0xFF;   // LCD control lines as output
	 DDRK = 0x0F;   // Lower 4 bits as output for keypad
	 DDRK &= 0x0F;  // Upper 4 bits as input for keypad
	 PORTK |= 0xF0; // Enable pull-up resistors for keypad inputs
	 

	

	
	// Configure INT5 for rising edge trigger

	EICRB |= (1<<ISC51) | (1<<ISC50); // Rising edge trigger on INT5
	EICRA |=(1<<ISC00) | (1<<ISC01);
	EICRB |=(1<<ISC40) | (1<<ISC41)|(1<<ISC50) | (1<<ISC51);//|(1<<ISC60) | (1<<ISC61);
	
	 // Configure external interrupts
	 EIMSK |= (1<<INT4) | (1<<INT0) | (1<<INT5);
	 EICRB |= (1<<ISC51) | (1<<ISC50);
	 EICRA |= (1<<ISC00) | (1<<ISC01);
	 EICRB |= (1<<ISC40) | (1<<ISC41) | (1<<ISC50) | (1<<ISC51);

	 
	

	
	//int key;
	//char key_string[16];  // String to store the number
	while (1) {
		
	}
	return 0;
}
