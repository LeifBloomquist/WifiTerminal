// ----------------------------------------------------------
// Hayes Emulation
// Portions of this code are adapted from Payton Byrd's Hayesduino - thanks!

#define ESCAPE_GUARD_TIME 1000
#define TIMEDOUT  -1
#define COMMAND_BUFFER_SIZE  81

// Hayes variables
boolean Modem_isCommandMode = true;
boolean Modem_isRinging = false;
boolean Modem_EchoOn = true;
boolean Modem_VerboseResponses = true;
boolean Modem_QuietMode = false;
boolean Modem_S0_AutoAnswer = false;
byte    Modem_X_Result = 0;
boolean Modem_suppressErrors = false;
boolean Modem_AT_Detected = false;
char    Modem_S2_EscapeCharacter = '+';
boolean Modem_isConnected = false;
boolean Modem_flowControl = false;   // for &K setting.
boolean Modem_isDcdInverted = true;
boolean Modem_DCDFollowsRemoteCarrier = false;    // &C
byte    Modem_dataSetReady = 0;         // &S
boolean Modem_isCtsRtsInverted = true;           // Normally true on a C64.  False for commodoreserver 38,400.
int     Modem_EscapeCount = 0;
int     Modem_EscapeTimer = 0;
boolean Modem_EscapeReceived = false;
char    Modem_LastCommandBuffer[COMMAND_BUFFER_SIZE];
char    Modem_CommandBuffer[COMMAND_BUFFER_SIZE];
char    Modem_LastCommandChar;
char    Modem_lastInputCharacter;


void EnterHayesMode()
{
  mode_Hayes = true;
  updateEEPROMByte(ADDR_HAYES_MENU, mode_Hayes);
  softSerial.println();
  softSerial.println(F("Hayes mode set.  Use AT&M to return to menu mode."));  
  HayesEmulationMode();
}


void HayesEmulationMode()
{
//  DSR, DTR, RI set here

  Modem_LoadDefaults(true);
  Modem_LoadSavedSettings();
  Modem_ResetCommandBuffer();

  delay(100);            //  Sometimes text after ShowInfo at boot doesn't appear.  Trying this..
  softSerial.println();

  // Do autoconnect here?
  /*
  if (autoConnectHost >= 1 && autoConnectHost <= ADDR_HOST_ENTRIES)
  {
    // Phonebook dial
    char address[ADDR_HOST_SIZE];

    strncpy(address, readEEPROMPhoneBook(ADDR_HOSTS + ((autoConnectHost - 1) * ADDR_HOST_SIZE)).c_str(), ADDR_HOST_SIZE);

    softSerial.print(F("Auto-connecting to:\r\n"));
    softSerial.println(address);

    softSerial.println(F("\r\nPress any key to cancel..."));
    // Wait for user to cancel

    int option = PeekByte(softSerial, 2000);

    if (option != TIMEDOUT)   // Key pressed
    {
      ReadByte(softSerial);    // eat character
      Display(("OK"), true, 0);
      softSerial.print(F("OK"));
      softSerial.println();
    }
    else
      Modem_Dialout(address);
  }
  else
  {
  */

  Modem_PrintOK();

  while (true)
  {
    Modem_Loop();    // If Menu mode is re-selected, ESP is rebooted!  See handling for &M
  }
}


inline void Modem_PrintOK()
{
  Modem_PrintResponse(0, ("OK"));
}

inline void Modem_PrintERROR()
{
  Modem_PrintResponse(4, ("ERROR"));
}

/* Modem response codes should be in ASCII.  */
void Modem_PrintResponse(byte code, String msg)
{
  // DEBUG !!!!
  softSerial.println();
  softSerial.println(msg);
  return;

  if (!Modem_QuietMode)
  {
    if (Modem_VerboseResponses)
    {
      softSerial.println();
      softSerial.println(msg);
    }
    else
    {
      softSerial.println(code);
    }
  }
}

void Modem_ResetCommandBuffer()
{
  memset(Modem_CommandBuffer, 0, COMMAND_BUFFER_SIZE);
  Modem_AT_Detected = false;
}


void Modem_LoadDefaults(boolean booting)
{
  Modem_isCommandMode = true;
  Modem_isConnected = false;
  Modem_isRinging = false;
  Modem_EchoOn = true;
  //Modem_SetEcho(true);
  Modem_VerboseResponses = true;
  Modem_QuietMode = false;
  Modem_S0_AutoAnswer = false;
  Modem_S2_EscapeCharacter = '+';
  //Modem_isDcdInverted = false;
  Modem_flowControl = false;
  Modem_DCDFollowsRemoteCarrier = false;
  Modem_suppressErrors = false;

  //Modem_DCD_Set();

  if (!booting)
  {
    Modem_dataSetReady = 0;
#ifdef C64_DSR
    Modem_DSR_Set();
#endif
  }
}

