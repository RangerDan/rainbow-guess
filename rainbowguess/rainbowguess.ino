// Serial Prep
// Used for logging behavior to the PC
const long SERIAL_BAUD_RATE = 115200L;

// button enumerator for all button types
typedef enum  ControlType {
  B_SELECT,
  B_A,
  B_B,
  B_C,
  B_UP,
  B_DOWN,
  INVALID
};
ControlType buttonPress = INVALID;

// Controls Button Setup and Tolerances
#include <AnalogButtons.h>
const int BUTTON_PIN = A0;
void b1Click()
{  
  buttonPress = B_SELECT;  
  logToSerial("B_SELECT"); 
}

void b2Click()
{  
  buttonPress = B_A;
  logToSerial("B_A"); 
}

void b3Click()
{  
  buttonPress = B_B;
  logToSerial("B_B"); 
}

void b4Click()
{  
  buttonPress = B_C;
  logToSerial("B_C"); 
}

void b5Click()
{  
  buttonPress = B_UP;
  logToSerial("B_UP"); 
}

void b6Click()
{  
  buttonPress = B_DOWN;
  logToSerial("B_DOWN"); 
}
/*
void configure() {
	Serial.println(analogRead(BUTTON_PIN));
	delay(150);
}
*/
AnalogButtons analogButtons(BUTTON_PIN,INPUT,1);
Button b1 = Button(1010, &b1Click);
Button b2 = Button(925, &b2Click);
Button b3 = Button(830, &b3Click);
Button b4 = Button(695, &b4Click);
Button b5 = Button(512, &b5Click);
Button b6 = Button(232, &b6Click);

// LCD Prep
// Structure in this sketch adapted from www.hacktronics.com 16x2 example
#include <LiquidCrystal.h>                // LCD Library
const int LCD_PIN_RS = 12;                // rs (LCD pin 4) to Arduino pin 12
const int LCD_PIN_RW = 11;                // rw (LCD pin 5) to Arduino pin 11
const int LCD_PIN_ENABLE = 10;            // enable (LCD pin 6) to Arduino pin 10
const int LCD_PIN_D4 = 5;                 // LCD pin 15 to Arduino pin 13
const int LCD_PIN_D5 = 4;                 // LCD pins d4, d5, d6, d7 to Arduino pins 5, 4, 3, 2
const int LCD_PIN_D6 = 3;
const int LCD_PIN_D7 = 2;
const int LCD_PIN_BACKLIGHT = 13;
LiquidCrystal lcd(LCD_PIN_RS,             // Assign pins for LCD control
                  LCD_PIN_RW,
                  LCD_PIN_ENABLE,
                  LCD_PIN_D4,
                  LCD_PIN_D5,
                  LCD_PIN_D6,
                  LCD_PIN_D7);
const int LCD_COLUMNS = 20;
const int LCD_ROWS = 4;
class LcdLines {
  public:
    void SetLines(String, String, String, String);
    String lines[LCD_ROWS];
};
void LcdLines::SetLines(String s0, String s1, String s2, String s3)
{
  lines[0] = s0;
  lines[1] = s1;
  lines[2] = s2;
  lines[3] = s3;
}
LcdLines lcdLines;

// LED Strip Prep
// Structure in this sketch adapted from Pololu LED Strip example LedStripColorTester
#include <RadioShackRgbLedStrip.h>                // LED Strip library
bool PololuLedStripBase::interruptFriendly = false;
const int LED_STRIP_PIN = 9;               // Strip input is on Pin 9
const int LED_COUNT = 6;                  // Project requires 21 LEDs in 3 light increments
PololuLedStrip<LED_STRIP_PIN> ledStrip;    // Create an ledStrip object on pin X.
rgb_color colors[LED_COUNT];               // Create a buffer for holding colors addressed on LED Strip
void setColors( rgb_color* colorsToWrite);
const rgb_color C_RED(0,0,255);  
const rgb_color C_ORANGE(255,0,255); 
const rgb_color C_YELLOW(255,0,100);
const rgb_color C_GREEN(255,0,0);
const rgb_color C_INDIGO(25,255,0);
const rgb_color C_VIOLET(0,255,255);
const rgb_color C_WHITE(255,200,100);
const rgb_color C_OFF(0,0,0);

