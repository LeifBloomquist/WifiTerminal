// Change baud rate

void ChangeBaudRate()
{
    String input;

    while (true)
    {
        softSerial.println();

        softSerial.print(F("New Baud Rate: "));
        input = GetInput();

        if (input.length() == 0)  // Cancel
        {
            return;  
        }

        unsigned int baud = input.toInt();

        if (CheckBaudRate(baud))
        {
            updateEEPROMInteger(ADDR_BAUD_LO, baud);
            break;
        }

        softSerial.println();
        softSerial.print(F("Unsupported Baud Rate"));        
    }

    softSerial.println(F("\r\Baud Rate changed.  Rebooting..."));
    delay(1000);
    ESP.restart();
    while (1);
}


// Returns True if baud rate is supported
boolean CheckBaudRate(unsigned int baud)
{
    switch (baud)
    {
        case 300:
        case 1200:
        case 2400:
        case 4800:
        case 9600:
        case 19200:
        case 38400:
            return true;

        default:
            return false;
    }
}

// Override baud rate if needed (used at startup, i.e. bad EEPROM value)
unsigned int ValidateBaudRate(unsigned int baud)
{
    if (CheckBaudRate(baud))
    {
        return baud;
    }
    else
    {
        updateEEPROMInteger(ADDR_BAUD_LO, DEFAULT_BAUD_RATE);
        return DEFAULT_BAUD_RATE;
    }
}