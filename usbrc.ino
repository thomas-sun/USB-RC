#include <IRremote.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>


//#define _DEBUG




IRsend *irsend = NULL;

#define bufLen 600
#define codeLen 100


unsigned int cmd;
char dataBuf[bufLen];
int idx = 0;
int record_flag = 0;
volatile unsigned int g_x; // Pointer thru irBuffer - volatile because changed by ISR
unsigned int codes[codeLen];


/****************************************************

****************************************************/
void rxIR_Interrupt_Handler() {
  if (g_x > codeLen) return; //ignore if irBuffer is already full
  codes[g_x++] = micros(); //just continually record the time-stamp of signal transitions
}


/****************************************************



****************************************************/
void setup()
{

  Serial.begin(9600);

  
#ifdef _DEBUG
  Serial.print("Debug mode\n");
#endif
  
  irsend = new IRsend;

 

}
/****************************************************



****************************************************/
int ConvCode()
{
  char tmp[16];
  int idx0 = 0;
  int idx1 = 0;

  for(int x=1; x<bufLen;x++) {
    if(dataBuf[x] == 0)  {
      tmp[idx0] = 0;
      if(idx0 > 0)
        codes[idx1++] = (unsigned int)atol(tmp);
        
#ifdef _DEBUG
    Serial.print(codes[idx1-1]);
    Serial.print(" ");
#endif

      break;
    }
    if(dataBuf[x] == ',') {
      tmp[idx0] = 0;
      codes[idx1++] = (unsigned int)atol(tmp);
#ifdef _DEBUG
    Serial.print(codes[idx1-1]);
    Serial.print(" ");
#endif      
      idx0 = 0;
      continue;
    }
    tmp[idx0++] = dataBuf[x];
  }
  
#ifdef _DEBUG
    Serial.print("Len:");
    Serial.println(idx1, HEX);
#endif  

  return idx1;
}

/****************************************************
    IR Mode
****************************************************/
void IR()
{
  int len;
   while (Serial.available()) //connected
      { 
        cmd = Serial.read();
        //Serial.println(cmd, HEX);
        if(cmd == '[') { // begin code
          dataBuf[0] = (char)cmd;
          idx = 1;
        }
        else if(cmd == ']') { // end code
          dataBuf[idx] = 0;
          idx = 0;
          len = ConvCode();
          irsend->sendRaw(codes,len,38);
    
        }
        else if(idx >= 1 && idx < bufLen) {
          if(idx == 1 && (char)cmd == '*') {
            record_flag = 1;
            break;
          }
    
          
          if((cmd >= '0' && cmd <= '9') || cmd == ',') {  
            dataBuf[idx] = (char)cmd;
            idx++;
          }
          else { // error
            idx = 0;
          }
    
        }
      }

}
/****************************************************
    Record mode
****************************************************/
void Record()
{

    Serial.println(F("Press the button on the remote now - once only"));
 
    g_x = 0;


    // interrupt 0 (pin 2)
    attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE);//set up ISR for receiving IR signal
    delay(5000); // pause 5 secs
    detachInterrupt(0);//stop interrupts & capture until finshed here


    if (g_x) { //if a signal is captured
    
 
      Serial.println();
      Serial.print(F("Raw: (")); //dump raw header format - for library
      Serial.print((g_x - 1));
      Serial.print(F(") ["));
  


      for (int i = 1; i < g_x; i++) { //now dump the times
        //if (!(i & 0x1)) Serial.print(F("-"));
        codes[i - 1] = codes[i] - codes[i - 1];
        Serial.print(codes[i - 1]);
   
        if(i==(g_x-1)) {
          Serial.print(F("]"));
          Serial.print(g_x-1, DEC);
        }
        else {
          Serial.print(F(","));
        }
      }
     
             
      Serial.println();
      Serial.println();

    } // g_x

  
}
/****************************************************

****************************************************/
void loop()
{
  
  if(record_flag == 1) {
    Record();
   
  } else {
    IR();
  }
  
  
}


