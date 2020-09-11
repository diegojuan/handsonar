#include <DHT.h>
#include <DHT_U.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

// Sonar a la mano
// Implementacion de un sonar en mano para asistir desplazamientos en condiciones de oscuridad.
// diego@linux.com
// Agosto 29, 2020
//

// Definimos pines
// Sensor derecho
int rTrigger = 10;
int rEcho = 9;

// Sensor izquierdo
int lTrigger = 6;
int lEcho = 5;

// Buzzer 
int buzzer = 4;

// microsegundos por centimetro
float soundSpeed;

// Definimos frecuencia de buzzer derecho e izquierdo
// La diferencia de 100Hz es suficiente para diferenciar el tono.
int fBuzzR = 500;
int fBuzzL = 600;

// Definimos un par de variables que usaremos para calcular la distancia
// Derecho
int travelTimeR, distanceRight;
// Izquierdo
int travelTimeL, distanceLeft;

// Vamos a medir temperatura para ser mas precisos en el calculo de la distancia, no es necesario
// pero es mas cool
int temperature, humidity; 
int tSensor = 2;

// creamos el objeto para el DHT11 (fue lo que consegui)
DHT dht(tSensor, DHT11);


// Definiciones para el display oled
#define OWIDE 128
#define OHIGH 64

// Esta definicion la requiere la libreria pues el OLED de Adafruit la usa, en mi caso uso uun
// generico, pero igual debemos crearla
#define OLED_RESET 4 // punt!

// creamos el objeto
Adafruit_SSD1306 oled(OWIDE, OHIGH, &Wire, OLED_RESET);


// Definimos in / out de los pines
void setup(){
  // Iniciamos bus I2C
  Wire.begin();
  
  // Iniciamos el display en la direccion 0x3c
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Sensor derecho
  pinMode(rTrigger, OUTPUT);
  pinMode(rEcho, INPUT);
  
  // Sensor izquierdo
  pinMode(lTrigger, OUTPUT);
  pinMode(lEcho, INPUT);
  
  // Buzzer 
  pinMode(buzzer, OUTPUT);

  // Temperatura y Humedad
  dht.begin();

}

// loop principal
void loop(){
  // Calculamos la velocidad del sonido para nuestro entorno
  soundSpeed = calculateSoundSpeed();
  
  // Lado derecho
  travelTimeR = travelTime(rTrigger, rEcho);

  // Delay entre lecturas, para prevenir rebotes del echo
  delay(10);
  
  // Lado izquierdo
  travelTimeL = travelTime(lTrigger, lEcho);
  
  // Calculamos la distancia usando la formula d = tiempo / Velocidad del sonido
  // Derecho
  distanceRight = travelTimeR / (soundSpeed * 2);
  beepOrNot(distanceRight, fBuzzR );
  
  // Izquierdo
  distanceLeft = travelTimeL / (soundSpeed *2);
  beepOrNot(distanceLeft, fBuzzL );
  
 // Actualizar display
  showMe();

  // Delay antes de hacer una nueva lectura
  delay(100);
  
}

float calculateSoundSpeed(){
  // Lo primero que hacemos es medir temperatura, ya que los demas calculos de distancia y
  // velocidad del sonido dependen de ello
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Calculamos la velocidad actual del sonido para las condiciones actuales
  // Vs = 331.5 + (temperature * 0.6)
  soundSpeed = 331.5 + (temperature * 0.6);
  soundSpeed = soundSpeed * 100;
  soundSpeed = (float(1) / float(soundSpeed)) * 1000000 ;
  return soundSpeed;
}

int travelTime(int trigger, int echo){
  int travelTime;
  // desactivamos interrupciones momentaneamente para aumentar precision
  noInterrupts();
  // Enviamos senal por 10 micro segundos 
  digitalWrite(trigger, 1);
  delayMicroseconds(10);
  digitalWrite(trigger, 0);
  // Capturamos el echo y lo asignamos el valor a travelTime
  travelTime = pulseIn(echo, 1, 35000);
  // Activamos de nuevos los interrupciones
  interrupts();
  return travelTime;
}

int beepOrNot(int distance, int fBuzz){
    if (distance < 100){
    tone(buzzer, fBuzz);
    delay((distance*100)/20);
    noTone(buzzer);  
  }
}

int showMe(){
    oled.clearDisplay();      // limpiamos el display
    oled.setTextColor(WHITE); // Color por defecto (la pantalla es monocromatica)
    oled.setCursor(30, 0);    // centramos cursor en 30,0
    oled.setTextSize(1);      // establece tamano de texto en 1
    oled.print("Hand sonar"); 
    oled.setCursor (5, 20);   // ubica cursor en coordenas 5,20
    oled.setTextSize(1);      // establece tamano de texto en 2
    oled.print("Left:");    // escribimos el valor de Left
    oled.print(distanceLeft);  
    oled.print("   Right:");    // escribimos el valor de Right
    oled.print(distanceRight);  
    oled.setCursor(5,40);
    oled.print("Temp:"); 
    oled.print(temperature);
    oled.print("  Humidity:");
    oled.print(humidity);
    oled.setCursor(5,50); 
    oled.print("Speed:");
    oled.print(soundSpeed);
    oled.print(" us/cm2");

    // escribimos al display
    oled.display();


    
}
