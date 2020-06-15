#include "Arduino.h"

boolean hexString2string(String input, char* output);

void hex_string2string(unsigned char *input, int n_input,char *output);

void ascii_string2Hex_string(char *input, unsigned int n_input, char *output);

String string2HexString(unsigned char* input, unsigned int n_bytes);

boolean is_valid_hex_String(String line);

boolean isValidHex_string(char *line, unsigned int line_length);

int SerialReadHexDigit(byte c); //CosineKitty

unsigned char nibble2Ascii(unsigned char nibble); 

void hexChar2Ascii(char input, char *output);

boolean are_stringsEqual(char *string1, char * string2, unsigned int n_bytes);

void copy_string(unsigned char *source, unsigned char *destination, unsigned int n_bytes);