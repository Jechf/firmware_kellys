/* Segun la documentación para la estructura y nomenclatura:
  - Ubicación: Bunkers code: bunker_2
        variables: 
            Temperatura del Aire        code: bunker_2_temp     topic: kellys/variables/bunker_2_temp
            Humedad Relativa del Aire   code: bunker_2_hum      topic: kellys/variables/bunker_2_hum
            Sonda de temperatura 1      code: bunker_2_sonda_1  topic: kellys/variables/bunker_2_sonda_1
            Sonda de temperatura 2      code: bunker_2_sonda_2  topic: kellys/variables/bunker_2_sonda_2
            Sonda de temperatura 3      code: bunker_2_sonda_3  topic: kellys/variables/bunker_2_sonda_3
            Sonda de temperatura 4      code: bunker_2_sonda_4  topic: kellys/variables/bunker_2_sonda_4

*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_AHTX0.h>

const char* ssid = "Doen ";
const char* password = "Clave12345";

const char* mqtt_server = "192.168.0.227"; // Cambia por la dirección de tu servidor MQTT
const int mqtt_port = 1883; // Puerto MQTT

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_AHTX0 aht;

#define PIN_SONDA_1 36
#define PIN_SONDA_2 39
#define PIN_SONDA_3 34
#define PIN_SONDA_4 35

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conexión WiFi establecida");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Función de callback para manejar las suscripciones MQTT, si es necesario
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando nuevamente en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  if (!aht.begin()) {
    Serial.println("No se pudo encontrar el sensor AHT21. Verifica la conexión.");
    while (1) delay(10);
  }
  Serial.println("Sensor AHT21 encontrado");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  // Publicar temperatura y humedad en MQTT
  client.publish("kellys/variables/bunker_2_temp", String(temp.temperature).c_str(), true);
  client.publish("kellys/variables/bunker_2_hum", String(humidity.relative_humidity).c_str(), true);

  // Leer temperaturas de las sondas analógicas
  float promedioSonda1 = 0.0;
  float promedioSonda2 = 0.0;
  float promedioSonda3 = 0.0;
  float promedioSonda4 = 0.0;

  for (int i = 0; i < 20; i++) {
    promedioSonda1 += analogRead(PIN_SONDA_1);
    promedioSonda2 += analogRead(PIN_SONDA_2);
    promedioSonda3 += analogRead(PIN_SONDA_3);
    promedioSonda4 += analogRead(PIN_SONDA_4);
    delay(10);
  }

  promedioSonda1 /= 20.0;
  promedioSonda2 /= 20.0;
  promedioSonda3 /= 20.0;
  promedioSonda4 /= 20.0;

  // Convertir a grados Celsius, Factor experimental
  promedioSonda1 = promedioSonda1 * 0.0283 + 0.7632;
  promedioSonda2 = promedioSonda2 * 0.0283 + 0.7632;
  promedioSonda3 = promedioSonda3 * 0.0283 + 0.7632;
  promedioSonda4 = promedioSonda4 * 0.0283 + 0.7632;

  // Publicar temperaturas promedio en MQTT
  client.publish("kellys/variables/bunker_2_sonda_1", String(promedioSonda1).c_str(), true);
  client.publish("kellys/variables/bunker_2_sonda_2", String(promedioSonda2).c_str(), true);
  client.publish("kellys/variables/bunker_2_sonda_3", String(promedioSonda3).c_str(), true);
  client.publish("kellys/variables/bunker_2_sonda_4", String(promedioSonda4).c_str(), true);

  delay(5000); // Esperar 5 segundos antes de volver a leer y publicar
}
