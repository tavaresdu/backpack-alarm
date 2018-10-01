#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

#define DISPARO 3000
#define SS_PIN 10
#define RST_PIN 9
#define LED_PIN 8
#define BUZZER_PIN 7
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

char st[20];
bool trava = LOW;
bool alarme = false;
bool som = LOW;

const int MPU = 0x68;
int AcX, AcY, AcZ, GyX, GyY, GyZ;
int AcX_lock, AcY_lock, AcZ_lock, GyX_lock, GyY_lock, GyZ_lock;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  SPI.begin();
  mfrc522.PCD_Init();
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);
  for (int i = 0; i < 7; i++)
  {
    Wire.read() << 8 | Wire.read();
  }
}

bool diff(int now, int locked)
{
  if (locked > now)
  {
    return (locked - now) > DISPARO;
  }
  else
  {
    return (now - locked) > DISPARO;
  }
}

void loop()
{
  if (trava)
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14, true);
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    Wire.read() << 8 | Wire.read();
    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();

    if (diff(AcX, AcX_lock) || diff(AcY, AcY_lock) || diff(AcZ, AcZ_lock) ||
        diff(GyX, GyX_lock), diff(GyY, GyY_lock), diff(GyZ, GyZ_lock))
    {
      alarme = true;
    }
  }

  if (alarme) {
    som = !som;
    digitalWrite(BUZZER_PIN, som);
    delay(200);
  }

  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  conteudo.toUpperCase();

  if (conteudo.substring(1) == "30 19 DB 2B")
  {
    trava = !trava;
    alarme = false;
    digitalWrite(LED_PIN, trava);
    if (trava) {
      Wire.beginTransmission(MPU);
      Wire.write(0x3B);
      Wire.endTransmission(false);
      Wire.requestFrom(MPU, 14, true);
      AcX_lock = Wire.read() << 8 | Wire.read();
      AcY_lock = Wire.read() << 8 | Wire.read();
      AcZ_lock = Wire.read() << 8 | Wire.read();
      Wire.read() << 8 | Wire.read();
      GyX_lock = Wire.read() << 8 | Wire.read();
      GyY_lock = Wire.read() << 8 | Wire.read();
      GyZ_lock = Wire.read() << 8 | Wire.read();
    }
    delay(3000);
  }
  delay(300);
}
