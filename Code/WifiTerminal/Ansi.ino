
#define ESC  27

void AnsiClearScreen(Stream& terminal)
{
  // Clear ^[[2J
  terminal.write(ESC);
  terminal.write("[2J");

  // Home ^[[H
  terminal.write(ESC);
  terminal.write("[H");
}

void AnsiNormal(Stream& terminal)
{
  AnsiTextFormat(terminal, "0");
}

void AnsiBold(Stream& terminal)
{
  AnsiTextFormat(terminal, "1");
}

void AnsiUnderline(Stream& terminal)
{
  AnsiTextFormat(terminal, "4");
}

void AnsiBlink(Stream& terminal)
{
  AnsiTextFormat(terminal, "5");
}

void AnsiReverse(Stream& terminal)
{
  AnsiTextFormat(terminal, "7");
}

/*
  0	normal display
  1	bold
  4	underline(mono only)
  5	blink on
  7	reverse video on
*/

void AnsiTextFormat(Stream& terminal, String code)
{
  // ESC[#(; #)m
  terminal.write(ESC);
  terminal.write("[");
  terminal.write(code.c_str());
  terminal.write("m");
}

