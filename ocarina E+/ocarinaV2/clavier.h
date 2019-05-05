#define push2 23 //valeur 1  v2 35
#define push3 25 // valeur 2 v1 23
#define push4 27 //valeur 4 v1 25
#define push5 29 //valeur 8 v1 27
#define push6 31 //valeur 16 v1 29
#define push7 33 //valeur 32 v1 31
#define push8 35 //valeur 64 - octave v1 33

bool MidiOn=false; //valide la sortie midi
bool pb=false; // pitchbend actif
bool vb=false; //vibrato actif
bool rb; //reverb active
bool oct;
bool chor;   //chorus à venir
byte canal=1;

/***********************************/
int lecture_touche() {
  int touch;
  // read the input pin:
  touch = 0;
   if (digitalRead(push2)==0)    touch = touch + 1;
   if (digitalRead(push3)== 0)   touch = touch + 2;
   if (digitalRead(push4) == 0)  touch = touch + 4;
   if (digitalRead(push5) == 0)  touch = touch + 8;
   if (digitalRead(push6) == 0)  touch = touch + 16;
   if (digitalRead(push7) == 0)  touch = touch + 32;
  return (touch);
}

 int decodage_touche(int touch) 
{    //peut s'adapter à autres doigtés
  int notex;
  switch (touch) {
    case 0: notex = 48; // do3 interdit, filtre le volume
      break;
    case 1: notex = 47; // si2
      break;
    case 8: notex = 47; // do bemol
      break;
    case 9: notex = 46;
      break;
    case 16: notex = 45; // la2
      break;
    case 24: notex = 44;
      break;
    case 48: notex = 43; //sol2
      break;
    case 56: notex = 42; // sol bemol
      break;
    case 49: notex = 41; //fa2
      break;
    case 57: notex = 40; // fa2 bemol
      break;
    case 50: notex = 40; // mi2
      break;
    case 58: notex = 39; // mi bemol
      break;
    case 52: notex = 38; //ré2
      break;
    case 60: notex = 37;
      break;
    case 54: notex = 36; // do2
      break;
    case 62: notex = 35; // do2 bemol // si2
      break;
    case 55: notex = 34; // si2 bemol
      break;
    case 63: notex = 33; // la2 
      break;
    default: 
    notex = 0;
  }
   if ((digitalRead(push8) == 0) && (notex != 0))     notex = notex + 12; // octave
   return(notex);
}

void init_clavier()
{
  pinMode(push2, INPUT);
  pinMode(push3, INPUT);
  pinMode(push4, INPUT);
  pinMode(push5, INPUT);
  pinMode(push6, INPUT);
  pinMode(push7, INPUT);
  pinMode(push8, INPUT);  
  if (digitalRead(push2)==0)
  { MidiOn=true;
  Serial.print("midi on canal; ");
  canal=0;
  if(digitalRead(push6)==0)
    {
    canal=canal+1;
   // Serial.print("push6   ");Serial.println(canal+1);
    }
  if(digitalRead(push7)==0)
   {
    canal=canal+2;
   // Serial.print("push7   ");Serial.println(canal+1);
   }
   Serial.println(canal+1);
 }
  if (digitalRead(push3)==0) rb=false; else rb=true;
  if (!rb)  Serial.print("reverb mode 2");
  if(digitalRead(push8)==0)oct=true; else oct=false;
  if (oct)  Serial.print("octave basse");
  /*if(digitalRead(push6)==0)chor=true; else chor=false;
  if (chor)  Serial.print("chorus ");*/
}

