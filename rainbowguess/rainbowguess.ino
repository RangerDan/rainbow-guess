// Serial Prep
// Used for logging behavior to the PC
const long SERIAL_BAUD_RATE = 115200L;

// Controls Prep
#include <Bounce2.h>
const int BUTTON_COUNT = 6;
typedef enum  ControlType {
  B_SELECT,
  B_A,
  B_B,
  B_C,
  B_UP,
  B_DOWN,
  INVALID
};
const int BUTTON_PIN[] = {A0, A1, A2, A3, A4, A5};
Bounce debouncer[BUTTON_COUNT];

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
#include <PololuLedStrip2.h>                // LED Strip library
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
  S_EXPLAIN_1,            // Machine is explaining rules: Problem
  S_EXPLAIN_2,            // Machine is explaining rules: Solution
  S_NAVIGATION,           // Machine is navigating between colors
  S_MIXER,                // Machine takes input to add/clear mixed colors on the row
  S_CONFIRM,              // Machine asks user if they are complete, can navigate away
  S_DISPLAY,              // Machine displays current mix
  S_CHECK,                // Machine is displaying results
  S_UNKNOWN               // Machine is in an unknown state (before init or bad state)
};
StateType machineState;
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
   if (cursorPos >=0 && cursorPos <= LED_COUNT)
   {
     colors[cursorPos] = C_WHITE;
   }
}
StateType Register::MoveCursor(int cursorDirection)
{
  if ((cursorPosition == 0 && cursorDirection == -1) || (cursorPosition == LED_COUNT-1 && cursorDirection == 1))
  {
    SetCursor(-1);
    return S_CONFIRM;   
  }
  
  if (cursorPosition == -1)
  {
    if (cursorDirection == -1)
    {
      SetCursor(5);
    }
    else
    {
      SetCursor(0);
    }
    return S_NAVIGATION;
  }
  else if (cursorDirection == -1 && cursorPosition != 0)
  {
    cursorPosition += cursorDirection;
  }
  else if (cursorDirection == 1 && cursorPosition != 5)
  {
    cursorPosition += cursorDirection;
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
void setup()
{  
  // Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  logToSerial("Serial setup complete");
  
  // Initialize Controls 
  for (ControlType i=B_SELECT;i<=B_DOWN;i=ControlType(int(i)+1))
  {
    pinMode(BUTTON_PIN[i], INPUT);
    debouncer[i] = Bounce();
    debouncer[i].attach(BUTTON_PIN[i]);
    debouncer[i].interval(5);
  }
  logToSerial("Control Button setup complete");
  
  // Initialize LCD
  pinMode(LCD_PIN_BACKLIGHT, OUTPUT);      // Initialize LCD Backlight Pin
  digitalWrite(LCD_PIN_BACKLIGHT, HIGH);   // Turn LCD Backlight on.
  lcd.begin(LCD_COLUMNS, LCD_ROWS);        // Initialize LCD size (columns, rows)
  
  // LCD Welcome Message
  // ----------------********************
  lcdLines.SetLines("   Light Mixer Pro",
                    "Mix the colors and", 
                    "     make a rainbow!", 
                    "SELECT to continue..");
  printToLcd(lcdLines.lines,4);
  logToSerial("LCD setup complete");

  // Initialize LED Strip  
  colorRegister.SetRows(C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_INDIGO, C_VIOLET);
  ledStrip.write(colorRegister.colors, LED_COUNT);
  logToSerial("LED Strip setup complete");
  
  // Initialize State Machine
  machineState = S_START;
}
/* ------------------------------------------------------------------------------- */
void loop()
{  
  ControlType buttonPress = INVALID;
  
  int reading = LOW;
  boolean stateChanged = false;
  // Read button states on this pass
  for(ControlType i=B_SELECT;i<=B_DOWN;i=ControlType(int(i)+1))
  {
    stateChanged = debouncer[i].update();
    reading = debouncer[i].read();
    if (stateChanged && reading == HIGH)
    {
      logToSerial("Button press detected: " + String(int(i)));
      buttonPress = i;
    }
    
    reading = LOW;
    stateChanged = false;
  }
  
  // Begin State Machine
  switch (buttonPress)
  {
    case B_SELECT:
      switch (machineState)
      {
        case S_START:
          machineState = S_EXPLAIN_1;
          logToSerial("Entered State Explain_1");
          // LCD Message
          // ----------------********************
          lcdLines.SetLines("Programmers mixed up",
                            "the colors. Test to",
                            "fix the mess.",
                            "SELECT to continue..");
          printToLcd(lcdLines.lines,4);
          break;
        case S_EXPLAIN_1:
          machineState = S_EXPLAIN_2;
          logToSerial("Entered State Explain_2");
          // LCD Message
          // ----------------********************
          lcdLines.SetLines("-Move with UP/DOWN",
                            "-Mix colors A, B, C",
                            "-SELECT to test it",
                            "SELECT to continue..");
          printToLcd(lcdLines.lines,4);

          break;
        case S_EXPLAIN_2:
          machineState = S_NAVIGATION;
          logToSerial("Entered State Navigation");
          // LCD Message
          // ----------------********************
          lcdLines.SetLines("UP/DOWN to move",
                            "SELECT to try a mix",
                            "This row's mix:",
                            colorRegister.GetMix(0));
          printToLcd(lcdLines.lines,4);
          colorRegister.SetCursor(0);
          ledStrip.write(colorRegister.colors, LED_COUNT);
          break;
        case S_NAVIGATION:
          machineState = S_MIXER;
          logToSerial("Entered State Mixer");
          // LCD Message
          // ----------------********************
          lcdLines.SetLines("Choose 2 from A/B/C",
                            "SELECT to exit Mixer",
                            "Current Mix:",
                            colorRegister.GetMix(colorRegister.cursorPosition));
          printToLcd(lcdLines.lines,4);
          break;
        case S_MIXER:
          machineState = S_NAVIGATION;
          logToSerial("Entered State Navigation");
          // LCD Message
          // ----------------********************
          lcdLines.SetLines("UP/DOWN to move",
                            "SELECT to try a mix",
                            "This row's mix:",
                            colorRegister.GetMix(colorRegister.cursorPosition));
          printToLcd(lcdLines.lines,4);
          ledStrip.write(colorRegister.colors, LED_COUNT);
          break;
/*        case S_CONFIRM:
          machineState = S_DISPLAY;
          logToSerial("Entered State Confirmation");
          // LCD Message
          // ----------------********************
          lcdLines.SetLines("Press SELECT to test",
                            "your mixing skills",
                            "",
                            "");
          printToLcd(lcdLines.lines,4);
          break;
        case S_DISPLAY:
          logToSerial("Entered State DISPLAY");
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
*/        default:
          break; 
      };
      break;
    case B_A:
    case B_B:
    case B_C:
      switch (machineState)
      {
        case S_MIXER:
          logToSerial("Detected Button Press: " + buttonPress);
          // LCD Message
          // ----------------********************
          colorRegister.SetMix(buttonPress);
          lcdLines.SetLines("Choose 2 from A/B/C",
                            "SELECT to exit Mixer",
                            "Current Mix:",
                            colorRegister.GetMix(colorRegister.cursorPosition));
          printToLcd(lcdLines.lines,4);
          break;
        default:
          break; 
      };
      break;
    case B_UP:
    case B_DOWN:
      switch (machineState)
      {
        case S_NAVIGATION:
          logToSerial("Detected Button Press: " + buttonPress);
          // LCD Message
          // ----------------********************
          if (buttonPress == B_UP)
          {
            machineState = colorRegister.MoveCursor(-1);
          }
          else if (buttonPress == B_DOWN)
          {
            machineState = colorRegister.MoveCursor(1);
          }
          
          if (machineState == S_NAVIGATION)
          {
            lcdLines.SetLines("Choose 2 from A/B/C",
                              "SELECT to exit Mixer",
                              "Current Mix:",
                              colorRegister.GetMix(colorRegister.cursorPosition));
            printToLcd(lcdLines.lines,4);
            ledStrip.write(colorRegister.colors, LED_COUNT);
          }
          else if (machineState == S_CONFIRM)
          {
            lcdLines.SetLines("Press SELECT to test",
                              "your mixing skills",
                              "",
                              "");
            printToLcd(lcdLines.lines,4);
          }
          break;
        case S_CONFIRM:
          logToSerial("Detected Button Press: " + buttonPress);
          // LCD Message
          // ----------------********************
          if (buttonPress == B_UP)
          {
            colorRegister.MoveCursor(-1);
          }
          else if (buttonPress == B_DOWN)
          {
            colorRegister.MoveCursor(1);
          }
          
          machineState = S_NAVIGATION;
          lcdLines.SetLines("Choose 2 from A/B/C",
                            "SELECT to exit Mixer",
                            "Current Mix:",
                            colorRegister.GetMix(colorRegister.cursorPosition));
          printToLcd(lcdLines.lines,4);
          break;
        default:
          break; 
      };
      break;
    case INVALID:
      break;
    default:
      machineState = S_UNKNOWN; 
      break;
  }
  if (buttonPress != INVALID)
  {
    logToSerial("Resetting Button Press: " + String(int(buttonPress)));
  }
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
/* ------------------------------------------------------------------------------- */
boolean printToLcd(String* lines, int lineCount)
{
  boolean linesTooLong = false;
  String errorMessageLineLength = "Line Length Error";
  
  if (lineCount > LCD_ROWS)              // If the number of lines exceeds the number of rows on the LCD,
  {                                      // only write the max number of lines.
     logToSerial("Too many lines sent.  Sent: " + String(lineCount) + ", Max: " + LCD_ROWS);
     lineCount = LCD_ROWS;
  }
  
  for(int i=0;i<lineCount;i++)            // Check the length of each line passed in for writing
  {
    if (lines[i].length() > LCD_COLUMNS) // If too long, set flag
    {
      linesTooLong = true;
      logToSerial("Line too Long: " + lines[i]);
    }
  }
  
  lcd.clear();                           // Clear the screen
  
  if (linesTooLong)                      // If flag has been set, print an error message
  {
    logToSerial("Printing error to LCD: " + errorMessageLineLength);
    lcd.setCursor(0,0);                  // set cursor to column 0, row 0 (the first row)
    lcd.print(errorMessageLineLength);   // change this text to whatever you like. keep it clean.
  }
  else
  {
    logToSerial("Printing lines to LCD");
    for (int j=0;j<lineCount;j++)
    {
      logToSerial("Printing line to LCD: " + lines[j]);
      lcd.setCursor(0,j);                // set cursor to column 0, row 0 (the first row)
      lcd.print(lines[j]);               // change this text to whatever you like. keep it clean.
    }
  }
  
  return !linesTooLong;
}
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
