#include <SoftwareSerial.h>
#include <Wire.h>
#include "jtBMP280.h"
#include "table_musique.h"
#include "sax256_byte.h"  
#include "sinus256_byte2.h"
#include "clavier.h"
#ifndef dfn
 #include "definitions.h"
#endif


/*************************************************************/
void loop() 
{
  time=millis();
  duree2=time-oldtime2;
  if (duree2>=40)          //limite usage des ressources IT
  {    
  //Serial.println(vol);                    
  oldtime2=time;
  lecture_capteurs();
  touche=lecture_touche();
  //Serial.println(touche);
  if (touche==8) base =bmjet.lirepression();  //reset base si note interdite volontaire...ne pas souffler
  if (vol <= seuil-5)
   {
     etatnote=false;
     analogWrite(rouge,0);
     analogWrite(vert,0);
     analogWrite(bleu,0);
     if (note!=0)
     {
      message(noteOFF, note, 0);
      note = 0; 
      oldnote = 0;
     }
   }
  else
   {
   if (vol >= (seuil + 10)) etatnote=true; //ici tester volume, si trop faible, on ne lit pas les touches
   }
  if (etatnote)
   {
     if (note==0)
     {
      message(noteOFF, note, 0); 
      etatnote = false;
      oldnote = 0; 
     }
    note=decodage_touche(touche);
    if ((oldnote != note)&&(note!=0))
    {
      message(noteOFF, oldnote, 0);
      message(noteON, note, velocity);
      message(control, volum, vol );
      oldnote = note;
    }
    if (note!=0)
    {
     controlchange();
     vol=(vol*((16384-((lfo3*vtremolo/2)))>>7))>>7; 
     voltmp=vol>>3;
     ecritLED();
    }
   // delay(2);
   if (pb)  increment1=((long)increment2*(128+vpitchbend))>>7;
   if (vb)  increment1=(increment2/2*((16384-(lfo1*vvibrato/8))>>8))>>5; //attention cadrage des nombres
  }
 else voltmp=0;
 LFO();
 }
}

/****************************************************************************/
// fonctions


void ecritLED()
{
  red=((127-(n*5))*vol)>>6;
  blue= (n*5*vol)>>6;
  if (n<=12)
  {
   green=(n*10*vol)>>6; 
  }
  else
  {
    green=((127-((n-13)*10))*vol)>>6;
  }
  //Serial.print("n= ");Serial.print(n);Serial.print("R= ");Serial.print(red);Serial.print("V= ");Serial.print(green);Serial.print("B= ");Serial.println(blue);
  analogWrite(rouge,red);
  analogWrite(vert,blue);
  analogWrite(bleu,green);
}

/*****************/
void controlchange()
{
  //test variation des CC, et message midi sont > 5
  if (abs(effet2n - olde2n) > 10)
  {
   message(control, tremolo, effet2n);
   olde2n = effet2n;
  }
  if (abs(effet2p - olde2p) > 10)
  {
    message(control, vibrato, effet2p);
    olde2p = effet2p;
  }
  if (abs(effet1n - olde1n) > 10)
  {
    message(control, pitchBend, effet1n);
    olde1n = effet1n;
   }
  if (abs(effet1p - olde1p) > 10)
  {
    //Serial.print("reverb = "); Serial.println(effet1p);
    message(control, reverb, effet1p);
    olde1p = effet1p;
  }
  if (abs(effet3 - olde3) > 10)
  {
   message(control, timbre, effet3);
   olde3 = effet3;
  }
  if (abs(vol  - oldvol) > 10)
  {
    message(control, volum, vol );
    oldvol = vol;
  }
  if (abs(effet4  - olde4) > 10)
  {
    message(control, chorus, effet4 );
    olde4 = effet4;
  }
  //imprime();
}

void imprime()
{  //exemple de fonction
  Serial.print("\t volume = ");  Serial.print(voltmp);  Serial.print("\t note = ");  Serial.println(note);
  Serial.print("\t touche= "); Serial.print(touche);Serial.print("\t volume2 = ");  Serial.print(vol);
  Serial.print("\t E3=");Serial.print(effet3);Serial.print("  E1 = ");Serial.print(effet1);Serial.print("  E2 = ");Serial.println(effet2);
  Serial.print("increment =  ");Serial.println(increment1);
}

