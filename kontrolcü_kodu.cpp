/*
  Arduino Mega ile 42 Dijital + 42 Analog Giriş (Toplam 84 Giriş) Örneği
  ---------------------------------------------------------------------
  - 3 adet MCP23017: Her biri 16 dijital I/O => Toplam 48 dijital kanal
  - 3 adet 74HC4067: Her biri 16 analog kanal => Toplam 48 analog kanal

*/

#include <Wire.h>              // I2C haberleşmesi
#include <Adafruit_MCP23017.h> // MCP23017 kütüphanesi

//----------------------------------------
// MCP23017 Tanımları
//----------------------------------------
#define MCP_COUNT 3               // Kaç adet MCP23017 kullanıldığı
Adafruit_MCP23017 mcp[MCP_COUNT]; // MCP nesnelerini dizi olarak tutuyoruz

// MCP'lerin I2C adres dizisi (A0,A1,A2 pinlerine göre).
// Örneğin begin(0)=0x20, begin(1)=0x21, begin(2)=0x22
uint8_t mcpAddresses[MCP_COUNT] = {0, 1, 2};

//----------------------------------------
// 74HC4067 Tanımları
//----------------------------------------
#define MUX_COUNT 3 // Kaç adet 74HC4067 kullanıldığı
// Bu entegrelerin ortak analog çıkışlarının bağlı olduğu Arduino analog pinleri
uint8_t muxAnalogPins[MUX_COUNT] = {A0, A1, A2};

// 74HC4067 Seçme pinleri (S0,S1,S2,S3) - Hepsi ortak kullanılıyor
const int s0 = 2;
const int s1 = 3;
const int s2 = 4;
const int s3 = 5;

// Enable pinleri (aktif LOW). Her 74HC4067 için ayrı enable (kapat/aç) hattı.
uint8_t muxEnablePins[MUX_COUNT] = {6, 7, 8};

// Her multiplexer’da okuyacağımız kanal sayısı (maks. 16).
// Örneğin ilk ikisi 16’şar kanal, üçüncüsü 10 kanal kullanarak toplam 42 analog sensör.
uint8_t muxChannelCount[MUX_COUNT] = {16, 16, 10};

//----------------------------------------
// Kurulum (setup) Fonksiyonu
//----------------------------------------
void setup()
{
    Serial.begin(115200);
    Wire.begin(); // I2C hattı başlat

    // -- MCP23017'lerin başlatılması ve pin ayarları --
    for (int i = 0; i < MCP_COUNT; i++)
    {
        // Her bir MCP23017 için I2C adresini belirliyoruz
        mcp[i].begin(mcpAddresses[i]);
        // Tüm pinleri giriş (INPUT) yapalım (42 adet dijital girişi karşılayabilir)
        for (uint8_t pinNum = 0; pinNum < 16; pinNum++)
        {
            mcp[i].pinMode(pinNum, INPUT);
            // İç pull-up gerekli ise açabilirsiniz:
            // mcp[i].pullUp(pinNum, HIGH);
        }
    }

    // -- 74HC4067 pinlerinin ayarlanması --
    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);
    pinMode(s3, OUTPUT);

    // Başlangıçta tüm seçme pinleri LOW
    digitalWrite(s0, LOW);
    digitalWrite(s1, LOW);
    digitalWrite(s2, LOW);
    digitalWrite(s3, LOW);

    // Her MUX için enable pinlerini ayarla (HIGH => devre dışı)
    for (int i = 0; i < MUX_COUNT; i++)
    {
        pinMode(muxEnablePins[i], OUTPUT);
        digitalWrite(muxEnablePins[i], HIGH);
    }

    Serial.println("Sistem basladi: MCP23017 ve 74HC4067 ayarlari tamam.");
}

//----------------------------------------
// MCP23017 Tüm Dijital Girişleri Okuma
//----------------------------------------
void readAllDigitalInputs()
{
    for (int m = 0; m < MCP_COUNT; m++)
    {
        for (uint8_t pinNum = 0; pinNum < 16; pinNum++)
        {
            int state = mcp[m].digitalRead(pinNum);
            // Örnek olarak seri porta yazdırıyoruz
            Serial.print("MCP");
            Serial.print(m);
            Serial.print(" Pin ");
            Serial.print(pinNum);
            Serial.print(" = ");
            Serial.println(state);
        }
    }
}

//----------------------------------------
// 74HC4067 Tek Kanal Okuma Fonksiyonu
//----------------------------------------
int readMuxChannel(int muxIndex, int channel)
{
    // Multiplexer seçimi (enable pin)
    for (int i = 0; i < MUX_COUNT; i++)
    {
        if (i == muxIndex)
        {
            // Seçili mux aktif => LOW
            digitalWrite(muxEnablePins[i], LOW);
        }
        else
        {
            // Diğerleri devre dışı => HIGH
            digitalWrite(muxEnablePins[i], HIGH);
        }
    }

    // Kanal seçimi (S0–S3 pinleri)
    digitalWrite(s0, (channel & 0x01) ? HIGH : LOW);
    digitalWrite(s1, (channel & 0x02) ? HIGH : LOW);
    digitalWrite(s2, (channel & 0x04) ? HIGH : LOW);
    digitalWrite(s3, (channel & 0x08) ? HIGH : LOW);

    // Anahtarlama süresi için kısa gecikme
    delayMicroseconds(50);

    // Seçili mux’un ortak çıkışı hangi analog pine bağlıysa ordan okumayı yapıyoruz
    int analogPin = muxAnalogPins[muxIndex];
    int value = analogRead(analogPin);
    return value;
}

//----------------------------------------
// 74HC4067 Tüm Analog Sensörleri Okuma
//----------------------------------------
void readAllAnalogSensors()
{
    for (int muxIdx = 0; muxIdx < MUX_COUNT; muxIdx++)
    {
        for (int ch = 0; ch < muxChannelCount[muxIdx]; ch++)
        {
            int sensorValue = readMuxChannel(muxIdx, ch);
            // Örnek olarak seri porta yazdırıyoruz
            Serial.print("MUX");
            Serial.print(muxIdx);
            Serial.print(" Kanal ");
            Serial.print(ch);
            Serial.print(" = ");
            Serial.println(sensorValue);
        }
    }
}

//----------------------------------------
// Ana Döngü (loop) Fonksiyonu
//----------------------------------------
void loop()
{
    // 1) 42 (veya 48) dijital girişi MCP23017 üzerinden oku
    readAllDigitalInputs();

    // 2) 42 (veya 48) analog sensörü 74HC4067 üzerinden oku
    readAllAnalogSensors();

    Serial.println("------------------------------------------");
    delay(1000); // Her tam taramadan sonra 1 saniye bekleyelim
}
