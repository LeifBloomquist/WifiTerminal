// Change baud rate

void ChangeBaudRateMenu()
{
    while (true)
    {
        softSerial.print
            (F("\r\n"
            "1. Set Baud Rate (Manual)\r\n"
            "2. Autodetect Baud Rate\r\n"
            "3. Return to Configuration Menu\r\n"
            "\r\n"
            "Select: "));

        int option = ReadByte(softSerial);
        softSerial.println((char)option);   // Echo back

        switch (option)
        {
            case '1':
                ChangeBaudRate();
                return;

            case '2':
                AutoBaudRate();
                return;

            case '3':
                return;

            default:
                softSerial.println(F("Unknown option, try again"));
                break;
        }
    }
}

void SetNewBaudRate(unsigned int baud)
{
    if (CheckBaudRate(baud))
    {
        updateEEPROMInteger(ADDR_BAUD_LO, baud);

        softSerial.print(F("\r\n\r\nBaud Rate changed to "));
        softSerial.print(baud);
        softSerial.println(F(".  Rebooting..."));
        delay(1000);
        ESP.restart();
        while (1);
    }
}


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
            SetNewBaudRate(baud);
            return;
        }
        else
        {
            softSerial.println();
            softSerial.print(F("Unsupported Baud Rate"));
        }
    }
}

// function to return valid received baud rate - !!! Not working yet
void AutoBaudRate()
{
    softSerial.print(F("Autodetecting Baud Rate (press 'u'): "));

    long baud = detRate(RX_PIN);

    if (CheckBaudRate(baud))
    {
        SetNewBaudRate(baud);
        return;
    }
    else
    {
        if (baud == -1)
        {
            softSerial.println(F("Timeout"));
            return;
        }
        else if (baud == -2)
        {
            softSerial.println(F("Invalid Baud Rate"));
            return;
        }
    }
}

long detRate(int recpin)  // function to return valid received baud rate
{
    unsigned long timer;
    timer = millis();

    while (1)
    {
        if ((millis() - 10000) < timer)  // was 3000
            return (-1); // Timeout
        if (digitalRead(recpin) == 0)
            break;
    }
    long baud;
    long rate = pulseIn(recpin, LOW); // measure zero bit width from character. ASCII 'U' (01010101) provides the best results.

    //if (rate < 12)
    //baud = 115200;
    //else if (rate < 20)
    //baud = 57600;
    //else 
    if (rate < 29)
        baud = 38400;
    //else if (rate < 40)
    //baud = 28800;
    else if (rate < 60)
        baud = 19200;
    //else if (rate < 80)
    //baud = 14400;
    else if (rate < 150)
        baud = 9600;
    else if (rate < 300)
        baud = 4800;
    else if (rate < 600)
        baud = 2400;
    else if (rate < 1200)
        baud = 1200;
    else
        baud = -2;  // Invalid
    return baud;
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