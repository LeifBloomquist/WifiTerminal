void PhoneBook()
{
  while (true)
  {
    char address[ADDR_HOST_SIZE];
    char numString[2];

    DisplayPhoneBook();

    softSerial.print(F("\r\nSelect: #, m to modify, c to clear all\r\na to set auto-start, 0 to go back: "));

    char addressChar = ReadByte(softSerial);
    softSerial.println((char)addressChar); // Echo 

    if (addressChar == 'm' || addressChar == 'M')
    {
      softSerial.print(F("\r\nEntry # to modify? (0 to abort): "));

      char addressChar = ReadByte(softSerial);

      numString[0] = addressChar;
      numString[1] = '\0';
      int phoneBookNumber = atoi(numString);

      if (phoneBookNumber >= 0 && phoneBookNumber <= ADDR_HOST_ENTRIES)
      {
        softSerial.print(phoneBookNumber);
        switch (phoneBookNumber) {
        case 0:
          break;

        default:
          softSerial.print(F("\r\nEnter address: "));
          String hostName = GetInput();
          if (hostName.length() > 0)
          {
            updateEEPROMPhoneBook(ADDR_HOSTS + ((phoneBookNumber - 1) * ADDR_HOST_SIZE), hostName);
          }
          else
            updateEEPROMPhoneBook(ADDR_HOSTS + ((phoneBookNumber - 1) * ADDR_HOST_SIZE), "");

        }
      }
    }
    else if (addressChar == 'c' || addressChar == 'C')
    {
      softSerial.print(F("\r\nAre you sure (y/n)? "));

      char addressChar = ReadByte(softSerial);

      if (addressChar == 'y' || addressChar == 'Y')
      {
        for (int i = 0; i < ADDR_HOST_ENTRIES; i++)
        {
          updateEEPROMPhoneBook(ADDR_HOSTS + (i * ADDR_HOST_SIZE), "\0");
        }
      }
    }
    else if (addressChar == 'a' || addressChar == 'A')
    {
      softSerial.print(F("\r\nEntry # to set to auto-start?\r\n""(0 to disable): "));

      char addressChar = ReadByte(softSerial);

      numString[0] = addressChar;
      numString[1] = '\0';
      int phoneBookNumber = atoi(numString);
      if (phoneBookNumber >= 0 && phoneBookNumber <= ADDR_HOST_ENTRIES)
      {
        softSerial.print(phoneBookNumber);
        updateEEPROMByte(ADDR_HOST_AUTO, phoneBookNumber);
      }

    }
    else
    {
      numString[0] = addressChar;
      numString[1] = '\0';
      int phoneBookNumber = atoi(numString);

      if (phoneBookNumber >= 0 && phoneBookNumber <= ADDR_HOST_ENTRIES)
      {
        switch (phoneBookNumber) 
        {
          case 0:
            return;

          default:
            strncpy(address, readEEPROMPhoneBook(ADDR_HOSTS + ((phoneBookNumber - 1) * ADDR_HOST_SIZE)).c_str(), ADDR_HOST_SIZE);
            removeSpaces(address);
            Dialout(address);
            break;
          }

      }
    }
  }
}

void DisplayPhoneBook() 
{
  softSerial.println();
  softSerial.println(F("Phone Book"));
  softSerial.println();

  for (int i = 0; i < ADDR_HOST_ENTRIES; i++)
  {
    softSerial.print(i + 1);
    softSerial.print(F(":"));
    softSerial.println(readEEPROMPhoneBook(ADDR_HOSTS + (i * ADDR_HOST_SIZE)));
  }
  softSerial.println();
  softSerial.print(F("Autostart: "));
  softSerial.print(EEPROM.read(ADDR_HOST_AUTO));
  softSerial.println();
}

void removeSpaces(char *temp)
{
  char *p1 = temp;
  char *p2 = temp;

  while (*p2 != 0)
  {
    *p1 = *p2++;
    if (*p1 != ' ')
      p1++;
  }
  *p1 = 0;
}

// Connect to an address in the form host:port 
void Dialout(char* host)
{
  char* index;
  uint16_t port = TELNET_DEFAULT_PORT;
  String hostname = String(host);

  if (strlen(host) == 0)
  {
    if (mode_Hayes)
    {
      Modem_PrintERROR();
    }
    return;
  }

  if ((index = strstr(host, ":")) != NULL)
  {
    index[0] = '\0';
    hostname = String(host);
    port = atoi(index + 1);
  }

  lastHost = hostname;
  lastPort = port;

  Connect(hostname, port);
}