void Modem_LoadSavedSettings(void)
{
  // Load saved settings
  Modem_EchoOn = EEPROM.read(ADDR_MODEM_ECHO);
  Modem_flowControl = EEPROM.read(ADDR_MODEM_FLOW);
  Modem_VerboseResponses = EEPROM.read(ADDR_MODEM_VERBOSE);
  Modem_QuietMode = EEPROM.read(ADDR_MODEM_QUIET);
  Modem_S0_AutoAnswer = EEPROM.read(ADDR_MODEM_S0_AUTOANS);
  Modem_S2_EscapeCharacter = EEPROM.read(ADDR_MODEM_S2_ESCAPE);
  //Modem_isDcdInverted = EEPROM.read(ADDR_MODEM_DCD_INVERTED);
  Modem_DCDFollowsRemoteCarrier = EEPROM.read(ADDR_MODEM_DCD);
  Modem_X_Result = EEPROM.read(ADDR_MODEM_X_RESULT);
  Modem_suppressErrors = EEPROM.read(ADDR_MODEM_SUP_ERRORS);
  Modem_dataSetReady = EEPROM.read(ADDR_MODEM_DSR);

 // Modem_DCD_Set();
}


void Modem_Disconnect(boolean printNoCarrier)
{
  /*
  //char temp[15];
  Modem_isCommandMode = true;
  Modem_isConnected = false;
  Modem_isRinging = false;
  commodoreServer38k = false;


  //if (wifly.available() == -1)
  wifly.stop();
  //else
  //wifly.closeForce();      // Incoming connections need to be force closed.  close()
  // does not work because WiFlyHQ.cpp connected variable is
  // not set for incoming connections.

  if (printNoCarrier)
    Modem_PrintResponse(3, ("NO CARRIER"));

  if (Modem_DCDFollowsRemoteCarrier)
    digitalWrite(C64_DCD, Modem_ToggleCarrier(false));

#ifdef C64_DSR
  if (Modem_dataSetReady == 1)
    digitalWrite(C64_DSR, HIGH);
#endif
    */
}

