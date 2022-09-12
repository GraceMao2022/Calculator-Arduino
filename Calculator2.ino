#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad

//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]=
{
{'1', '2', '3', '/'},
{'4', '5', '6', '*'},
{'7', '8', '9', '-'},
{'.', '0', '=', '+'}
};

//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3

enum state {WaitSign, WaitOp1, WaitOp2, WaitOp3, Answer}st;
enum operation {None, Add, Sub, Mult, Div};
char sign = '+';
double decimalPlace = 1;
double answer = 0;
double currNum = 0;
double prevNum = 0;
bool negative;
bool onOperator;

operation currOp = None;
operation prevOp = None;
operation prevPrevOp = None;

//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
lcd.init(); // initialize the lcd
lcd.backlight();
lcd.setCursor(0,0);
st = WaitSign;
}

void loop()
{
  char keypressed = myKeypad.getKey();
  
  if(st == WaitSign)
  {
    if(keypressed == '+' || keypressed == '-')
    {
      if((sign == '-' && keypressed == '-') || (sign == '+' && keypressed == '+'))
        sign = '+';
      else
        sign = '-';
      lcd.clear();
      lcd.print(sign);
    }
    else if(isNum(keypressed) || keypressed == '.')
    {
      st = WaitOp1;
      if(keypressed == '.')
        decimalPlace = 0.1;
      else
        currNum += keypressed - '0';
      lcd.print(keypressed);
    }
  }
  else if(st == WaitOp1)
  {
    if(keypressed == '.' && decimalPlace == 1)
    {
      decimalPlace = 0.1;
      lcd.print(keypressed);
    }
    else if(isNum(keypressed))
    {
      if(decimalPlace == 1)
      {
        currNum = currNum*10 + (keypressed - '0');
        lcd.print(keypressed);
      }
      else
      {
        currNum += (keypressed - '0')*decimalPlace;
        decimalPlace *= 0.1;
        lcd.print(keypressed);
      }
    }
    else if(isOperator(keypressed))
    {
      decimalPlace = 1;
      currOp = getCurrentOp(keypressed);
      st = WaitOp2;
      lcd.print(keypressed);
      if(sign == '+')
        answer = currNum;
      else
        answer = -1*currNum;
      onOperator = true;
      currNum = 0;
      sign = '+';
    }
    else if(keypressed == '=')
    {
      st = Answer;
      if(sign == '+')
        answer = currNum;
      else
        answer = -1*currNum;
      decimalPlace = 1;
      currNum = 0;
    }
  }
  else if(st == WaitOp2)
  {
    if(keypressed == '-' && !negative && onOperator)
    {
      negative = true;
      lcd.print(keypressed);
    }
    else if(keypressed == '.' && decimalPlace == 1)
    {
      decimalPlace = 0.1;
      lcd.print(keypressed);
      onOperator = false;
    }
    else if(isNum(keypressed))
    {
      if(decimalPlace == 1)
      {
        currNum = currNum*10 + (keypressed - '0');
        lcd.print(keypressed);
      }
      else
      {
        currNum += (keypressed - '0')*decimalPlace;
        decimalPlace *= 0.1;
        lcd.print(keypressed);
      }
      onOperator = false;
    }
    else if(isOperator(keypressed) && !onOperator)
    {
      decimalPlace = 1;
  
      prevOp = currOp;
      currOp = getCurrentOp(keypressed);
  
      if(negative)
      {
        currNum = -1*currNum;
        negative = false;
      }
     
      if(getOpLevel(currOp) > getOpLevel(prevOp))
      {
        prevNum = currNum;
        prevPrevOp = prevOp;
        prevOp = currOp;
        currOp = None;
        st = WaitOp3;
        onOperator = true;
      }
      else
      {
        answer = evaluateOperation(answer, currNum, prevOp);
        onOperator = true;
      }
    
      lcd.print(keypressed);
      currNum = 0;
    }
    else if(keypressed == '=' && !onOperator)
    {
      st = Answer;
      if(negative)
        currNum = -1*currNum;
      answer = evaluateOperation(answer, currNum, currOp);
    }
  }
  else if(st == WaitOp3)
  {
    if(keypressed == '-' && !negative && onOperator)
    {
      negative = true;
      lcd.print(keypressed);
    }
    else if(keypressed == '.' && decimalPlace == 1)
    {
      decimalPlace = 0.1;
      lcd.print(keypressed);
      onOperator = false;
    }
    else if(isNum(keypressed))
    {
      if(decimalPlace == 1)
      {
        currNum = currNum*10 + (keypressed - '0');
        lcd.print(keypressed);
      }
      else
      {
        currNum += (keypressed - '0')*decimalPlace;
        decimalPlace *= 0.1;
        lcd.print(keypressed);
      }
      onOperator = false;
    }
    else if(isOperator(keypressed) && !onOperator)
    {
      decimalPlace = 1;
      
      if(negative)
      {
        currNum = -1*currNum;
        negative = false;
      }
      currOp = getCurrentOp(keypressed);
     
      prevNum = evaluateOperation(prevNum, currNum, prevOp);
      if(getOpLevel(currOp) > getOpLevel(prevPrevOp))
      {
        prevOp = currOp;
      }
      else
      {
        answer = evaluateOperation(answer, prevNum, prevPrevOp);
        prevOp = currOp;
        prevPrevOp = None;
        st = WaitOp2;
      }
    
      lcd.print(keypressed);
      onOperator = true;
      currNum = 0;
    }
    else if(keypressed == '=' && !onOperator)
    {
      st = Answer;
  
      if(negative)
        currNum = -1*currNum;
   
      prevNum = evaluateOperation(prevNum, currNum, prevOp);
      answer = evaluateOperation(answer, prevNum, prevPrevOp);
    }
  }
  else if(st == Answer)
  {
    lcd.setCursor(0,1);
  
    lcd.print("= ");
    lcd.print(answer);
  
    decimalPlace = 1;
    currNum = 0;
    prevNum = 0;
    sign = '+';
    negative = false;
    currOp = None;
    onOperator = false;
  
    if(keypressed == '+' || keypressed == '-')
    {
      lcd.clear();
      answer = 0;
      lcd.setCursor(0,0);
      lcd.print(keypressed);
      if((sign == '-' && keypressed == '-') || (sign == '+' && keypressed == '+'))
        sign = '+';
      else
        sign = '-';
      st = WaitSign;
    }
    else if(isNum(keypressed) || keypressed == '.')
    {
      lcd.clear();
      answer = 0;
      lcd.setCursor(0,0);
      st = WaitOp1;
      if(keypressed == '.')
        decimalPlace = 0.1;
      else
        currNum += keypressed - '0';
      lcd.print(keypressed);
    }
  }
}

bool isNum(char key)
{
  if(key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9')
    return true;
  return false;
}

bool isOperator(char key)
{
  if(key == '+' || key == '-' || key == '*' || key == '/')
    return true;
  return false;
}

double getOpLevel(operation op)
{
  if(op == Add || op == Sub)
    return 0;
  else if(op == None)
    return -1;
  else
    return 1;
}

double evaluateOperation(double x, double y, operation op)
{
  if(op == Add)
    return x+y;
  else if(op == Sub)
    return x-y;
  else if(op == Mult)
    return x*y;
  else if(op == Div)
    return x/y;
}

operation getCurrentOp(char character)
{
  if(character == '+')
    return Add;
  else if(character == '-')
    return Sub;
  else if(character == '*')
    return Mult;
  else if(character == '/')
    return Div;
}
