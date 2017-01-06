// *** Hardwarespecific functions ***
// PORTA => DB15 - DB8 with 10k Ohm Resistor
// PORTC => DB7 - DB0 with 10k Ohm Resistor

void UTFT::_hw_special_init(){ }

void UTFT::LCD_Writ_Bus(char VH,char VL, byte mode){   
	switch (mode){
// 	case 1:
// 		break;
// 	case 8:
// 		break;
	case 16:
		PORTA = VH;
		PORTC = VL;
		pulse_low(P_WR, B_WR);
//		delayMicroseconds(10000);
//		Serial.println("Testing");
//		Serial.println((VH << 8)|VL,HEX);
		break;
	// case LATCHED_16:
	// 	break;
	}
}

void UTFT::_set_direction_registers(byte mode){
	DDRA |= 0xFF;
	DDRC |= 0xFF;
}

void UTFT::_fast_fill_16(int ch, int cl, long pix){	
	long blocks;

	PORTA = ch;
	PORTC = cl;

	blocks = pix/16;
	for (int i=0; i<blocks; i++){
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);
	}
	if ((pix % 16) != 0)
		for (int i=0; i<(pix % 16); i++){
			pulse_low(P_WR, B_WR);
		}
}

void UTFT::_fast_fill_8(int ch, long pix){
	long blocks;
	PORTA = ch;
	blocks = pix/16;
	for (int i=0; i<blocks; i++){
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
	}
	if ((pix % 16) != 0)
		for (int i=0; i<(pix % 16); i++){
			pulse_low(P_WR, B_WR);pulse_low(P_WR, B_WR);
		}
}