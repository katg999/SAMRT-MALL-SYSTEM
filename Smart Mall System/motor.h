/*
 * motor.h
 *
 * Created: 10/30/2024 4:46:10 PM
 *  Author: MUSILAMU
 */ 


#ifndef MOTOR_H_
#define MOTOR_H_


// LCD functions remain the same
void lcd_cmd(unsigned char cmd) {
	PORTB = cmd;
	PORTA &= ~(1<<PA0);
	PORTA &= ~(1<<PA1);
	PORTA |= (1<<PA2);
	_delay_ms(50);
	PORTA &= ~(1<<PA2);
}

void lcd_data1(unsigned char data) {
	PORTB = data;
	PORTA |= (1<<PA0);
	PORTA &= ~(1<<PA1);
	PORTA |= (1<<PA2);
	_delay_ms(50);
	PORTA &= ~(1<<PA2);
}

void lcd_init1() {
	lcd_cmd(0x38);
	lcd_cmd(0x0c);
	lcd_cmd(0x06);
	lcd_cmd(0x01);
}

void lcd_data_print(char info[]) {
	int len = strlen(info);
	for (int a = 0; a < len; a++) {
		lcd_data1(info[a]);
	}
}

void lcd_clear1() {
	lcd_cmd(0x01);
	_delay_ms(50);
}

void int_to_string(int number, char *string) {
	itoa(number, string, 10);
}

// Motor control functions
void motor_init() {
	DDRL |= (1 << PL0) | (1 << PL1);
	PORTL &= ~((1 << PL0) | (1 << PL1));
}

void motor_control(int level) {
	switch(level) {
		case 2:
		PORTL |= (1 << PL0);
		_delay_ms(2500);
		PORTL &= ~(1 << PL0);
		break;
		case 3:
		PORTL |= (1 << PL1);
		_delay_ms(2500);
		PORTL &= ~(1 << PL1);
		break;
	}
}

// Modified keypad function to be used in ISR
int keypad_scan() {
	PORTK = 0b11111110;
	_delay_ms(1); // Short delay for port settling
	if ((PINK & (1<<4))==0) return 3;
	if ((PINK & (1<<5))==0) return 6;
	if ((PINK & (1<<6))==0) return 9;
	if ((PINK & (1<<7))==0) return 100;

	PORTK = 0b11111101;
	_delay_ms(1);
	if ((PINK & (1<<4))==0) return 2;
	if ((PINK & (1<<5))==0) return 5;
	if ((PINK & (1<<6))==0) return 8;
	if ((PINK & (1<<7))==0) return 0;

	PORTK = 0b11111011;
	_delay_ms(1);
	if ((PINK & (1<<4))==0) return 1;
	if ((PINK & (1<<5))==0) return 4;
	if ((PINK & (1<<6))==0) return 7;
	if ((PINK & (1<<7))==0) return 99;

	return 200;
}

// Timer1 ISR for keypad scanning
ISR(TIMER1_COMPA_vect) {
	if (!process_key) {
		key_pressed = keypad_scan();
		if (key_pressed != 200) {
			process_key = 1;
			keypad_scan_required = 1;
		}
	}
}

// INT5 ISR remains the same
ISR(INT5_vect) {
	PORTE |= (1<<PE2);
	PORTE &= ~(1<<PE2);
}

// Function to handle keypad input
void handle_keypad_input(int key) {
	char key_string[16];  // Increased buffer size

	lcd_clear1();
	
	switch(key) {
		case 2:
		lcd_data_print("Access Level 2");
		motor_control(2);
		break;
		case 3:
		lcd_data_print("Access Level 3");
		motor_control(3);
		break;
		case 100:
		lcd_data_print("*");
		break;
		case 99:
		lcd_data_print("#");
		break;
		default:
		if(key != 200) {
			int_to_string(key, key_string);
			lcd_data_print(key_string);
		}
		break;
	}
}



#endif /* MOTOR_H_ */