// Validate and handle AT sequence  (A/ was handled already)
void Modem_ProcessCommandBuffer()
{
  byte errors = 0;
  //boolean dialed_out = 0;
  // Phonebook dial
  char numString[2];
  char address[ADDR_HOST_SIZE];
  int phoneBookNumber;
  boolean suppressOkError = false;
  char atCommandFirstChars[] = "AEHIQVZ&*RSD\r\n";   // Used for AT commands that have variable length values such as ATS2=45, ATS2=123
  char tempAsciiValue[3];

  // Used for a/ and also for setting SSID, PASS, KEY as they require upper/lower
  strcpy(Modem_LastCommandBuffer, Modem_CommandBuffer);

  // Force uppercase for consistency 

  for (int i = 0; i < strlen(Modem_CommandBuffer); i++)
  {
    Modem_CommandBuffer[i] = toupper(Modem_CommandBuffer[i]);
  }

  // Define auto-start phone book entry
  if (strncmp(Modem_CommandBuffer, ("AT&PBAUTO="), 10) == 0)
  {
    char numString[2];
    numString[0] = Modem_CommandBuffer[10];
    numString[1] = '\0';

    int phoneBookNumber = atoi(numString);
    if (phoneBookNumber >= 0 && phoneBookNumber <= ADDR_HOST_ENTRIES)
    {
      updateEEPROMByte(ADDR_HOST_AUTO, phoneBookNumber);
    }
    else
      errors++;
  }
  // Set listening TCP port
  /*else if (strncmp(Modem_CommandBuffer, ("AT&PORT="), 8) == 0)
  {
  char numString[6];

  int localport = atoi(&Modem_CommandBuffer[8]);
  if (localport >= 0 && localport <= 65535)
  {
  if (setLocalPort(localport))
  while (1);
  WiFlyLocalPort = localport;
  }
  else
  errors++;
  }*/
  // List phone book entries
  else if (strstr(Modem_CommandBuffer, ("AT&PB?")) != NULL)
  {
    softSerial.println();
    for (int i = 0; i < ADDR_HOST_ENTRIES; i++)
    {
      softSerial.print(i + 1);
      softSerial.print(F(":"));
      softSerial.println(readEEPROMPhoneBook(ADDR_HOSTS + (i * ADDR_HOST_SIZE)));
      yield();  // For 300 baud
    }
    softSerial.println();
    softSerial.print(F("Autostart: "));
    softSerial.print(EEPROM.read(ADDR_HOST_AUTO));
    softSerial.println();
  }
  // Clear phone book
  else if (strstr(Modem_CommandBuffer, ("AT&PBCLEAR")) != NULL)
  {
    for (int i = 0; i < ADDR_HOST_ENTRIES; i++)
    {
      updateEEPROMPhoneBook(ADDR_HOSTS + (i * ADDR_HOST_SIZE), "\0");
    }
  }
  /*    else if (strstr(Modem_CommandBuffer, ("AT&PBCLEAR")) != NULL)
  {
  for (int i = 0; i < ADDR_HOST_ENTRIES - STATIC_PB_ENTRIES; i++)
  {
  updateEEPROMPhoneBook(ADDR_HOSTS + (i * ADDR_HOST_SIZE), "\0");
  }

  // To add static entries, update STATIC_PB_ENTRIES and add entries below increased x: ADDR_HOST_ENTRIES - x
  updateEEPROMPhoneBook(ADDR_HOSTS + ((ADDR_HOST_ENTRIES - 1) * ADDR_HOST_SIZE), F("WWW.COMMODORESERVER.COM:1541"));   // last entry
  updateEEPROMPhoneBook(ADDR_HOSTS + ((ADDR_HOST_ENTRIES - 2) * ADDR_HOST_SIZE), F("BBS.JAMMINGSIGNAL.COM:23"));       // second last entry

  updateEEPROMByte(ADDR_HOST_AUTO, 0);
  Modem_PrintOK();
  }*/
  // Add entry to phone book
  else if (strncmp(Modem_CommandBuffer, ("AT&PB"), 5) == 0)
  {
    char numString[2];
    numString[0] = Modem_CommandBuffer[5];
    numString[1] = '\0';

    int phoneBookNumber = atoi(numString);
    if (phoneBookNumber >= 1 && phoneBookNumber <= ADDR_HOST_ENTRIES && Modem_CommandBuffer[6] == '=')
    {
      updateEEPROMPhoneBook(ADDR_HOSTS + ((phoneBookNumber - 1) * ADDR_HOST_SIZE), Modem_CommandBuffer + 7);
    }
    else
      errors++;
  }

  else if (strncmp(Modem_CommandBuffer, ("AT"), 2) == 0)
  {
    for (int i = 2; i < strlen(Modem_CommandBuffer) && i < COMMAND_BUFFER_SIZE - 3;)
    {
      int WiFicounter = 0;
      int WiFiConnectSuccess = false;

      switch (Modem_CommandBuffer[i++])
      {
      case 'Z':   // ATZ
        Modem_LoadSavedSettings();
        //if (wifly.isSleeping())
        //    wake();
        break;

      case 'I':   // ATI
        ShowInfo(false);
        break;

      case 'A':   // ATA
        Modem_Answer();
        suppressOkError = true;
        break;

      case 'E':   // ATE
        switch (Modem_CommandBuffer[i++])
        {
        case '0':
          Modem_EchoOn = false;
          break;

        case '1':
          Modem_EchoOn = true;
          break;

        default:
          errors++;
        }
        break;

      case 'H':       // ATH
        switch (Modem_CommandBuffer[i++])
        {
        case '0':
          //if (wifly.isSleeping())
          //    wake();
          //else
          Modem_Disconnect(false);
          break;

        case '1':
          //if (!wifly.isSleeping())
          //    if (!wifly.sleep())
          //        errors++;
          break;

        default:
          i--;                        // User entered ATH
          //if (wifly.isSleeping())
          //    wake();
          //else
          Modem_Disconnect(false);
          break;
        }
        break;

      case 'O':
        if (Modem_isConnected)
        {
          Modem_isCommandMode = false;
        }
        else
          errors++;
        break;

      case 'Q':
        switch (Modem_CommandBuffer[i++])
        {
        case '0':
          Modem_QuietMode = false;
          break;

        case '1':
          Modem_QuietMode = true;
          break;

        default:
          errors++;
        }
        break;

      case 'S':   // ATS
        switch (Modem_CommandBuffer[i++])
        {
        case '0':
          switch (Modem_CommandBuffer[i++])
          {
          case '=':
            switch (Modem_CommandBuffer[i++])
            {
            case '0':
              Modem_S0_AutoAnswer = 0;
              break;

            case '1':
              Modem_S0_AutoAnswer = 1;
              break;

            default:
              errors++;
            }
            break;
          }
          break;

        case '2':
          switch (Modem_CommandBuffer[i++])
          {
          case '=':
            char numString[3] = "";

            // Find index of last character for this setting.  Expects 1-3 numbers (ats2=43, ats2=126 etc)
            int j = i;
            for (int p = 0; (p < strlen(atCommandFirstChars)) && (j <= i + 2); p++)
            {
              if (strchr(atCommandFirstChars, Modem_CommandBuffer[j]))
                break;
              j++;
            }

            strncpy(numString, Modem_CommandBuffer + i, j - i);
            numString[3] = '\0';

            Modem_S2_EscapeCharacter = atoi(numString);

            i = j;
            break;
          }
          break;

        case '9':
          switch (Modem_CommandBuffer[i++])
          {
          case '9':
            switch (Modem_CommandBuffer[i++])
            {
            case '=':
              switch (Modem_CommandBuffer[i++])
              {
              case '0':
                Modem_suppressErrors = 0;
                break;

              case '1':
                Modem_suppressErrors = 1;
                break;
              }
            }
          }
          break;
        }
        break;

      case 'V':   // ATV
        switch (Modem_CommandBuffer[i++])
        {
        case '0':
          Modem_VerboseResponses = false;
          break;

        case '1':
          Modem_VerboseResponses = true;
          break;

        default:
          errors++;
        }
        break;


        /*
        X0 = 0-4
        X1 = 0-5, 10
        X2 = 0-6, 10
        X3 = 0-5, 7, 10
        X4 = 0-7, 10

        0 - OK
        1 - CONNECT
        2 - RING
        3 - NO CARRIER
        4 - ERROR
        5 - CONNECT 1200
        6 - NO DIALTONE
        7 - BUSY
        8 - NO ANSWER
        10 - CONNECT 2400
        11 - CONNECT 4800
        etc..
        */
      case 'X':   // ATX
        Modem_X_Result = (Modem_CommandBuffer[i++] - 48);
        if (Modem_X_Result < 0 || Modem_X_Result > 4)
        {
          Modem_X_Result = 0;
          errors++;
        }

        break;

      case '&':   // AT&
        switch (Modem_CommandBuffer[i++])
        {
        case 'C':
          switch (Modem_CommandBuffer[i++])
          {
          case '0':
            Modem_DCDFollowsRemoteCarrier = false;
            //digitalWrite(C64_DCD, Modem_ToggleCarrier(true));
            break;

          case '1':
            Modem_DCDFollowsRemoteCarrier = true;
            if (Modem_isConnected) {
              //digitalWrite(C64_DCD, Modem_ToggleCarrier(true));
            }
            else {
             // digitalWrite(C64_DCD, Modem_ToggleCarrier(false));
            }
            break;

          default:
            errors++;
          }
          break;

        case 'F':   // AT&F
          Modem_LoadDefaults(false);
          //if (wifly.isSleeping())
          //    wake();

          break;

        case 'K':   // AT&K
          switch (Modem_CommandBuffer[i++])
          {
          case '0':
            Modem_flowControl = false;
            break;

          case '1':
            Modem_flowControl = true;
            break;

          default:
            errors++;
          }
          break;

          //case 'R':   // AT&R
          //    RawTerminalMode();
          //    break;

        case 'M':   // AT&M
          mode_Hayes = false;
          updateEEPROMByte(ADDR_HAYES_MENU, mode_Hayes);
          softSerial.println(F("Restarting in Menu mode."));
          softSerial.println();
          ESP.restart();
          while (1);
          break;

        case 'S':   // AT&S
          switch (Modem_CommandBuffer[i++])
          {
          case '0':
            //Modem_dataSetReady = 0;
            //break;
          case '1':
            //Modem_dataSetReady = 1;
            //break;
          case '2':
            //Modem_dataSetReady = 2;
            Modem_dataSetReady = Modem_CommandBuffer[i - 1] - '0x30';
#ifdef C64_DSR
            Modem_DSR_Set();
#endif
            break;

          default:
            errors++;
          }
          break;

        case 'W':   // AT&W
          updateEEPROMByte(ADDR_MODEM_ECHO, Modem_EchoOn);
          updateEEPROMByte(ADDR_MODEM_FLOW, Modem_flowControl);
          updateEEPROMByte(ADDR_MODEM_VERBOSE, Modem_VerboseResponses);
          updateEEPROMByte(ADDR_MODEM_QUIET, Modem_QuietMode);
          updateEEPROMByte(ADDR_MODEM_S0_AUTOANS, Modem_S0_AutoAnswer);
          updateEEPROMByte(ADDR_MODEM_S2_ESCAPE, Modem_S2_EscapeCharacter);
          updateEEPROMByte(ADDR_MODEM_DCD, Modem_DCDFollowsRemoteCarrier);
          updateEEPROMByte(ADDR_MODEM_X_RESULT, Modem_X_Result);
          updateEEPROMByte(ADDR_MODEM_SUP_ERRORS, Modem_suppressErrors);
          updateEEPROMByte(ADDR_MODEM_DSR, Modem_dataSetReady);

          //if (!(wifly.save()))
          //    errors++;
          break;

          /*case '-':   // AT&-
          softSerial.println();
          softSerial.print(freeRam());
          softSerial.println();
          break;*/
        }
        break;

      case '*':               // AT* Moving &ssid, &pass and &key to * costs 56 flash but saves 26 mimimum RAM.
        switch (Modem_CommandBuffer[i++])
        {
        case 'M':   // AT*M     Message sent to remote side when answering
          switch (Modem_CommandBuffer[i++])
          {
          case '=':
          {
            int j = 0;
            for (; j < ADDR_ANSWER_MESSAGE_SIZE - 1; j++)
            {
              EEPROM.write(ADDR_ANSWER_MESSAGE + j, (Modem_LastCommandBuffer + i)[j]);
              EEPROM.commit();
            }

            i = strlen(Modem_LastCommandBuffer);    // Consume the rest of the line.
            break;
          }

          default:
            errors++;
          }
          break;

        case 'S':   // AT*S     Set SSID
          switch (Modem_CommandBuffer[i++])
          {
          case '=':
           // WiFi.begin(Modem_LastCommandBuffer + i, SSID_passphrase.c_str());

            WiFicounter = 0;
            WiFiConnectSuccess = false;
            while (WiFicounter < 40) {
              delay(500);
              softSerial.print(".");
              if (WiFi.status() == WL_CONNECTED) {
                WiFiConnectSuccess = true;
                break;
              }
              WiFicounter++;
            }
            if (WiFiConnectSuccess == false)
              errors++;

            //wifly.setSSID(Modem_LastCommandBuffer + i);

            /*wifly.leave();
            if (!wifly.join(20000))    // 20 second timeout
            errors++;
            */

            i = strlen(Modem_LastCommandBuffer);    // Consume the rest of the line.
            break;

          default:
            errors++;
          }
          break;

        case 'P':   // AT*P     Set SSID passphrase
          switch (Modem_CommandBuffer[i++])
          {
          case '=':
            //SSID_passphrase = (String)(Modem_LastCommandBuffer + i);
            //wifly.setPassphrase(Modem_LastCommandBuffer + i);

            i = strlen(Modem_LastCommandBuffer);    // Consume the rest of the line.
            break;

          default:
            errors++;
          }
          break;
          /*
          case 'K':   // AT*K     Set SSID key for WEP
          switch (Modem_CommandBuffer[i++])
          {
          case '=':
          wifly.setKey(Modem_LastCommandBuffer + i);

          i = strlen(Modem_LastCommandBuffer);    // Consume the rest of the line.
          break;

          default:
          errors++;
          }*/
        default:
          errors++;
        }
        break;

        // Dialing should come last..
        // TODO:  Need to allow for spaces after D, DT, DP.  Currently fails.
      case 'D':   // ATD
        switch (Modem_CommandBuffer[i++])
        {
        case '\0':                          /* ATD = ATO.  Probably don't need this...' */
          if (Modem_isConnected)
          {
            Modem_isCommandMode = false;
          }
          else
            errors++;
          break;

        case 'T':
        case 'P':

          switch (Modem_CommandBuffer[i++])
          {
          case '#':
            // Phonebook dial
            numString[0] = Modem_CommandBuffer[i];
            numString[1] = '\0';

            phoneBookNumber = atoi(numString);
            if (phoneBookNumber >= 1 && phoneBookNumber <= ADDR_HOST_ENTRIES)
            {
              strncpy(address, readEEPROMPhoneBook(ADDR_HOSTS + ((phoneBookNumber - 1) * ADDR_HOST_SIZE)).c_str(), ADDR_HOST_SIZE);
              removeSpaces(address);
              Dialout(address);
              suppressOkError = 1;
            }
            else
              errors++;
            break;

          default:
            i--;
            removeSpaces(&Modem_CommandBuffer[i]);
            Dialout(&Modem_CommandBuffer[i]);
            suppressOkError = 1;
            i = COMMAND_BUFFER_SIZE - 3;    // Make sure we don't try to process any more...
            break;
          }
          break;

        case '#':
          // Phonebook dial
          numString[0] = Modem_CommandBuffer[i];
          numString[1] = '\0';

          phoneBookNumber = atoi(numString);
          if (phoneBookNumber >= 1 && phoneBookNumber <= ADDR_HOST_ENTRIES)
          {
            strncpy(address, readEEPROMPhoneBook(ADDR_HOSTS + ((phoneBookNumber - 1) * ADDR_HOST_SIZE)).c_str(), ADDR_HOST_SIZE);
            removeSpaces(address);
            Dialout(address);
            suppressOkError = 1;
          }
          else
            errors++;
          break;

        default:
          i--;        // ATD
          removeSpaces(&Modem_CommandBuffer[i]);
          Dialout(&Modem_CommandBuffer[i]);
          suppressOkError = 1;
          i = COMMAND_BUFFER_SIZE - 3;    // Make sure we don't try to process any more...
          break;
        }
        break;

      case '\n':
      case '\r':
        break;

      default:
        errors++;
      }
    }
  }

  if (!suppressOkError)           // Don't print anything if we just dialed out etc
  {
    if (Modem_suppressErrors || !errors)        // ats99=1 to disable errors and always print OK
      Modem_PrintOK();
    else if (errors)
      Modem_PrintERROR();
  }

  Modem_ResetCommandBuffer();
}

