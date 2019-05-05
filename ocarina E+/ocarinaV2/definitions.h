#define dfn

/* parametrages au reset
 *  touche 1 (si) midi on
 *  touche 2 reverb 1 ou 2( duree)
 *  touche 3 chorus on
 *  touche d'octave : - 1 octave
 *  
 *  touche 4 seule remet base de pression à 0
 */
//defintion des parametres PWM audio
#define PWM_FREQ 0x00FF // pwm frequency - 31.5KHz
#define PWM_MODE 0 // Fast (1) ou Phase Correct (0)
#define PWM_QTY 2 // 2 PWMs en parallele, pour multieffet
#define seuil 20 // valeur mini pour lire note
// definitions midi
#define noteON     0x90 //canal 1 144
#define noteOFF    0x80 //canal 1 128
#define volum      0x07
#define control    0xB0 //canal 1 , message 176, type, valeur
#define pitchBend  0xF0 //canal 1 , message 240,0,E2p
#define reverb     0x5B // E1p 91
#define tremolo    0x5C // E2n 92
#define vibrato    0x4D // E3 77
#define chorus     0x5D // E1n 93
#define timbre     0x47 // E4 71
#define velocity   110 // pour message MIDI on..on joue sur le volume par CC

#define rouge 13
#define bleu 12
#define vert 11

#define temoin 39 //test point

#define MAX_DELAY1 4400
#define min_del 10
#define max_del 200

const int MPU_addr = 0x68; // adresse I2C du MPU-6050
const int BME_addr = 0x77; // I2C address of the BME with ADO=1
SoftwareSerial midiSerial(2, 3); //pins 2 et 3 pour Rx et TX en V1, c'est 12,13
jt_BMP280 bmjet; // I2C

long time, oldtime, oldtime2; //pour faire LFO
int16_t AcX, AcY, baseX, baseY;
int lfo1, lfo2, lfo3, count;  //dents de scie pour vibrato, tremolo etc..
int p;
int note = 0, n=0, oldnote = 0, vol = 0 , oldvol = 0, effet1 = 0, effet2 = 0, effet3 = 0,effet4=0, base, base2, niveau=0, niveau1=0;
unsigned int DelayCounter1 =0;
unsigned int DelayDepth1 = MAX_DELAY1;
unsigned int touche = 0; //effet1 et effet2 pour combiner 1p et 1n, 2p et 2n
unsigned int increment1 , increment2, index, duree, duree2, tremolo1, vibrato1, pitchbend1, vpitchbend,red, blue, green;
unsigned int sample1 =2048, sample2 =2048, sample3=2048 ;
unsigned int sample=2048;
unsigned int delcount=0 , deldepth=max_del/2;


bool etatnote = false, flagencours=false;
bool moitie=false;
bool moitie1=false;
bool moitie2=false;
bool count_up=false;
byte delaybuffer1[MAX_DELAY1];
byte delbuf[max_del+10];
byte sample4, sample5; //pour high byte de sample3
byte timbre1,timbre2,vtremolo,vvibrato,voltmp, voltmp2, rev1 , rev2=16, vchorus=0; 
byte effet1p = 0, effet1n = 0, effet2n = 0, effet2p = 0;
byte olde1p = 0, olde1n = 0 , olde2n = 0 , olde2p = 0 , olde3 = 0 , olde4 = 0 ;



// init audio PWM
void InitAudioPWM(void)
{
  TCCR4A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); 
  TCCR4B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK4 = 0x20; // interrupt on capture interrupt
  ICR4H = (PWM_FREQ >> 8);
  ICR4L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // sorties
}


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  midiSerial.begin(31250);
  pinMode(6, OUTPUT); //PWM0 sortie
  pinMode(7, OUTPUT); //PWM1 sortie
  pinMode(temoin,OUTPUT); //temoin pour mesures de temps - debug

  // entrées sans pull-up interne
  init_clavier();
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     //  zero (reveille le MPU-6050)
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 4, true); // request a total of 8 registers
  baseX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  baseY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  olde1n = constrain(map((AcX - baseX), 0, 4000, 0, 100), 0, 127);
  olde1p = constrain(map((AcX - baseX), 0, -4000, 0, 100), 0, 127);
  olde2n = constrain(map((AcY - baseY), 0, 4000, 10, 100), 0, 127);
  olde2p = constrain(map((AcY - baseY), 0, -4000, 10, 100), 0, 127);
  if (!bmjet.begin())
  {
    Serial.println("probleme avec BMP280");
  }
  base =bmjet.lirepression();
  niveau=0;
  Serial.print("  base=  "); Serial.println(base);

  oldvol = 0; oldnote = 0; index=0;
  voltmp=0;
  effet3=0; effet4=0;
  increment1=0; 
  timbre1=0; timbre2=16;
  oldtime=millis();
  oldtime2=millis();
  if(!rb)
   {
    DelayDepth1=MAX_DELAY1-1000;
    Serial.print(" reverb courte ");
   }
  DelayCounter1=0;
  while (DelayCounter1<MAX_DELAY1)
  {
    DelayCounter1++;
    delaybuffer1[DelayCounter1]=0;
  }
  DelayCounter1=0;

  delcount=0;
  while (delcount<200)
  {
    delcount++;
    delbuf[delcount]=0;
  }
  delcount=0;

  InitAudioPWM();
  Serial.print("init effectuée   ");
}
/****************************************************************/
void LFO()
{
time=millis();
duree=time-oldtime;
if (duree>= 8)
  {
    oldtime=time;
    if (moitie1)
    {
      lfo1 +=(duree*7)/10;
      if (lfo1>=127)
      {
        lfo1=127;
         moitie1=false;
      }
    }
    else
    {
      lfo1 -=(duree*7)/10;
      if (lfo1<=0)
      {
        lfo1=0;
        moitie1=true;
      }
    }
  }
  if (moitie2)
    {
      lfo2 +=duree;
      if (lfo2>=127)
      {
        lfo2=127;
         moitie2=false;
      }
    }
    else
    {
      lfo2 -=duree;
      if (lfo2<=0)
      {
        lfo2=0;
        moitie2=true;
      }
    }
    if (moitie)
    {
      lfo3 +=2*duree;
      if (lfo3>=127)
      {
        lfo3=127;
         moitie=false;
      }
    }
    else
    {
      lfo3 -=2*duree;
      if (lfo3<=0)
      {
        lfo3=0;
        moitie=true;
      }
    }

}

