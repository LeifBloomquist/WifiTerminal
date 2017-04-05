#include "ADDR_EEPROM.h"

void updateEEPROMByte(int address, byte value)
{
  if (EEPROM.read(address) != value)
  {
    EEPROM.write(address, value);
    EEPROM.commit();
  }
}

void updateEEPROMInteger(int address, uint16 value)
{
    byte hi = value / 256;
    byte lo = value % 256;        

    updateEEPROMByte(address+0, lo);
    updateEEPROMByte(address+1, hi);
}

uint16 readEEPROMInteger(int address)
{
    return (EEPROM.read(address + 0) +
            EEPROM.read(address + 1) * 256);
}


void updateEEPROMPhoneBook(int address, String host)
{
    for (int i = 0; i < ADDR_HOST_SIZE - 2; i++)
	{
	    EEPROM.write(address + i, host.c_str()[i]);
	}
	EEPROM.commit();
}

String readEEPROMPhoneBook(int address)
{
    char host[ADDR_HOST_SIZE - 2];

    for (int i = 0; i < ADDR_HOST_SIZE - 2; i++)
    {
    host[i] = EEPROM.read(address + i);
    }
    return host;
}

void updateEEPROMString(int address, String text)
{
	for (int i = 0; i < 38; i++)
	{
		EEPROM.write(address + i, text.c_str()[i]);
	}
	EEPROM.commit();
}

String readEEPROMString(int address)
{
	char text[STRING_SIZE - 2];

    for (int i = 0; i < STRING_SIZE - 2; i++)
	{
        text[i] = EEPROM.read(address + i);
	}
    return text;
}