void Modem_Ring()
{
  Modem_isRinging = true;

  Modem_PrintResponse(2, ("\r\nRING"));
  if (Modem_S0_AutoAnswer != 0)
  {
#ifdef C64_RI
    digitalWrite(C64_RI, HIGH);
    delay(250);
    digitalWrite(C64_RI, LOW);
#endif
    Modem_Answer();
  }
  else
  {
#ifdef C64_RI
    digitalWrite(C64_RI, HIGH);
    delay(250);
    digitalWrite(C64_RI, LOW);
#endif
  }
}

void Modem_Connected(boolean incoming)
{
  if (Modem_X_Result == 0)
  {
    Modem_PrintResponse(1, ("CONNECT"));
  }
  else {
    switch (BAUD_RATE)
    {
    case 1200:
      Modem_PrintResponse(5, ("CONNECT 1200"));
      break;
    case 2400:
      Modem_PrintResponse(10, ("CONNECT 2400"));
      break;
    case 4800:
      Modem_PrintResponse(11, ("CONNECT 4800"));
      break;
    case 9600:
      Modem_PrintResponse(12, ("CONNECT 9600"));
      break;
    case 19200:
      Modem_PrintResponse(14, ("CONNECT 19200"));
      break;
    case 38400:
      Modem_PrintResponse(28, ("CONNECT 38400"));
      break;
    default:
      Modem_PrintResponse(1, ("CONNECT"));
    }
  }

 // if (Modem_DCDFollowsRemoteCarrier)
 //   digitalWrite(C64_DCD, Modem_ToggleCarrier(true));

#ifdef C64_DSR    
  if (Modem_dataSetReady == 1)
    digitalWrite(C64_DSR, LOW);
#endif

  //    CheckTelnet();
 // isFirstChar = true;
  //telnetBinaryMode = false;

  Modem_isConnected = true;
  Modem_isCommandMode = false;
  Modem_isRinging = false;

  if (incoming) {
    //wifly.println(F("CONNECTING..."));
    if (EEPROM.read(ADDR_ANSWER_MESSAGE))
    {
      for (int j = 0; j < ADDR_ANSWER_MESSAGE_SIZE - 1; j++)
      {
        // Assuming it was stored correctly with a trailing \0
        char temp = EEPROM.read(ADDR_ANSWER_MESSAGE + j);
        if (temp == '^')
    //      wifly.print("\r\n");
//        else
   //       wifly.print(temp);
        if (temp == 0)
          break;
      }
   //   wifly.println();
    }
  }
}

