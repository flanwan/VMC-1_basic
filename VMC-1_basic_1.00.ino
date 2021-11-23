/* 
 *  generic program to provide examples for MIDI-Handling in the VMC-1
 *  
 */

String firmwareversion = "VMC-1_basic_1.0";
 
#include <EEPROM.h>
#include <MIDI.h>
#include <LiquidCrystal.h>#
//#include <EEPROM.h>
MIDI_CREATE_DEFAULT_INSTANCE();
LiquidCrystal lcd(2,3,4,5,6,7);

// Define inputs and outputs
const int button1Pin=8;   //PLAY
const int button2Pin=9;   //STOP
const int button3Pin=12;  //WRITE
const int button4Pin=11;  //F1
const int button5Pin=10;  //F2
const int clockInPin=13;  //CLOCK-IN

const int tempoCVInPin = A0; // =Tempo potentiometer
const int CV1Pin = A1; // (5V or CV1-In) via CV1 potentiometer
const int CV2Pin = A2; // (5V or CV2-In) via CV2 potentiometer
const int clockOutPin = A5; //->sent via the clock-socket-switch to triggerPin

// Variables to handle the buttons and digital inputs
int button1State=0; 
int button2State=0; 
int button3State=0;
int button4State=0;
int button5State=0;
int clockInState=0;
int button1StatePrev=0;
int button2StatePrev=0;
int button3StatePrev=0;
int button4StatePrev=0;
int button5StatePrev=0;
int clockInStatePrev=0;

// Variables to handle the potentiometers
unsigned long CV1;
unsigned long CV2;
unsigned long tempoCV;


// Variables to handle the generic status of the software
int menu=0;
int edit=0;
int runmode=0;
int writemode=0;

//define parameter variables
int outchannel=1;
int inchannel=1;
int some_value=64;
int some_mode=3;
int maxmenu;

void setup()
{
   lcd.begin(16, 2);
   MIDI.begin();

   //write default values for parameters if EEPROM is blank
   if (EEPROM.read(1) == 255) EEPROM.write(1,3); //some_mode
   if (EEPROM.read(2) == 255) EEPROM.write(2,64); //some_value
   if (EEPROM.read(3) == 255) EEPROM.write(3,1); //outchannel
   if (EEPROM.read(4) == 255) EEPROM.write(4,1); //inchannel

   //read stored parameter values from EEPROM
   some_mode=EEPROM.read(1);
   some_value=EEPROM.read(2);
   outchannel=EEPROM.read(3);
   inchannel=EEPROM.read(4);

   //setup the inputs and outputs
   pinMode(button1Pin, INPUT);
   pinMode(button2Pin, INPUT);
   pinMode(button3Pin, INPUT);
   pinMode(button4Pin, INPUT);
   pinMode(button5Pin, INPUT);
   pinMode(clockInPin, INPUT);
   pinMode(tempoCVInPin, INPUT);
   pinMode(CV1Pin, INPUT);
   pinMode(CV2Pin, INPUT);
   pinMode(clockOutPin, OUTPUT);

   //some default settings for MIDI (recommended, but not necessary)
   MIDI.setThruFilterMode(1);
   MIDI.turnThruOff();

   midireset();
      
   lcd.setCursor(0, 0); 
   lcd.print(firmwareversion);
   delay(1000);
   lcd.setCursor(0, 0);
   lcd.print("            ");  
   loadsequence();
      
   clearscreen();
        
}

void midireset() {
   int i=0;
   for (i=0; i <= 126 ; i++) {
      // some synths expect NoteOn with velocity zero as NoteOff:
      MIDI.sendNoteOn(i,0,outchannel);
      // some synths expect a dedicated NoteOff command:
      MIDI.sendNoteOff(i,0,outchannel);
      delay(1);
   }
   MIDI.sendControlChange(123,0,outchannel);
}

// a simple clearscreen:
void clearscreen(){
   lcd.setCursor(0,0);
   lcd.print("                ");
   lcd.setCursor(0,1);
   lcd.print("                ");
}


