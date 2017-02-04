// NFC

#include <PN532_HSU.h>
#include <PN532.h>

PN532_HSU pn532hsu(Serial1);
PN532 nfc(pn532hsu);

// Displayi & buttons

#include <LiquidCrystal.h>
#include <stdio.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int lcd_key			= 0;
int adc_key_in		= 0;

#define btnRIGHT		0
#define btnUP			1
#define btnDOWN			2
#define btnLEFT			3
#define btnSELECT		4
#define btnNONE			5

int btnLAST = -1;

int read_LCD_buttons(){
	adc_key_in = analogRead(0);

	if (adc_key_in > 1000) return btnNONE;

	if (adc_key_in < 68) return btnRIGHT;  
	if (adc_key_in < 150) return btnUP; 
	if (adc_key_in < 310) return btnDOWN; 
	if (adc_key_in < 460) return btnLEFT; 
	if (adc_key_in < 690) return btnSELECT;	

	return btnNONE;
}

// Menu

#define FUNCTION		0
#define SUBMENU			1

typedef struct menuItem {
	char text[16];

	int type;

	menuItem *next;
	menuItem *previous;
	
	menuItem *submenu;

	void (*function)();
} menuItem;

menuItem *stack[8];
int currentItem = 0;

menuItem *current;
menuItem *root;

menuItem *initializeMenuItem(menuItem *previous = NULL) {
	menuItem *item = malloc(sizeof(menuItem));
	
	item->next = NULL;
	item->previous = previous;

	return item;
}

void pushItem(menuItem *item = NULL) {
	stack[currentItem++] = item;
}

menuItem *popItem(menuItem *item = NULL) {
	return stack[--currentItem];
}

void printMenu(){
	clearScreen();
	
	lcd.setCursor(0, 0);
	lcd.print(">");
	
	lcd.print(current->text);

	if(current->next){
		lcd.setCursor(1,1);
		lcd.print(current->next->text);
	}
}

void clearScreen(){
	lcd.setCursor(0, 0);

	for(int i = 0; i < 64; i++){
		lcd.print(" ");
	}

	lcd.setCursor(0, 0);
};

// Programs

void nfcID(){
	lcd.setCursor(0,0);

	clearScreen();

	lcd.print("Waiting for card");

	nfc.begin();

	uint32_t versiondata = nfc.getFirmwareVersion();
	if (! versiondata) {
		return;
	}

	nfc.SAMConfig();

	while(true){
		uint8_t success; // Flag to check if there was an error with the PN532
		uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // Buffer to store the returned UID
		uint8_t uidLength; // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

		success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

		if (success) {
			clearScreen();

			lcd.setCursor(0,0);
			lcd.print("LEN: ");

			lcd.print(uidLength);

			lcd.setCursor(0,1);

			for (uint8_t i = 0; i < uidLength; i++) {
				lcd.print(uid[i], HEX);
				lcd.print(' ');
			}

			delay(2000);
			break;
		}

		delay(100);
	}
};

// Arduino

void setup(){
	lcd.begin(16, 2);
	Serial.begin(9600);

	//Add first one

	current = initializeMenuItem();
	sprintf(current->text, "NFC");

	current->type = SUBMENU;
	current->submenu = initializeMenuItem();

	sprintf(current->submenu->text, "NFC ID");
	current->submenu->type = FUNCTION;
	current->submenu->function = nfcID;


	root = current;

	//Add one more

	current->next = initializeMenuItem(current);
	current = current->next;
	
	sprintf(current->text, "Second one");

	// Added one more

	//Add one more

	current->next = initializeMenuItem(current);
	current = current->next;
	
	sprintf(current->text, "Third one");

	// Added one more
	
	current = root;
	printMenu();
}

void loop(){	
	lcd_key = read_LCD_buttons();

	if(btnLAST != lcd_key){
		btnLAST = lcd_key;
		
		switch (lcd_key){
			case btnRIGHT:{
				if(current->type == FUNCTION){
					current->function();
				}

				if(current->type == SUBMENU){
					pushItem(current);
					current = current->submenu;
				}

				break;
			} case btnLEFT: {
				if(currentItem > 0){
					current = popItem();
				}

				break;
			} case btnUP: {
				if(current->previous){
					current = current->previous;
				}

				break;
			} case btnDOWN: {
				if(current->next){
					current = current->next;
				}

				break;
			} case btnSELECT: {

				break;
			} case btnNONE: {

				break;
			}
		}
		printMenu();
	}

	delay(100);
}

