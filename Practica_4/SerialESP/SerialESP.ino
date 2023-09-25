#include "WiFi.h"
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Define specific pins for Serial2.
#define RXD2 33
#define TXD2 4

#define AIO_SERVER      "garceta.tsc.urjc.es"
#define AIO_SERVERPORT  21883

const char* ssid = "sensoresurjc";
const char* password = "Goox0sie_WZCGGh25680000";
char topic[] = "/SETR/2022/8/";
long time_start = 0;
long time_end = 0;
long time_ping = 0;
int counter = 1;
bool end_lap = false;
bool init_search = false;

char* json_obstacle = "{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"OBSTACLE_DETECTED\" }";
char* json_lostline = "{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"LINE_LOST\" }";
char* json_start = "{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"START_LAP\" }";
char* json_linefound = "{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"LINE_FOUND\" }";
char* json_start_search = "{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"INIT_LINE_SEARCH\" }";
char* json_end_search = "{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"STOP_LINE_SEARCH\" }";
char json_end[512];
char json_ping[512];

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT,"","");
Adafruit_MQTT_Publish msg = Adafruit_MQTT_Publish(&mqtt, topic);

// Se realiza la conexion MWTT
void MQTT_connect() {
  int8_t ret;

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }

  Serial.println("MQTT Connected!");
}

// Se realiza la conexion WiFi
void initWiFi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  
  MQTT_connect();

  // WiFi y MQTT conectados, publicar en topic y enviar mensaje al arduino para empezar el circuito
  Serial2.println("CONNECTED");

  msg.publish(json_start);

  time_start = millis();

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void setup() {

  // Regular serial connection to show traces for debug porpuses
  Serial.begin(9600);

  // Serial port para comunicar con Arduino UNO
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  initWiFi();
}

void loop() {
  
  
  if (Serial2.available() && !end_lap) {

    char c = Serial2.read();
    if (c == 'O')  {
      msg.publish(json_obstacle);
      if (!end_lap) {
        time_end = millis() - time_start;
        sprintf(json_end, "{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"END_LAP\", \"time\" : %ld }", time_end);
        msg.publish(json_end);
        end_lap = true;
      }
    } else if (c == 'L') {
      if (!init_search) {
        msg.publish(json_lostline);
        msg.publish(json_start_search);
        init_search = true;
      }
    } else if ( c == 'F') {
      if (init_search) {
        msg.publish(json_linefound);
        msg.publish(json_end_search);
        init_search = false;
      }
    }
  }
  time_ping = millis() - time_start;
  if (time_ping >= 4000*counter && !end_lap) {
    counter++;
    sprintf(json_ping,"{ \"team_name\" : \"DuQui\", \"id\" : \"8\", \"action\" : \"PING\", \"time\" : %ld }",time_ping);
    msg.publish(json_ping);
  }
}