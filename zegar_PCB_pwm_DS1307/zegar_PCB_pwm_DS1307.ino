#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

tmElements_t tm;

//#define Serial_debug
#define automatyczna_jasnosc
//#define kalibracja_jasnosci //tylko gdy #define automatyczna_jasnosc

// GDY AUTOMATYCZNA JASNOSC
const int jasnosc[2] = {200, 2000}; //us
int zakresy[2] = {520, 700}; //nizsza, wyzsza
unsigned const PWM_time = 13; //us, nie dotyczy, kiedy #define kalibracja_jasnosci

//GDY JASNOŚĆ GODZINOWA
const uint8_t godziny[2] = {8, 16}; //8

//////////////////////////////////////////////////////////////////////////////////

const uint8_t roznica = 25;

int delay_time = 0;
int poprzedni_wynik = 0;
int repeating = 0;
int h = 0;

bool LED = true;

bool segmenty[4][7] = 
{{false, false, false, false, false, false, false},
 {false, false, false, false, false, false, false},
 {false, false, false, false, false, false, false},
 {false, false, false, false, false, false, false}};

const bool cyfry[10][7] = 
{
  {true, true, true, true, true, true, false},
  {false, true, true, false, false, false, false},
  {true, true, false, true, true, false, true},
  {true, true, true, true, false, false, true},
  {false, true, true, false, false, true, true},
  {true, false, true, true, false, true, true},
  {true, false, true, true, true, true, true},
  {true, true, true, false, false, false, false},
  {true, true, true, true, true, true, true},
  {true, true, true, true, false, true, true}
};

uint8_t cyfry_do_zegara[4] = {4, 5, 6, 7};

void setup() {
  //pobieranie czasu podczas kompilacji i ustawianie go w RTC
  bool parse=false;
  bool config=false;

  if (getTime(__TIME__)) {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  }

  #ifdef Serial_debug
    if (parse && !config) {
      Serial.println("DS1307 Communication Error :-{");
      Serial.println("Please check your circuitry");
    } 
    else if(!parse && config) {
      Serial.print("Could not parse info from the compiler, Time=\"");
      Serial.print(__TIME__);
      Serial.print("\", Date=\"");
      Serial.print(__DATE__);
      Serial.println("\"");
    }
  #endif

  if(!config && !parse) while(1);

  //////////////////////////////////////////

  for(int i = 2; i <= 13; i++) pinMode(i, OUTPUT);
  pinMode(A7, INPUT);

  #ifdef Serial_debug
    Serial.begin(9600);
  #endif
  #ifdef kalibracja_jasnosci
    Serial.begin(9600);
  #endif

  int w = 0;
  for(int i = 1; i <= 5; i++)
  {
    w += analogRead(A7);
    delay(500);
  }

  w /= 5;
  zakresy[0] = w + 25;

  ktora_godzina();
  brightness();
}

void loop() 
{
  for(int a = 0; a <= 20; a++)
  {
    for(int i = 0; i <= repeating; i++)
    {
      for(int wyswietlacz_nr = 1; wyswietlacz_nr <= 4; wyswietlacz_nr++) wyswietlanie_cyfr(wyswietlacz_nr);
    }

    brightness();
    LED = !LED;
  }
  
  ktora_godzina();
}

void wyswietlanie_cyfr(uint8_t wyswietlacz) // 1 - 4
{
  digitalWrite(13, LOW);

  for(int i = 9; i <= 12; i++) digitalWrite(i, HIGH);
  for(int i = 2; i <= 8; i++) digitalWrite(i, segmenty[wyswietlacz - 1][i-2]);

  digitalWrite(wyswietlacz+8, LOW);

  if(wyswietlacz == 1) digitalWrite(13, LED);

  if(wyswietlacz == 4 && delay_time >= 250) delayMicroseconds(delay_time - 250);
  else if(wyswietlacz == 1) delayMicroseconds(delay_time + 10);
  else delayMicroseconds(delay_time);
}

void zmiana_segmentow(uint8_t wyswietlacz, uint8_t cyfra, bool odwracanie_pinami, bool odwracanie_lustro) //0 - pierwsza z ostatnią itd. 1 - lustrzane odbicie
{
  bool stany[7];
  for(int i = 0; i <= 6; i++) stany[i] = cyfry[cyfra][i];

  if(odwracanie_pinami)
  {
    bool stany_odwrocone[7];

    for(int i = 0; i <= 6; i++) stany_odwrocone[i] = stany[i];
    for(int i = 0; i <= 6; i++) stany[6-i] = stany_odwrocone[i];
  }
  if(odwracanie_lustro)
  {
    bool stany_odwrocone[7];

    for(int i = 0; i <= 6; i++) stany_odwrocone[i] = stany[i];

    stany_odwrocone[1] = stany[5];
    stany_odwrocone[2] = stany[4];
    stany_odwrocone[4] = stany[2];
    stany_odwrocone[5] = stany[1];

    for(int i = 0; i <= 6; i++) stany[i] = stany_odwrocone[i];
  }

  for(int i = 0; i <= 6; i++) segmenty[wyswietlacz - 1][i] = stany[i];
}

void ktora_godzina()
{
  if (!RTC.read(tm)) 
  {
    #ifdef Serial_debug
      if (RTC.chipPresent()) Serial.println("The DS1307 is stopped.  Please run the SetTime example to initialize the time and begin running.");
      else Serial.println("DS1307 read error!  Please check the circuitry.");
    #endif

    while(1);
  }

  cyfry_do_zegara[1] = tm.Hour / 10;
  cyfry_do_zegara[0] = tm.Hour % 10;
  cyfry_do_zegara[3] = tm.Minute / 10;
  cyfry_do_zegara[2] = tm.Minute % 10;

  for(int i = 1; i <= 4; i++) zmiana_segmentow(i, cyfry_do_zegara[i-1], i == 2 || i == 3, i == 3 || i == 4);

  #ifdef Serial_debug
    for(int i = 0; i <= 3; i++) Serial.print(cyfry_do_zegara[i] + " | ");
    Serial.println();
  #endif
}

void brightness()
{
  #ifndef automatyczna_jasnosc
    repeating = (tm.Hour > godziny[0] && h < godziny[1]) ? 63 : 2010;
    delay_time = (tm.Hour > godziny[0] && h < godziny[1]) ? 2000 : 0;
  #else
    //Do zrobienia
    int a = analogRead(A7);

    delay_time = (a > zakresy[0]) ? 0 : 2000;
    repeating = (delay_time == 2000) ? 63 : 2010;

    #ifdef kalibracja_jasnosci
      Serial.print(zakresy[0]);
      Serial.print(" | ");
      Serial.println(a);
    #endif
  #endif
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}