void Incoming_ProcessData()
{
  yield();

  /*
  // Modem to C64 flow
  boolean wiflyIsConnected = false; // wifly.connected();

  if (wiflyIsConnected)
  {
    // Check for new remote connection
    if (!Modem_isConnected && !Modem_isRinging)
    {
      //wifly.println(F("CONNECTING..."));

      Modem_Ring();
      return;
    }

    // If connected, handle incoming data  
    if (Modem_isConnected)
    {
      // Echo an error back to remote terminal if in command mode.
      if (Modem_isCommandMode && wifly.available() > 0)
      {
        // If we print this, remote end gets flooded with this message 
        // if we go to command mode on the C64 and remote side sends something..
        //wifly.println(F("error: remote modem is in command mode."));
      }
      else
      {
        int data;

        // Buffer for 1200 baud
        char buffer[10];
        int buffer_index = 0;

        {
          while (wifly.available() > 0)
          {
            yield();
            int data = wifly.read();

            // If first character back from remote side is NVT_IAC, we have a telnet connection.
            if (isFirstChar) {
              if (data == NVT_IAC)
              {
                isTelnet = true;
                CheckTelnetInline();
              }
              else
              {
                isTelnet = false;
              }
              isFirstChar = false;
            }
            else
            {
              if (data == NVT_IAC && isTelnet)
              {
                {
                  if (CheckTelnetInline())
                    softSerial.write(NVT_IAC);
                }

              }
              else
              {
                {
                  DoFlowControlModemToC64();
                  softSerial.write(data);
                  if (BAUD_RATE >= 9600)
                    delay(1);       // Slow things down a bit..  1 seems to work with Desterm 3.02 at 9600.
                }
              }
            }
          }
        }
      }
    }
  }
  else  // !wiflyIsConnected
  {
    // Check for a dropped remote connection while ringing
    if (Modem_isRinging)
    {
      Modem_Disconnect(true);
      return;
    }

    */

    // Check for a dropped remote connection while connected
    if (Modem_isConnected)
    {
      Modem_Disconnect(true);
      return;
    }
  
}

