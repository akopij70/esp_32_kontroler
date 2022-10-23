#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <BleMouse.h>

#define INTERVAL 25
#define NOT_PRESSED 1
#define PRESSED 0
#define LEFT_MOUSE 19
#define RIGHT_MOUSE 23
#define NORMAL_CLICKER 4
#define NUM_OF_CLICKERS 3

void StartMpu();
void SetActions();

MPU6050 mpu(Wire);
BleMouse ble_mouse("totylkotest");

int clickers[NUM_OF_CLICKERS] = {19, 23, 4};
int active_mouse_buttons[NUM_OF_CLICKERS] = {5, 18}; // RPM, LPM
double X, Y, prevX, prevY;
unsigned long timer = 0;

void setup()
{
    for (int i = 0; i < NUM_OF_CLICKERS; i++)
    {
        pinMode(clickers[i], INPUT_PULLUP);
        pinMode(active_mouse_buttons[i], OUTPUT);
    }
    Serial.begin(115200);
    while (!Serial)
        delay(10);
    Serial.println("Starting BLE work!");
    ble_mouse.begin();
    StartMpu();
    Serial.println("Done!\n");
}

void loop()
{
    if (((millis() - timer) > INTERVAL) && ble_mouse.isConnected())
    {

        /*
        while (digitalRead(LEFT_MOUSE) == PRESSED)
        {
            ble_mouse.move(1,0);
            delay(INTERVAL);
        }
        while (digitalRead(RIGHT_MOUSE) == PRESSED)
        {
            ble_mouse.move(10,0);
            delay(INTERVAL);
        }

        */
        SetActions();
        timer = millis();
    }
}

void StartMpu()
{
    Wire.begin();
    byte status = mpu.begin();
    Serial.print(F("MPU6050 status: "));
    Serial.println(status);
    while (status != 0)
        ; // czekaj na polaczenie
    Serial.println(F("Calculating offsets, do not move MPU6050"));
    delay(1000);
    mpu.calcOffsets(); // gyro and accelero
}

