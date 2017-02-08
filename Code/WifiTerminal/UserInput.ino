// ----------------------------------------------------------
// User Input Handling

boolean IsBackSpace(char c)
{
  if ((c == 8) || (c == 20) || (c == 127))
  {
    return true;
  }
  else
  {
    return false;
  }
}

String GetInput()
{
  String temp = GetInput_Raw();
  temp.trim();
  return temp;
}

String GetInput_Raw()
{
  char temp[80];

  int max_length = sizeof(temp);

  int i = 0; // Input buffer pointer
  char key;

  while (true)
  {
    key = ReadByte(softSerial);  // Read in one character

    if (!IsBackSpace(key))  // Handle character, if not backspace
    {
      temp[i] = key;
      softSerial.write(key);    // Echo key press back to the user

      if (((int)key == 13) || (i >= (max_length - 1)))   // The 13 represents enter key.
      {
        temp[i] = 0; // Terminate the string with 0.
        return String(temp);
      }
      i++;
    }
    else     // Backspace
    {
      if (i > 0)
      {
        softSerial.write(key);
        i--;
      }
    }

    // Make sure didn't go negative
    if (i < 0) i = 0;
  }
}


// ----------------------------------------------------------
// Helper functions for read/peek

int ReadByte(Stream& in)
{
  while (in.available() == 0)
  {
    yield();
  }
  return in.read();
}