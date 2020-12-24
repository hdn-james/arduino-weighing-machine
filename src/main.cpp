#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ThingSpeak.h>

#define DT 16
#define SCK 17
#define sw 9

LiquidCrystal_I2C lcd(0x27, 16, 2);

long sample = 0;
float val = 0;
long count = 0;

unsigned channelID = 1174501;
const char *myCounterWriteAPIKey = "UV9FTX7K5D8N5U22";
char *ssid = "Phuc Long";
char *pass = "Luckytea";
const char *server = "https://api.thingspeak.com/channels/1174501/feeds.json?api_key=YO32BRAWKRDXDOQ4&results=3";
WiFiClient client;
int w;
int temp = -1;

void calibrate();

unsigned long readCount(void)
{
  unsigned long Count;
  unsigned char i;
  pinMode(DT, OUTPUT);
  digitalWrite(DT, HIGH);
  digitalWrite(SCK, LOW);
  Count = 0;
  pinMode(DT, INPUT);
  while (digitalRead(DT))
    ;
  for (i = 0; i < 24; i++)
  {
    digitalWrite(SCK, HIGH);
    Count = Count << 1;
    digitalWrite(SCK, LOW);
    if (digitalRead(DT))
      Count++;
  }
  digitalWrite(SCK, HIGH);
  Count = Count ^ 0x800000;
  digitalWrite(SCK, LOW);
  return (Count);
}

void setup()
{
  pinMode(SCK, OUTPUT);
  pinMode(sw, INPUT_PULLUP);
  WiFi.begin(ssid, pass);
  ThingSpeak.begin(client);
  lcd.init();
  lcd.backlight();
  lcd.print(" Weight ");
  lcd.setCursor(0, 1);
  lcd.print(" Measurement ");
  delay(1000);
  lcd.clear();
  calibrate();
}

void loop()
{
  count = readCount();
  w = (((count - sample) / val) - 2 * ((count - sample) / val));
  lcd.setCursor(0, 0);
  lcd.print("Measured Weight");
  lcd.setCursor(0, 1);
  lcd.print(w);
  lcd.print("g      ");

  ThingSpeak.writeField(channelID, 1, w, myCounterWriteAPIKey);

  if (digitalRead(sw) == 0)
  {
    if (temp == -1)
    {
      temp = val;
      val = 0;
      lcd.print("Please Wait....");
      delay(200);
      for (int i = 0; i < 100; i++)
      {
        count = readCount();
        val += sample - count;
      }
      val = val / 100.0;
      val = val / float(w); //get current weight
    }
    else
    {
      val = temp;
      temp = -1;
    }
  }
}

void calibrate()
{
  lcd.clear();
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait...");
  for (int i = 0; i < 100; i++)
  {
    count = readCount();
    sample += count;
  }
  sample /= 100;
  lcd.clear();
  lcd.print("Put 100g & wait");
  count = 0;
  while (count < 1000)
  {
    count = readCount();
    count = sample - count;
  }
  lcd.clear();
  lcd.print("Please Wait....");
  delay(2000);
  for (int i = 0; i < 100; i++)
  {
    count = readCount();
    val += sample - count;
  }
  val = val / 100.0;
  val = val / 100.0; // put here your calibrating weight
  lcd.clear();
}