void SetActions()
{
    mpu.update();
    X = mpu.getAngleX();
    Y = mpu.getAngleY();
    double click_guard = X;
    double horizontal, vertical;
    if (digitalRead(LEFT_MOUSE) == PRESSED)
        Serial.print("LPM wcisniety");
    if (digitalRead(RIGHT_MOUSE) == PRESSED)
        Serial.print("\tRPM wcisniety");
    if (digitalRead(NORMAL_CLICKER) == PRESSED)
        Serial.print("\tklikacz wcisniety");

    if (Y > 75.0)
    {
        Y = 75.0;
        X = prevX;
    }
    else if (Y < -75.0)
    {
        Y = -75.0;
        X = prevX;
    }
    else if (X > 85.0)
        X = 85.0;
    else if (X < -85.0)
        X = -85.0;

    prevY = Y;
    prevX = X;

    if (digitalRead(LEFT_MOUSE) == PRESSED && digitalRead(RIGHT_MOUSE) == NOT_PRESSED)
    {
        while (click_guard > -25.0 && digitalRead(LEFT_MOUSE) == PRESSED)
        {
            mpu.update();
            click_guard = mpu.getAngleX();
            // ble_mouse.click();
        }
        while (click_guard <= -25.0 && digitalRead(LEFT_MOUSE) == PRESSED)
        {
            Serial.print("\tpowinno swiecic LPM");
            digitalWrite(active_mouse_buttons[1], HIGH);
            digitalWrite(active_mouse_buttons[0], LOW);
            mpu.update();
            ble_mouse.press();
            delay(INTERVAL);
        }
        ble_mouse.release();
        X = 0.0;
        Y = 0.0;
        delay(INTERVAL);

        /*
        Serial.print("\tpowinno swiecic LPM");
        digitalWrite(active_mouse_buttons[1], HIGH);
        digitalWrite(active_mouse_buttons[0], LOW);
        game_controller.press(BUTTON_7);
        game_controller.sendReport();
        delay(INTERVAL);
        game_controller.release(BUTTON_7);
        game_controller.sendReport();
        */
    }

    else if (digitalRead(RIGHT_MOUSE) == PRESSED && digitalRead(LEFT_MOUSE) == NOT_PRESSED)
    {
        while (digitalRead(RIGHT_MOUSE) == PRESSED)
        {
            Serial.print("\tpowinno swiecic RPM");
            digitalWrite(active_mouse_buttons[1], LOW);
            digitalWrite(active_mouse_buttons[0], HIGH);
            mpu.update();
            ble_mouse.press(MOUSE_RIGHT);
            X = mpu.getAngleX();
            Y = mpu.getAngleY();
            if (abs(X) > 15.0)
                vertical = map(X, -85, 85, 30, -30);
            else
                vertical = 0;
            if (abs(Y) > 15.0)
                horizontal = map(Y, -75, 75, -30, 30);
            else
                horizontal = 0;
            ble_mouse.move(horizontal, vertical);
            delay(INTERVAL);
        }
        ble_mouse.release(MOUSE_RIGHT);

        /*
        Serial.print("\tpowinno swiecic RPM");
        digitalWrite(active_mouse_buttons[1], LOW);
        digitalWrite(active_mouse_buttons[0], HIGH);
        game_controller.press(BUTTON_8);
        game_controller.sendReport();
        delay(INTERVAL);
        game_controller.release(BUTTON_8);
        game_controller.sendReport();
        */
    }
    
    else if (digitalRead(NORMAL_CLICKER) == PRESSED && digitalRead(RIGHT_MOUSE) == NOT_PRESSED && digitalRead(LEFT_MOUSE) == NOT_PRESSED)
    {
        while (digitalRead(NORMAL_CLICKER) == PRESSED)
        {
            Serial.print("KLIKACZ ZWYKLY");
            digitalWrite(active_mouse_buttons[0], LOW);
            digitalWrite(active_mouse_buttons[1], HIGH);
            mpu.update();
            ble_mouse.press();
            X = mpu.getAngleX();
            Y = mpu.getAngleY();
            if (abs(X) > 15.0)
                vertical = map(X, -85, 85, 30, -30);
            else
                vertical = 0;
            if (abs(Y) > 15.0)
                horizontal = map(Y, -75, 75, -30, 30);
            else
                horizontal = 0;
            ble_mouse.move(horizontal, vertical);
            delay(INTERVAL);
        }
        ble_mouse.release();

        /*
        game_controller.press(BUTTON_1);
        game_controller.sendReport();
        delay(INTERVAL);
        game_controller.release(BUTTON_1);
        game_controller.sendReport();
        */
    }
    else
    {
        digitalWrite(active_mouse_buttons[0], LOW);
        digitalWrite(active_mouse_buttons[1], LOW);
    }

    Serial.print("\tX : ");
    Serial.print(X);
    Serial.print("\tY : ");
    Serial.println(Y);
    if (abs(X) > 15.0)
        vertical = map(X, -85, 85, 25, -25);
    else
        vertical = 0;
    if (abs(Y) > 15.0)
        horizontal = map(Y, -75, 75, -25, 25);
    else
        horizontal = 0;
    ble_mouse.move(horizontal, vertical);
}

