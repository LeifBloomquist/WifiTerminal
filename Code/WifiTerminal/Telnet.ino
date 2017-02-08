// ----------------------------------------------------------
// Telnet Handling

#include "telnet.h"

void DoTelnet()
{
  int port = lastPort;

  softSerial.println();
  softSerial.print(F("Telnet host ("));
  softSerial.print(lastHost);
  softSerial.print(F("): "));

  String hostName = GetInput();

  if (hostName.length() > 0)
  {
    port = getPort();

    lastHost = hostName;
    lastPort = port;

    Connect(hostName, port);
  }
  else
  {
    if (lastHost.length() > 0)
    {
      port = getPort();

      lastPort = port;
      Connect(lastHost, port);
    }
    else
    {
      return;
    }
  }
}

int getPort(void)
{
  softSerial.println();
  softSerial.print(F("Port ("));
  softSerial.print(lastPort);
  softSerial.print(F("): "));

  String strport = GetInput();

  if (strport.length() > 0)
  {
    return(strport.toInt());
  }
  else
  {
    return(lastPort);
  }
}

void Connect(String host, int port)
{
  char temp[80];
  softSerial.println();
  softSerial.print(F("Connecting to "));
  softSerial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host.c_str(), port))
  {
    softSerial.println("Connection failed");
    return;
  }

  WiFiServer dummy(0);

  TerminalMode(client, dummy);
}


boolean CheckTelnet(bool isFirstChar, bool telnetBinaryMode, Stream& client)
{
  int inpint, verbint, optint;                        //    telnet parameters as integers

  // First time through
  if (isFirstChar)
  {
    SendTelnetParameters(client);                         // Start off with negotiating
  }

  verbint = ReadByte(client);                          // receive negotiation verb character

  if (verbint == NVT_IAC && telnetBinaryMode)
  {
    return true;                                    // Received two NVT_IAC's so treat as single 255 data
  }

  switch (verbint) {                                  // evaluate negotiation verb character
  case NVT_WILL:                                      // if negotiation verb character is 251 (will)or
  case NVT_DO:                                        // if negotiation verb character is 253 (do) or
    optint = ReadByte(client);                       // receive negotiation option character

    switch (optint) {

    case NVT_OPT_SUPPRESS_GO_AHEAD:                 // if negotiation option character is 3 (suppress - go - ahead)
      SendTelnetDoWill(verbint, optint, client);
      break;

    case NVT_OPT_TRANSMIT_BINARY:                   // if negotiation option character is 0 (binary data)
      SendTelnetDoWill(verbint, optint, client);
      telnetBinaryMode = true;
      break;

    default:                                        // if negotiation option character is none of the above(all others)
      SendTelnetDontWont(verbint, optint, client);
      break;                                      //  break the routine
    }
    break;
  case NVT_WONT:                                      // if negotiation verb character is 252 (wont)or
  case NVT_DONT:                                      // if negotiation verb character is 254 (dont)
    optint = ReadByte(client);                       // receive negotiation option character

    switch (optint) {

    case NVT_OPT_TRANSMIT_BINARY:                   // if negotiation option character is 0 (binary data)
      SendTelnetDontWont(verbint, optint, client);
      telnetBinaryMode = false;
      break;

    default:                                        // if negotiation option character is none of the above(all others)
      SendTelnetDontWont(verbint, optint, client);
      break;                                      //  break the routine
    }
    break;
  case NVT_IAC:                                       // Ignore second IAC/255 if we are in BINARY mode
  default:
    ;
  }
  return false;
}

void SendTelnetDoWill(int verbint, int optint, Stream& client)
{
  client.write(NVT_IAC);                               // send character 255 (start negotiation)
  client.write(verbint == NVT_DO ? NVT_DO : NVT_WILL); // send character 253  (do) if negotiation verb character was 253 (do) else send character 251 (will)
  client.write((int16_t)optint);
}

void SendTelnetDontWont(int verbint, int optint, Stream& client)
{
  client.write(NVT_IAC);                               // send character 255   (start negotiation)
  client.write(verbint == NVT_DO ? NVT_WONT : NVT_DONT);    // send character 252   (wont) if negotiation verb character was 253 (do) else send character254 (dont)
  client.write((int16_t)optint);
}

void SendTelnetParameters(Stream& client)
{
  client.write(NVT_IAC);                               // send character 255 (start negotiation) 
  client.write(NVT_DONT);                              // send character 254 (dont)
  client.write(34);                                    // linemode

  client.write(NVT_IAC);                               // send character 255 (start negotiation)
  client.write(NVT_DONT);                              // send character 253 (do)
  client.write(1);                                     // echo
}

// EOF