// Initialize the State Machine
typedef enum StateType {
  S_START,                // Machine is on start screen
  S_NAVIGATION,           // Machine is navigating between colors
  S_MIXER,                // Machine takes input to add/clear mixed colors on the row
  S_CONFIRM,              // Machine asks user if they are complete, can navigate away
  S_DISPLAY,              // Machine displays current mix
  S_CHECK,                // Machine is displaying results
  S_UNKNOWN               // Machine is in an unknown state (before init or bad state)
};
/* ------------------------------------------------------------------------------- */
class Register {
  public:
    void SetRows(rgb_color, rgb_color, rgb_color, rgb_color, rgb_color, rgb_color);
    void SetCursor(int);
    StateType MoveCursor(int cursorDirection);
    String GetMix(int);
    String SetMix(ControlType);
    Register();
    rgb_color colors[LED_COUNT];
    int cursorPosition;
    boolean mix[LED_COUNT][3];
};
Register::Register()
{
  for (int i=0;i<LED_COUNT;i++)
  {
    for (int j=0;j<3;j++)
    {
      mix[i][j] = false;
    } 
  }
}
void Register::SetRows(rgb_color r0, rgb_color r1, rgb_color r2, rgb_color r3, rgb_color r4, rgb_color r5)
{
  colors[0] = r0;
  colors[1] = r1;
  colors[2] = r2;
  colors[3] = r3;
  colors[4] = r4;
  colors[5] = r5;
}
void Register::SetCursor(int cursorPos)
{
   SetRows(C_OFF, C_OFF, C_OFF, C_OFF, C_OFF, C_OFF);
   if (cursorPos >=0 && cursorPos < LED_COUNT)
   {
     colors[cursorPos] = C_WHITE;
   }
   logToSerial("Cursor Position: " + cursorPos);
}
StateType Register::MoveCursor(int cursorDirection)
{
  if ((cursorPosition == 0 && cursorDirection == -1) || (cursorPosition == LED_COUNT-1 && cursorDirection == 1))
  {
    SetCursor(-1);
    return S_CONFIRM;   
  }
  else if (cursorPosition == -1)
  {
    if (cursorDirection == -1)
    {
      SetCursor(5);
    }
    else
    {
      SetCursor(0);
    }
  }
  else
  {
    SetCursor(cursorPosition + cursorDirection);
  }
  return S_NAVIGATION;
}
String Register::GetMix(int row)
{
  String mixture = "";
  if (mix[row][0] && ! mix[row][1] && ! mix[row][2])
  {
    mixture = "A";
  }
  else if (mix[row][0] && mix[row][1] && !mix[row][2])
  {
    mixture = "A + B";
  }
  else if (mix[row][0] && !mix[row][1] && mix[row][2])
  {
    mixture = "A + C";
  }
  else if (!mix[row][0] && mix[row][1] && !mix[row][2])
  {
    mixture = "B";
  }
  else if (!mix[row][0] && mix[row][1] && mix[row][2])
  {
    mixture = "B + C";    
  }
  else if (!mix[row][0] && !mix[row][1] && mix[row][2])
  {
    mixture = "C";
  }
  else
  {
    mixture = "None"; 
  }
  return mixture;
}
String Register::SetMix(ControlType buttonPress)
{
  switch (buttonPress)
  {
    case B_A:
      mix[cursorPosition][0] = !mix[cursorPosition][0];
    case B_B:
      mix[cursorPosition][1] = !mix[cursorPosition][1];
    case B_C:
      mix[cursorPosition][2] = !mix[cursorPosition][2];
  };
  
  if (mix[cursorPosition][0] && mix[cursorPosition][1] && mix[cursorPosition][2])
  {
     mix[cursorPosition][0] = false;
     mix[cursorPosition][1] = false;
     mix[cursorPosition][2] = false;
  }
}
Register colorRegister;
/* ------------------------------------------------------------------------------- */
class StateMachine {
  public:
    void SetState(StateType, Register);
    StateType state;
    StateMachine();
};
StateMachine::StateMachine()
{
}