/*
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <BleConnectionStatus.h>
#include <BleMouse.h>

BleMouse bleMouse("mysz");
const int VRxPin = 34;
const int VRyPin = 35;
int potValueX = 0, potValueY = 0;
int centrum_PortX, centrum_PortY, licznik_usrednienia;
#define ILE_USREDNIEN 50
bool stat_centr;
int pauza;
int wektorX = 0, wektorY = 0;
static double przesuw_x_ulamek = 0, przesuw_y_ulamek = 0;
bool WyliczCentrumXY(int ValueX, int ValueY);
bool KonwersjaADC_przesuwKursora(
    int *p_pauza, int *p_wektorX, int *p_wektorY, int ValueX, int ValueY);
double ObliczPrzesuw(int przesuw, bool przesuw_plus, double przesuw_ulamek);
// double ObliczPrzesuw1(int przesuw,bool przesuw_plus,double przesuw_ulamek);
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    while(!Serial)
        delay(10);
    Serial.println("Starting BLE work!");
    bleMouse.begin();
    centrum_PortX = 0;
    centrum_PortY = 0;
    licznik_usrednienia = 0;
    delay(500);
}

void loop()
{
    // put your main code here, to run repeatedly:
    potValueX = analogRead(VRxPin);
    potValueY = analogRead(VRyPin);
    stat_centr = WyliczCentrumXY(potValueX, potValueY);
    if (stat_centr == false)
    {
        pauza = 10;
    }
    else
    {
        bool stat_poz = KonwersjaADC_przesuwKursora(
            &pauza, &wektorX, &wektorY, potValueX, potValueY);
        Serial.println("pauza, x, y, potX, potY");
        Serial.print(pauza);
        Serial.print(" ;");
        Serial.print(wektorX);
        Serial.print(" ;");
        Serial.print(wektorY);
        Serial.print(" ;");
        Serial.print(potValueX);
        Serial.print(" ;");
        Serial.println(potValueY);
        if (bleMouse.isConnected())
        {
            bleMouse.move(wektorY, wektorX);
        }
        delay(pauza);
    }
}
/* wyliczanie centrum X,Y poprzez uśrednione pomiary
we: wartości odczytane z przetwornika dla portów X i Y joystika
wy: true -centra XY wyliczone */
/*
bool WyliczCentrumXY(int ValueX, int ValueY)
{
    if (licznik_usrednienia >= ILE_USREDNIEN)
        return true;
    centrum_PortX += ValueX;
    centrum_PortY += ValueY;
    licznik_usrednienia++;
    if (licznik_usrednienia >= ILE_USREDNIEN)
    {
        centrum_PortX = centrum_PortX / licznik_usrednienia;
        centrum_PortY = centrum_PortY / licznik_usrednienia;
        return true;
    }
    else
        return false;
}
/* konwersja wartości ADC X i Y na przesunięcie kursora
wy: true -ruch kursora */
/*
bool KonwersjaADC_przesuwKursora(int *p_pauza, int *p_wektorX,
                                 int *p_wektorY, int ValueX, int ValueY)
{
#define PAUZA_MS_MAX 50
#define PAUZA_MS_MIN 10
#define CENTRUM_ZERO_ADC 300
    bool przesuw_plus_x, przesuw_plus_y;
    int przesuw_x, przesuw_y;
    double wspolcz;
    // wyznaczanie kierunku ruchu kursora w osi X i Y
    przesuw_x = centrum_PortX - ValueX;
    // odwrócenie kierunku przesuwu w osi X
    if (przesuw_x < 0)
        przesuw_plus_x = true;
    // dla joystika analogowego typu Thumb Slide Joystick
    //- SparkFun COM-09426
    else
        przesuw_plus_x = false;
    przesuw_y = centrum_PortY - ValueY;
    if (przesuw_y < 0)
        przesuw_plus_y = false;
    else
        przesuw_plus_y = true;
    // określenie czy pozycja absolutna przesunięć obu wektorów
    // znajduje się w obszarze centrum
    przesuw_x = abs(przesuw_x);
    przesuw_y = abs(przesuw_y);
    if ((przesuw_x <= CENTRUM_ZERO_ADC) && (przesuw_y <= CENTRUM_ZERO_ADC))
    {
        // pozycja joystika w strefie zerowego ruchu
        *p_pauza = 50;
        *p_wektorX = 0;
        *p_wektorY = 0;
        przesuw_x_ulamek = 0;
        przesuw_y_ulamek = 0;
        return false;
    }
    else
    {
        if (przesuw_x > CENTRUM_ZERO_ADC)
        {
            /* to procedura symulacji dla testów
             * if (przesuw_plus_x==false) *p_wektorX =-1 *2;
             * else *p_wektorX =1 *2;
             */
/*
przesuw_x_ulamek = ObliczPrzesuw(przesuw_x, przesuw_plus_x, przesuw_x_ulamek);
*p_wektorX = przesuw_x_ulamek;
przesuw_x_ulamek -= *p_wektorX;
}
else
*p_wektorX = 0;
if (przesuw_y > CENTRUM_ZERO_ADC)
{
/*
 * to procedura symulacji dla testów
 * if (przesuw_plus_y==false) *p_wektorY =-1 *2;
 * else *p_wektorY =1 *2;
 */
/*
przesuw_y_ulamek = ObliczPrzesuw(przesuw_y, przesuw_plus_y, przesuw_y_ulamek);
*p_wektorY = przesuw_y_ulamek;
przesuw_y_ulamek -= *p_wektorY;
}
else
*p_wektorY = 0;
*p_pauza = 1;
return true;
}
}
/* obliczanie przesuwu w funkcji wychylenia joystika */
/*
double ObliczPrzesuw(int przesuw, bool przesuw_plus, double przesuw_ulamek)
{
    const int delta_stopien_tab[] = {CENTRUM_ZERO_ADC + 300, CENTRUM_ZERO_ADC + 700,
                                     CENTRUM_ZERO_ADC + 800, CENTRUM_ZERO_ADC + 1000};
    const double delta_mnoznik_tab[] = {0.33, 0.66, 1, 1.5, 2.0};
    const char ile_delta = 4;
    char x;
    double ulamek;
    for (x = 0; x < ile_delta; x++)
    {
        if (przesuw < delta_stopien_tab[x])
            break;
    }
    ulamek = delta_mnoznik_tab[x];
    if (przesuw_plus == false)
        ulamek = 0 - ulamek;
    return przesuw_ulamek + ulamek;
}
*/