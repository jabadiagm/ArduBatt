#include "Arduino.h"
#include "D:\datos\Arduino\libraries\Functions\src\Functions.h"

boolean hexString2string(String input, char* output) {
	int n_bytes;
	int counter;
	char high;
	char low;
	if ((input.length() % 2) != 0) input = "0" + input; //input string must have even nibbles
	n_bytes = input.length() / 2;
	for (counter = 0; counter<n_bytes; counter++) { //parse bytes
		high = input.charAt(counter * 2); //high nibble
		low = input.charAt(counter * 2 + 1); //low nibble
		output[counter] = SerialReadHexDigit(high) * 16 + SerialReadHexDigit(low);
	}
}

void hex_string2string(unsigned char *input, int n_input,char *output) {
	//takes an hex string and returns an ascii string
	int counter;
	char ascii_char[2];
	for (counter = 0; counter<n_input; counter++) {
		hexChar2Ascii(input[counter], ascii_char);
		output[2 * counter] = ascii_char[0];
		output[2 * counter + 1] = ascii_char[1];
	}
}

void ascii_string2Hex_string(char *input, unsigned int n_input, char *output) {
	int counter;
	char high;
	char low;
	for (counter = 0; counter<n_input; counter++) { //parse bytes
		high = input[counter * 2]; //high nibble
		low = input[counter * 2 + 1]; //low nibble
		output[counter] = SerialReadHexDigit(high) * 16 + SerialReadHexDigit(low);
	}
}

String string2HexString(unsigned char* input, unsigned int n_bytes) {
	//returns an hex String with the first n_bytes of the input sequence
	String data = "";
	String byte_hex;
	int counter;
	for (counter = 0; counter<n_bytes; counter++) {
		byte_hex = String(input[counter], HEX);
		if (byte_hex.length()<2) data.concat('0');
		data.concat(byte_hex);
	}
	return data;
}

boolean is_valid_hex_String(String line) {
	//returns true if string is only composed by numbers and letters a-f/A-F
	int counter;
	char character;
	for (counter = 0; counter<line.length(); counter++) {
		character = line.charAt(counter);
		if (SerialReadHexDigit(character)<0) return false;
	}
	return true;
}

boolean isValidHex_string(char *line, unsigned int line_length) {
	//returns true if string is only composed by numbers and letters a-f/A-F
	int counter;
	char character;
	for (counter = 0; counter<line_length; counter++) {
		character = line[counter];
		if (SerialReadHexDigit(character)<0) return false;
	}
	return true;
}

int SerialReadHexDigit(byte c) {//CosineKitty
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	else if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	else {
		return -1;   // getting here is bad: it means the character was invalid
	}
}
unsigned char nibble2Ascii(unsigned char nibble) {
	//returns ascii from hex nibble
	if (nibble<10) return '0' + nibble;
	return 'A' + nibble - 10;
}
void hexChar2Ascii(char input, char *output) {
	char nibble;
	output[0] = nibble2Ascii((input & 0xf0) >> 4);
	output[1] = nibble2Ascii(input & 0x0F);
}

boolean are_stringsEqual(char *string1, char * string2, unsigned int n_bytes) {
	//returns true if first n_bytesare the same in both input strings
	unsigned int counter;
	for (counter = 0; counter<n_bytes; counter++) {
		if (string1[counter] != string2[counter]) return false;
	}
	return true;
}

void copy_string(unsigned char *source, unsigned char *destination, unsigned int n_bytes) {
	unsigned int counter;
	for (counter = 0; counter<n_bytes; counter++) destination[counter] = source[counter];
}