void Modem_ProcessData()
{
  while (softSerial.available() > 0)
  {
    yield();

      // Command Mode -----------------------------------------------------------------------
      if (Modem_isCommandMode)
      {
        char inbound = softSerial.read();

        // Block non-ASCII/PETSCII characters
        //unsignedInbound = (unsigned char)inbound;

        // Do not delete this.  Used for troubleshooting...
        //char temp[5];
        //sprintf(temp, "-%d-",unsignedInbound);
        //softSerial.write(temp);
        
        /* What is this code??
        if (unsignedInbound == 0x08 || unsignedInbound == 0x0a || unsignedInbound == 0x0d || unsignedInbound == 0x14) {}  // backspace, LF, CR, C= Delete
        else if (unsignedInbound <= 0x1f)
          break;
        else if (unsignedInbound >= 0x80 && unsignedInbound <= 0xc0)
          break;
        else if (unsignedInbound >= 0xdb)
          break;
        */

        if (Modem_EchoOn)
        {
          softSerial.write(inbound);
        }

        if (IsBackSpace(inbound))
        {
          if (strlen(Modem_CommandBuffer) > 0)
          {
            Modem_CommandBuffer[strlen(Modem_CommandBuffer) - 1] = '\0';
          }
        }
        //else if (inbound != '\r' && inbound != '\n' && inbound != Modem_S2_EscapeCharacter)
        else if (inbound != '\r' && inbound != '\n')
        {
          if (strlen(Modem_CommandBuffer) >= COMMAND_BUFFER_SIZE) 
          {
            //Display (F("CMD Buf Overflow"));
            Modem_PrintERROR();
            Modem_ResetCommandBuffer();
          }
          else 
          {
            // TODO:  Move to Modem_ProcessCommandBuffer?
            if (Modem_AT_Detected)
            {
              Modem_CommandBuffer[strlen(Modem_CommandBuffer)] = inbound;
            }
            else
            {
              switch (strlen(Modem_CommandBuffer))
              {
              case 0:
                switch (inbound)
                {
                case 'A':
                case 'a':
                  Modem_CommandBuffer[strlen(Modem_CommandBuffer)] = inbound;
                  break;
                }
                break;
              case 1:
                switch (inbound)
                {
                case 'T':
                case 't':
                case '/':
                  Modem_CommandBuffer[strlen(Modem_CommandBuffer)] = inbound;
                  Modem_AT_Detected = true;
                  break;
                }
                break;
              }
            }

            // Special case:  A/
            if ((toupper(Modem_CommandBuffer[0]) == 'A') && (Modem_CommandBuffer[1] == '/'))
            {
              strcpy(Modem_CommandBuffer, Modem_LastCommandBuffer);
              Modem_ProcessCommandBuffer();
              Modem_ResetCommandBuffer();    // To prevent A matching with A/ again
            }
          }
        }
        // It was a '\r' or '\n'
        else if ((toupper(Modem_CommandBuffer[0]) == 'A') && (toupper(Modem_CommandBuffer[1]) == 'T'))
        {
          Modem_ProcessCommandBuffer();
        }
        else
        {
          Modem_ResetCommandBuffer();
        }
      }

      else    // Online ------------------------------------------
      {
        if (Modem_isConnected)
        {
          char InputCharacter = softSerial.read();

          // +++ escape
          if (Modem_S2_EscapeCharacter < 128) // 128-255 disables escape sequence
          {
            if ((millis() - ESCAPE_GUARD_TIME) > Modem_EscapeTimer)
            {
              if (InputCharacter == Modem_S2_EscapeCharacter && Modem_lastInputCharacter != Modem_S2_EscapeCharacter)
              {
                Modem_EscapeCount = 1;
                Modem_lastInputCharacter = InputCharacter;
              }
              else if (InputCharacter == Modem_S2_EscapeCharacter && Modem_lastInputCharacter == Modem_S2_EscapeCharacter)
              {
                Modem_EscapeCount++;
                Modem_lastInputCharacter = InputCharacter;
              }
              else
              {
                Modem_EscapeCount = 0;
                Modem_EscapeTimer = millis();   // Last time non + data was read
              }
            }
            else
            {
              Modem_EscapeTimer = millis();   // Last time data was read
            }


            if (Modem_EscapeCount == 3) 
            {
              Modem_EscapeReceived = true;
              Modem_EscapeCount = 0;
              Modem_EscapeTimer = 0;
              Modem_isCommandMode = true;
              softSerial.println();
              Modem_PrintOK();
            }
          }

          Modem_lastInputCharacter = InputCharacter;

          // If we are in telnet binary mode, write and extra 255 byte to escape NVT
      //    if ((unsigned char)C64input == NVT_IAC && telnetBinaryMode)
      //      wifly.write(NVT_IAC);

      //    int result = wifly.write((int16_t)C64input);
        }
      }
    
  }
}