void StateMachine::SetState(StateType newState, Register colorRegister)
{
  state = newState;
  switch (newState)
  {
    case S_START:        // Machine is on the first screen
      logToSerial("S_START");
      // ----------------********************
      lcdLines.SetLines("   Light Mixer Pro",
                        "Which colors mix to", 
                        "   make a rainbow?", 
                        "SELECT to continue..");
      break;
    case S_NAVIGATION:   // Machine is navigating between colors
      logToSerial("S_NAVIGATION");
      // ----------------********************
      lcdLines.SetLines("UP/DOWN to move",
                        "SELECT to try a mix",
                        "This row's mix:",
                        colorRegister.GetMix(colorRegister.cursorPosition));
      break;
    case S_MIXER:        // Machine is able to mix colors
      logToSerial("S_MIXER");
      // ----------------********************
      lcdLines.SetLines("Choose 2 from A/B/C",
                        "SELECT to exit Mixer",
                        "Current Mix:",
                        colorRegister.GetMix(colorRegister.cursorPosition));
      break;
    case S_CONFIRM:      // Machine tries to confirm the mix
      logToSerial("S_CONFIRM");
      // ----------------********************
      lcdLines.SetLines("Press SELECT to test",
                        "your mixing skills",
                        "",
                        "");
      break;
    case S_DISPLAY:      // Machine asks for confirmation    
      logToSerial("S_DISPLAY");
      // ----------------********************
      lcdLines.SetLines("Did your mix fix",
                        "the rainbow?",
                        "SELECT - Yes",
                        "UP/DOWN - No");
      break;
    case S_UNKNOWN:     // Machine is in an unknown state
      logToSerial("S_UNKNOWN");
      // ----------------********************
      lcdLines.SetLines("Unknown State",
                        "",
                        "",
                        "");
      break;
  }
  printToLcd(lcdLines.lines,4);
}

StateMachine machine;