void lecture_capteurs() 
{
  // lecture des capteurs

  effet3 = effet3/2 + (constrain(map(analogRead(A1), 500, 950, 0, 120), 0, 120))/2;  // filtrage PB
  effet4 = effet4/2 + (constrain(map(analogRead(A2), 500, 950, 0, 127), 0, 120))/2;  // filtrage PB

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 4, true); // request a total of 4 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  effet1n = effet1n*3/4 + (constrain(map((AcX - baseX), 0, 7000, -20, 120), 0, 127))/4;
  effet1p = effet1p*3/4 +(constrain(map((AcX - baseX), 0, -7000, -10, 120), 0, 127))/4;
  effet2n = effet2n*3/4 + (constrain(map((AcY - baseY), 0, 8000, -50, 140), 0, 127))/4;
  effet2p = effet2p*3/4 + (constrain(map((AcY - baseY), 0, -8000, 0, 100), 0, 127))/4;
  effet1 = 64 + effet1p / 2 - effet1n / 2;
  effet2 = 64 + effet2p / 2 - effet2n / 2;

  niveau1 =bmjet.lirepression() - base;
  niveau=niveau*3/4 + niveau1/4;
  vol= constrain(map(niveau, 0 , 110, 0, 130), 0, 127);
  //imprime();
 // vol=effet4;
 //vol=100;
  }


void MIDImessage(byte command, byte MIDInote, byte MIDIvelocity)
{
  midiSerial.write(command+canal);
  midiSerial.write(MIDInote);
  midiSerial.write(MIDIvelocity);
}

void message(byte type, byte note, byte param)
{
 if (MidiOn)
 {
  MIDImessage(type, note, param);
  //Serial.print("MIDI type note param   ");Serial.print(type);Serial.print ("/t");Serial.print(note);Serial.print ("/t");Serial.println(param);
 }
  switch(type)
  {
   case noteON:  
      if (oct)
      { 
        increment2=increment[note-33];
        n=note-45;
      }
      else 
      {
        increment2=increment[note-21];
        n=note-33;
      }
      increment1=increment2;
      etatnote=true;
      pb=false;
      vb=false;
      break; 
   case noteOFF:  
      increment1=0; 
      etatnote=false;
      pb=false;
      vb=false;
      //Serial.print("note off ");
      break;
  case control:
    {
      switch(note)
      {
        case volum:
         break;
       case tremolo:
         vtremolo=(constrain((param-35), 0, 127));
         break;
      case vibrato:
      vvibrato=constrain((param-40), 0, 100);
        if (vvibrato>=5)  vb=true;
        else             vb=false;
        break;
      case timbre:
       timbre1=(constrain(param,0,127))>>3;
       timbre2=16-timbre1;
       //Serial.print("timbre1  = ");Serial.print(timbre1);Serial.print("timbre2  = ");Serial.println(timbre2);
       break;
     case reverb:
       rev1=constrain((param>>3),0,16);
       rev2=16-(rev1/2);
       //Serial.print("r1  = ");Serial.print(rev1);Serial.print("r2  = ");Serial.println(rev2);
      // break;
      break;
     case chorus:
     vchorus=effet4/8;
      break;
     case pitchBend:
      vpitchbend=constrain((param-20)/4, 0, 20);
      if(vpitchbend>=2)
      {
       pb=true;  
      }
      else
      {
        pb=false;
      }
      break;
     default:
      break;
     }
   }
 }
}


/************************************************************************************/
ISR(TIMER4_CAPT_vect) 
{
  byte index1;
  
   //digitalWrite(temoin,HIGH);
   OCR4AL = (sample5*voltmp)>>4 ; // direct 
   OCR4BL = sample4;  //reverb
   index+= increment1;   
   index1=index>>8;
   sample1=sin256byte[index1];  //calculer *timbre (entre 0 et 127)>>7
   sample2=sax256byte[index1];  //calculer * 1-timbre (entre 127 et 0 )>>7
   sample=((sample1*timbre2/2)+(sample2*timbre1/2));
   sample5=sample>>3;
   if (MidiOn) sample5=sample5/4;
   //sample4 = sample5;
   /*************************** reverb **********/
    delaybuffer1[DelayCounter1]=(sample5+delaybuffer1[DelayCounter1])/2;
    sample4=(delaybuffer1[DelayCounter1]*rev1)>>4;  
    DelayCounter1++;
    if (DelayCounter1>=DelayDepth1) DelayCounter1=0;  
  /*************************** chorus *********/
   delbuf[delcount]=(sample5*vchorus)>>4; 
   delcount++;
   if(delcount >= deldepth) //(entre min et max)
   {
    delcount = 0; 
    if(count_up)
    {
       for(p=0;p<2;p++)delbuf[deldepth+p]=delbuf[deldepth-1]; 
       deldepth++;       
       if (deldepth>=max_del)count_up=false;
    }
    else
    {
       deldepth--;
       if (deldepth<=min_del)count_up=true;
    }
   }
 /******************double effet *******************/  
  sample4=(delbuf[delcount]+sample4)>>1; 
  //digitalWrite(temoin,LOW);
}