void Modem_Answer()
{
  if (!Modem_isRinging)    // If not actually ringing...
  {
    Modem_Disconnect(true);  // This prints "NO CARRIER"
    return;
  }

  Modem_Connected(true);
}

// Main processing loop for the virtual modem.
void Modem_Loop()
{
  yield();

  // Incoming to Terminal Flow
  Incoming_ProcessData();

  // Terminal to Modem flow
  Modem_ProcessData();
}

// Check for the <pause>+++ sequence.  It's handled differently between Menu and Hayes, so this merely returns true/false.
bool CheckEscape(char InputCharacter)
{
    // Debug
    Modem_S2_EscapeCharacter = '+';

    if (Modem_S2_EscapeCharacter < 128) // 128-255 disables escape sequence
    {
        if ((millis() - ESCAPE_GUARD_TIME) > Modem_EscapeTimer)                                                           // 1. Guard Time has elapsed
        {
            if ((InputCharacter == Modem_S2_EscapeCharacter) && (Modem_lastInputCharacter != Modem_S2_EscapeCharacter))   // 2. Start of sequence
            {
                Modem_EscapeCount = 1;
                Modem_lastInputCharacter = InputCharacter;
                return false;
            }
            else if (InputCharacter == Modem_S2_EscapeCharacter && Modem_lastInputCharacter == Modem_S2_EscapeCharacter)  // 3. Sequence continues
            {
                Modem_lastInputCharacter = InputCharacter;
                Modem_EscapeCount++;                                

                if (Modem_EscapeCount == 3)                                                                               // 3a. Sequence complete if three!
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else                                                                                                           // 4. Reset sequence on other character
            {
                Modem_EscapeCount = 0;
                Modem_EscapeTimer = millis();   // Last time non + data was read
                return false;
            }
        }
        else
        {
            Modem_EscapeTimer = millis();   // Last time data was read
            return false;
        }

       
            /*
            Modem_EscapeReceived = true;
            Modem_EscapeCount = 0;
            Modem_EscapeTimer = 0;
            Modem_isCommandMode = true;
            softSerial.println();
            Modem_PrintOK();
            */
    }

    return false;
}