/* ------------------------------------------------------------------------------- */
/// void setup
///   
/* ------------------------------------------------------------------------------- */
void setup()
{  
  // Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  
  // Initialize Button Reading
  analogButtons.add(b1);
  analogButtons.add(b2);
  analogButtons.add(b3);
  analogButtons.add(b4);
  analogButtons.add(b5);  
  analogButtons.add(b6);  

  // Initialize LCD
  pinMode(LCD_PIN_BACKLIGHT, OUTPUT);      // Initialize LCD Backlight Pin
  digitalWrite(LCD_PIN_BACKLIGHT, HIGH);   // Turn LCD Backlight on.
  lcd.begin(LCD_COLUMNS, LCD_ROWS);        // Initialize LCD size (columns, rows)
  
  // Initialize LED Strip  
  colorRegister.SetRows(C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_INDIGO, C_VIOLET);
  ledStrip.write(colorRegister.colors, LED_COUNT);
  
  // Initialize State Machine
  machine.SetState(S_START,colorRegister);
}
/* ------------------------------------------------------------------------------- */
/// void loop
/// Author: RangerDan
/// Created: 04202014
/// Description: 
/// - State machine that transitions between states by reacting to button clicks
/// RangerDan-07242014
/// - Reduced number of states
/// - Moved LCD messages to State Machine object
/// RangerDan-08192014
/// - Completed move of LCD Messages to State Machine object
/// - Changed buttons to use one pin instead of six using resistors
/// - Fixed state machine so it rotates through Navigation and Confirmation states
/* ------------------------------------------------------------------------------- */
void loop()
{  
  // Check for button clicks
  analogButtons.check();
  
  // Uncomment to output analog reads to serial, used to configure button values
  // configure();
    
  // Begin State Machine
  switch (buttonPress)
  {
    case B_SELECT:
      switch (machine.state)
      {
        case S_START:
          machine.SetState(S_NAVIGATION, colorRegister);
          colorRegister.SetCursor(0);
          ledStrip.write(colorRegister.colors, LED_COUNT);
          break;
        case S_MIXER:
          machine.SetState(S_NAVIGATION, colorRegister);
          break;
        case S_NAVIGATION:
          machine.SetState(S_MIXER, colorRegister);
          break;
        case S_CONFIRM:
          machine.SetState(S_DISPLAY, colorRegister);
          break;
        case S_DISPLAY:
          logToSerial("S:Display");
          // TODO: Add display code here
          // TODO: Add confirmation code here
          // LCD Message
          // ----------------********************
            lcdLines.SetLines("Did your mix fix",
                              "the rainbow?",
                              "SELECT - Yes",
                              "UP/DOWN - No");
            printToLcd(lcdLines.lines,4);

          break;
        default:
          break; 
      };
      break;
    case B_A:
    case B_B:
    case B_C:
      switch (machine.state)
      {
        case S_MIXER:
          colorRegister.SetMix(buttonPress);
          machine.SetState(S_MIXER, colorRegister);
          break;
        default:
          break; 
      };
      break;
    case B_UP:
    case B_DOWN:
          if (buttonPress == B_UP)
          {
            machine.SetState(colorRegister.MoveCursor(-1), colorRegister);
          }
          else if (buttonPress == B_DOWN)
          {
            machine.SetState(colorRegister.MoveCursor(1), colorRegister);
          }
          break;
      break;
    case INVALID:
      break;
    default:
      machine.SetState(S_UNKNOWN, colorRegister); 
      break;
  }
  // Reset Button press
  buttonPress = INVALID;
}
/* ------------------------------------------------------------------------------- */
/// boolean printToLCD
///   String* lines: Lines to be written to the LCD
///   int lineCount: Number of lines to be written.  Must be less than or equal to
///                  LCD_ROWS
/// Author: RangerDan
/// Created: 04202014
/// Description: 
/// - Takes in an array of strings to be written to an LCD screen "lcd" with a width
///   of LCD_COLUMNS and height of LCD_ROWS.
/// - Checks the length of each row and prints an error to the LCD if they exceed the
///   number of columns.
/// - Only writes the number of lines the screen is capable of supporting.
/// RangerDan-07242014
/// - Simplified function to not write as much supporting info and reduce memory footprint
/* ------------------------------------------------------------------------------- */
boolean printToLcd(String* lines, int lineCount)
{
  boolean linesTooLong = false;
  
  if (lineCount > LCD_ROWS)              // If the number of lines exceeds the number of rows on the LCD,
  {                                      // only write the max number of lines.
     logToSerial("Too many lines");
     lineCount = LCD_ROWS;
  }
  
  for(int i=0;i<lineCount;i++)            // Check the length of each line passed in for writing
  {
    if (lines[i].length() > LCD_COLUMNS) // If too long, set flag
    {
      linesTooLong = true;
      logToSerial("Line too Long");
    }
  }
  
  lcd.clear();                           // Clear the screen
  
  if (!linesTooLong)                      // If flag has been set, print an error message
  {
    for (int j=0;j<lineCount;j++)
    {
      logToSerial("To LCD: " + lines[j]);
      lcd.setCursor(0,j);                // set cursor to column 0, row 0 (the first row)
      lcd.print(lines[j]);               // change this text to whatever you like. keep it clean.
    }
  }

  return !linesTooLong;
}
/* ------------------------------------------------------------------------------- */
/// void setColors
///   rgb_color* colorsToWrite: array of colors to be written to the LED Strip
/// Author: RangerDan
/// Created: 07242014
/// Description: 
/// - Takes in an array of colors and writes them to the array of RGB LED Strip colors
/* ------------------------------------------------------------------------------- */
void setColors( rgb_color* colorsToWrite)
{
  for(byte i = 0; i < LED_COUNT; i++)
  {
    colors[i] = colorsToWrite[i];
  }
}  
/* ------------------------------------------------------------------------------- */
/// void logToSerial
///   String line: Line to be written to the serial log
/// Author: RangerDan
/// Created: 04202014
/// Description: 
/// - Takes in a string and writes it to the log.
/* ------------------------------------------------------------------------------- */
void logToSerial(String line)
{
  Serial.print(line + '\n');
}