// a routine for save and load to/from EEPROM
void savesequence() {
   lcd.setCursor(1,1);
   lcd.print("SAVING        ");
   /*
   // DO DATA SAVING HERE. EXAMPLE FROM SQ-3P:
   for (stepNumber=0; stepNumber < maxSteps; stepNumber++) {
      for (noteInChord=0; noteInChord < maxNotes; noteInChord++) {
          int address=((stepNumber * maxNotes ) + noteInChord) + 32;
          EEPROM.write(address,note[stepNumber][noteInChord]);
          lcd.setCursor(1,1);
          lcd.print(address);
      }
   }
   */
   //delay(1000); //PAUSE ONLY FOR DEMO REASONS
   lcd.setCursor(1,1);
   lcd.print("SAVE DONE     ");  
   //delay(1000);
}

void loadsequence() {
   lcd.setCursor(0,0);
   lcd.print("LOAD SEQUENCE ");
   /*
   // DO DATA LOADING HERE. EXAMPLE FROM SQ-3P:
   for (stepNumber=0; stepNumber < maxSteps; stepNumber++) {
      for (noteInChord=0; noteInChord < maxNotes; noteInChord++) {
          int address=((stepNumber * maxNotes ) + noteInChord) + 32;
          note[stepNumber][noteInChord]=EEPROM.read(address);
          if ( note[stepNumber][0] == 254 ) lastStep=stepNumber - 1; 
          lcd.setCursor(1,1);
          lcd.print(address);
      }
   }
   */
   lcd.setCursor(1, 1); 
   //delay(1000); //PAUSE ONLY FOR DEMO REASONS
   lcd.print("LOAD DONE      ");
   //delay(1000);
   clearscreen();
}


void loop()
{

   /* READ BUTTONS */ 
   button1State=!digitalRead(button1Pin);
   button2State=!digitalRead(button2Pin);
   button3State=!digitalRead(button3Pin);
   button4State=!digitalRead(button4Pin);
   button5State=!digitalRead(button5Pin);

   /* READ POTENTIOMETERS / CVs and adopt them to wanted Values if necessary*/
   CV1=(int) ((analogRead(CV1Pin) * 1 ) / 4); // divide by eight provides a value between 0 and 255
   //CV1=(int) ((analogRead(CV1Pin) * 1.029 ) / 4); // divide by eight provides a value between 0 and 255
   CV2=(int) (analogRead(CV2Pin) * 1.019 / 8 ); // divide by eight provides a value between 0 and 127 (suitable for MIDI data)
   tempoCV=(analogRead(tempoCVInPin) * 1.006);


   /* TIE plus REST will switch to setup*/
   if (button4State == 1 && button4StatePrev == 0 && button5State == 1 && button5StatePrev == 0) {
      button4StatePrev=1;
      button5StatePrev=1;
      clearscreen();
      menu=1;
   }
   if (button4State == 0 && button4StatePrev == 1 && button5State == 0 && button5StatePrev == 1) {
      button4StatePrev=0;
      button5StatePrev=0;
   }
   

   if (menu == 0) { // NORMAL ACTION MODE
     /*
      lcd.setCursor(0, 0);
      lcd.print("F1+F2    enterEdit");
      lcd.setCursor(0, 1);
      lcd.print("Play+Stop exitEdit");
     */

      lcd.setCursor(0, 0);
      lcd.print("Tempo CV1   CV2    ");
      lcd.setCursor(0, 1);
      lcd.print(tempoCV);
      lcd.print(" ");
      lcd.setCursor(5, 1);
      lcd.print(CV1);
      lcd.print(" ");
      lcd.setCursor(11, 1);
      lcd.print(CV2);
      lcd.print(" ");
      
      /* TIE BUTTON SENDS ALLNOTESOFF */
      if (button4State == 1 && button4StatePrev == 0 && writemode == 0) {
         button4StatePrev=1;
         midireset();
      }
      if (button4State == 0 && button4StatePrev == 1 && writemode == 0) {
         button4StatePrev=0;
      }
      
      /* PLAY BUTTON */
      if (button1State == 1 && button1StatePrev == 0 && runmode==0 && writemode == 0) { //PLAY
         button1StatePrev=1;
         runmode=1;
         writemode=0; 
         clearscreen();
         //if (button2State == 0) stepNumber=0; // DO NOT RESET THE SEQUENCE IF LEAVING THE SETUP 
         lcd.setCursor(0, 0);
         lcd.print("PLAY");           
         //delay(2);
      }
      if (button1State == 0 && button1StatePrev == 1)  {
         button1StatePrev=0;
      }

      /* STOP BUTTON */
      if (button2State == 1 && button2StatePrev == 0 && writemode == 0) { //STOP
         button2StatePrev=1;
         midireset();
         runmode=0;
         //clearscreen();
      }
      if (button2State == 0 && button2StatePrev == 1)  {
        //clearscreen();
        button2StatePrev=0;
      }
      
      /* WRITE BUTTON */
      if (button3State == 1 && button3StatePrev == 0 && runmode == 0 && writemode == 0) { //WRITE
         button3StatePrev=1;
         runmode=0;
         writemode=1; 
         clearscreen(); 
      }
      if (button3State == 0 && button3StatePrev == 1)  button3StatePrev=0;
   }
   
   if (menu > 0 ) { //SETUP
      lcd.setCursor(0,1);
      if (edit == 0) lcd.print(" ");
      if (edit == 1) lcd.print("!");

      /* STOP+PLAY exits the setup */
      if ( button1State == 1 && button1StatePrev == 0 && button2State == 1 && button2StatePrev == 0 && menu > 0 && edit == 0) { 
         button1StatePrev = 1;
         button2StatePrev = 1;
         clearscreen();
         lcd.setCursor(0, 0); 
         if (runmode == 0) lcd.print("STOP");
         if (runmode == 1) lcd.print("PLAY");
         menu=0;
      }

      
      /*
       * Stepping thru the setup menu
       */
      
      maxmenu=7; // MAXMENU DETERMINES HOW MANY MENU PAGES ARE AVAILABLE 
      
      if (edit == 0) {
         if (button4State == 1 && button4StatePrev == 0) { // SCROLL THRU MENU DOWN BY PRESSING F1
          button4StatePrev=1;
          clearscreen();
          //lcd.setCursor(0, 1);
          //lcd.print("                ");
          menu--;
          if (menu < 1) menu = maxmenu;
         }

         if (button4State == 0 && button4StatePrev == 1) {
            button4StatePrev=0;
         }
         
         if (button5State == 1 && button5StatePrev == 0) { // SCROLL THRU MENU UP BY PRESSING F2
            button5StatePrev=1;
            clearscreen();
            //lcd.setCursor(0, 1);
            //lcd.print("                ");
            menu++;
            if (menu > maxmenu) menu = 1;
         }
         if (button5State == 0 && button5StatePrev == 1) {
            button5StatePrev=0;
         }
         
         if (button3State == 1 && button3StatePrev == 0) { // ENTER EDIT MODE BY PRESSING WRITE
            button3StatePrev=1;
            edit=1;           
         }
         if (button3State == 0 && button3StatePrev == 1) {
            button3StatePrev=0;
         }
         
      }
      //
      switch (menu) {
        /*PLAYMODE,Some_Mode,Some_Value 1,Some_Value 2,OutChannel,InChannel,LoadData,SaveData};
         *0        1         2            3            4          5         6        7           
         */
        case 1: //SOME MODE SWITCHING

           lcd.setCursor(0, 0); 
           lcd.print("SOME MODE       ");
           lcd.setCursor(1, 1); 
           if (some_mode==0) lcd.print("Mode 1       ");
           if (some_mode==1) lcd.print("Mode 2       ");
           if (some_mode==2) lcd.print("Mode 3       ");
           if (some_mode==3) lcd.print("Mode 4       ");
           if (edit == 1) {
              if (button4State == 1 && button4StatePrev == 0) { //DECREASE MODE
                 button4StatePrev=1;
                 if (some_mode > 0) some_mode--;
              }
              if (button4State == 0 && button4StatePrev == 1) {
                 button4StatePrev=0;
              }
      
              if (button5State == 1 && button5StatePrev == 0) { //INCREASE MODE
                 button5StatePrev=1;
                 if (some_mode < 3) some_mode++;
              }
              if (button5State == 0 && button5StatePrev == 1) {
                 button5StatePrev=0;
              }
              
              if (button3State == 1 && button3StatePrev == 0) { 
                 button3StatePrev=1;
                 if (some_mode == 0) some_mode=0;
                 EEPROM.write(1,some_mode);
                 delay(10);
                 edit=0;
              }
              if (button3State == 0 && button3StatePrev == 1) {
                 button3StatePrev=0;
              }
           }                      
        break;


        case 2://SOME VALUE COUNTED FINE COARSE WITH TWO BUTTONS
           lcd.setCursor(0, 0); 
           lcd.print("SOME VALUE 1   ");
           lcd.setCursor(1, 1); 
           lcd.print(some_value);
           lcd.print(" ");
           if (edit == 1 ){
              if (button4State == 1 && button4StatePrev == 0 && button5State == 0)  {
                some_value--;
                button4StatePrev=1;
              }

              if (button5State == 1 && button5StatePrev == 0 && button4State == 0) {
                some_value++;
                button5StatePrev=1;
              }

              if (button4State == 1 && button4StatePrev == 0 && button5State == 1)  {
                some_value=some_value+10; 
                button4StatePrev=1;
              }

              if (button5State == 1 && button5StatePrev == 0 && button4State == 1) {
                some_value=some_value - 10; 
                button5StatePrev=1;
              }


              if (button4State == 0 && button4StatePrev == 1) {
                button4StatePrev=0;
              }
              if (button5State == 0 && button5StatePrev == 1)  {
                button5StatePrev=0;
              }
 
              if (some_value < 0) some_value=0;
              if (some_value > 127) some_value=127;

              
              if (button3State == 1 && button3StatePrev == 0) { 
                 button3StatePrev=1;
                 EEPROM.write(3,some_value);
                 delay(10);
                 edit=0;
              }
              if (button3State == 0 && button3StatePrev == 1) {
                 button3StatePrev=0;
              }
           }
        break;

        case 3://SOME VALUE COUNTED 1-19 as 1, 20-250 as 10
           lcd.setCursor(0, 0); 
           lcd.print("SOME VALUE 2   ");
           lcd.setCursor(1, 1); 
           lcd.print(some_value);
           if (edit == 1) {
              if (button4State == 1 && button4StatePrev == 0) { //DECREASE VALUE
                 button4StatePrev=1;
                 if (some_value < 21 && some_value > 1) {
                    some_value = some_value - 1;
                 }
                 if (some_value >= 30) some_value = some_value - 10;
                 //lcd.setCursor(1, 1); 
                 //lcd.print("      ");
                 lcd.setCursor(1, 1); 
                 lcd.print(some_value);                 
                 lcd.print(" ");                 
              }
              if (button4State == 0 && button4StatePrev == 1) {
                 button4StatePrev=0;
              }
      
              if (button5State == 1 && button5StatePrev == 0) { //INCREASE CHANNEL
                 button5StatePrev=1;
                 if (some_value > 19 && some_value < 240) some_value = some_value + 10;
                 if (some_value < 20) some_value = some_value + 1;
                 lcd.setCursor(1, 1); 
                 lcd.print("      ");
                 lcd.setCursor(1, 1); 
                 lcd.print(some_value);                 
                 lcd.print(" ");                 
              }
              if (button5State == 0 && button5StatePrev == 1) {
                 button5StatePrev=0;
              }
              if (button3State == 1 && button3StatePrev == 0) { 
                 button3StatePrev=1;
                 EEPROM.write(3,some_value);
                 delay(10);
                 edit=0;
              }
              if (button3State == 0 && button3StatePrev == 1) {
                 button3StatePrev=0;
              }
           }                      
        break;
            
        case 4://OUTCHANNEL
           lcd.setCursor(0, 0); 
           lcd.print("OUTPUT CHANNEL ");
           lcd.setCursor(1, 1);
           //String stringChannel=String(outchannel);
           //stringChannel=stringChannel + "           ";
           lcd.print(outchannel);
           if (edit == 1) {
              if (button4State == 1 && button4StatePrev == 0) { //DECREASE CHANNEL
                 button4StatePrev=1;
                 if (outchannel > 1) outchannel--;
                 lcd.setCursor(1, 1); 
                 lcd.print("      ");
                 lcd.setCursor(1, 1); 
                 lcd.print(outchannel);                 

              }
              if (button4State == 0 && button4StatePrev == 1) {
                 button4StatePrev=0;
              }
      
              if (button5State == 1 && button5StatePrev == 0) { //INCREASE CHANNEL
                 button5StatePrev=1;
                 if (outchannel < 16) outchannel++;
                 lcd.setCursor(1, 1); 
                 lcd.print("      ");
                 lcd.setCursor(1, 1); 
                 lcd.print(outchannel);              }
              if (button5State == 0 && button5StatePrev == 1) {
                 button5StatePrev=0;
              }
              if (button3State == 1 && button3StatePrev == 0) { 
                 button3StatePrev=1;
                 EEPROM.write(4,outchannel);
                 delay(10);
                 edit=0;
              }
              if (button3State == 0 && button3StatePrev == 1) {
                 button3StatePrev=0;
              }
           }                      
        break;

        case 5://INCHANNEL
           lcd.setCursor(0, 0); 
           lcd.print("INPUT CHANNEL  ");
           lcd.setCursor(1, 1);
           lcd.print(inchannel);
           if (edit == 1) {
              if (button4State == 1 && button4StatePrev == 0) { //DECREASE CHANNEL
                 button4StatePrev=1;
                 if (inchannel > 0) inchannel--;
                 lcd.setCursor(1, 1); 
                 lcd.print("      ");
                 lcd.setCursor(1, 1); 
                 lcd.print(inchannel);              }
              if (button4State == 0 && button4StatePrev == 1) {
                 button4StatePrev=0;
              }
      
              if (button5State == 1 && button5StatePrev == 0) { //INCREASE CHANNEL
                 button5StatePrev=1;
                 if (inchannel < 16) inchannel++;
                 lcd.setCursor(1, 1); 
                 lcd.print("      ");
                 lcd.setCursor(1, 1); 
                 lcd.print(inchannel);              }
              if (button5State == 0 && button5StatePrev == 1) {
                 button5StatePrev=0;
              }
              if (button3State == 1 && button3StatePrev == 0) { 
                 button3StatePrev=1;
                 EEPROM.write(5,inchannel);
                 delay(10);
                 edit=0;
              }
              if (button3State == 0 && button3StatePrev == 1) {
                 button3StatePrev=0;
              }
           }                      
        break;

        case 6: //LOAD DATA
           lcd.setCursor(0, 0); 
           lcd.print("LOAD DATA");
           lcd.setCursor(1, 1); 
           lcd.print("PRESS 2xWRITE");          
           if (edit == 1) {
              if (button3State == 1 && button3StatePrev == 0) {
                 runmode = 0;
                 button3StatePrev=1;
                 loadsequence();
                 delay(10);
                 edit=0;
                 clearscreen();
                 menu=0;          
              }
              if (button3State == 0 && button3StatePrev == 1) {
                 button3StatePrev=0;
              }
           }                      
        break;

        case 7: //SAVE DATA
           lcd.setCursor(0, 0); 
           lcd.print("SAVE DATA");
           lcd.setCursor(1, 1); 
           lcd.print("PRESS 2xWRITE");          
           if (edit == 1) {
              if (button3State == 1 && button3StatePrev == 0) { 
                 button3StatePrev=1;
                 savesequence();
                 edit=0;
                 lcd.setCursor(1, 1); 
                 lcd.print("SAVE DONE    ");
                 if (runmode == 0) delay(500);
                 clearscreen();
                 menu=0;          
              }
              if (button3State == 0 && button3StatePrev == 1) {
                 button3StatePrev=0;
              }
           }                      
        break;


        } // END OF SWITCH:CASE
   }  // END OF SETUP       
         
} // EndOfLoop
