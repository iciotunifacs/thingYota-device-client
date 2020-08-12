#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "GVT-55F1";
const char* password = "1907284039";

const char* mqtt_server = "broker.hivemq.com";
int broker_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() 
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Menssagem Recebida!! [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  if ((char)payload[0] == '1') 
  {
    digitalWrite(BUILTIN_LED, LOW); 
  } else 
  {
    digitalWrite(BUILTIN_LED, HIGH);
  }

}

void create();
void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      String macESP = (String)WiFi.macAddress();
      client.subscribe("inTopic");
      create();
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

#define sensorN1 4  //pino d2
#define sensorN2 2  //pino d4
#define sensorN3 14 //pino d5
#define rele 13     //pino d7
#define sensorF 12   //pino d6


int contaPulso = 0; 
void ICACHE_RAM_ATTR funcao()
{
  contaPulso++;
}

void upload(String pnome, float valorF, bool valorB);
void setup() 
{
  //---------------------------------------
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, broker_port);
  client.setCallback(callback);
  //---------------------------------------

  pinMode(sensorN1, INPUT);
  pinMode(sensorN2, INPUT);
  pinMode(sensorN3, INPUT);
  pinMode(sensorF, INPUT);
  pinMode(rele, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(sensorF), funcao, RISING);

}

double Litros, total, aux;
double vazao; 
double media=0; 
int i=0;
int chaveN1 = 0, chaveN2 = 0, chaveN3 = 0;
void loop() 
{
  //---------------------------------------
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  //---------------------------------------
  digitalWrite(rele, 1); 
  if(sensorN2 == 0)
  {
    while(sensorN1 == 1 && sensorN3 == 0)
    {
      digitalWrite(rele, 0);
      upload("Bomba", 0, true);  
    }

    upload("Bomba", 0, false);
  }

  
  if(digitalRead(sensorN1) != chaveN1)
  {
    chaveN1 = digitalRead(sensorN1);
    if(chaveN1 == 1)
    {
      upload("sensorN1", 0, true);
    }
    else
    {
      upload("sensorN1", 0, false);
    }
    Serial.print("Mandou: SensorN1 - ");
    Serial.println(chaveN1);
  }

  if(digitalRead(sensorN2) != chaveN2)
  {
    chaveN2 = digitalRead(sensorN2);
    if(chaveN2 == 1)
    {
      upload("sensorN2", 0, true);
    }
    else
    {
      upload("sensorN2", 0, false);
    }
    Serial.print("Mandou: SensorN2 - ");
    Serial.println(chaveN2);
  }

  if(digitalRead(sensorN3) != chaveN3)
  {
    chaveN3 = digitalRead(sensorN3);
    if(chaveN3 == 1)
    {
      upload("sensorN3", 0, true);
    }
    else
    {
      upload("sensorN3", 0, false);
    }
    Serial.print("Mandou: SensorN3 - ");
    Serial.println(chaveN3);
  }
  //---------------------------------------
    sei();      
    delay (1000); 
    cli(); 
    
    i++;

    vazao = (double)contaPulso/5.5;
    
    Serial.print(contaPulso); 
    Serial.print(" N° de Pulsos - ");
    Serial.print(i); 
    Serial.println("s");
 
    Serial.print(" Vazão: ");
    Serial.print(vazao);
    Serial.println(" - Litros/S");
    
    if(i==60)
    {
      i = 0;
      total = (double)contaPulso/345;
      Serial.println("---------------------------------------------");
      Serial.print("Enchimento em ");
      Serial.print(total);
      Serial.println(" L");
      contaPulso = 0;
      upload("sensorF - Enchimento", total, true);
    }

    upload("sensor Fluxo - Vazao", vazao, true);
}

void create()
{
  String macESP = (String)WiFi.macAddress();
  char json[300];
  
  String createDevice = "{\"to\":\"Device\",\"from\":\"Device\",\"event\":\"CREATE_DEVICE\",\"name\":\"device - 01\",\"entity\":\"Device\",\"mac_addres\": \""+macESP+"\"}";
  String createSN1 = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorN1+"\",\"type\":\"water-sensor\",\"name\": \"Sensor Nivel 1\"},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_CREATE\"}";
  String createSN2 = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorN2+"\",\"type\":\"water-sensor\",\"name\": \"Sensor Nivel 2\"},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_CREATE\"}";  
  String createSN3 = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorN3+"\",\"type\":\"water-sensor\",\"name\": \"Sensor Nivel 3\"},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_CREATE\"}";  
  String createSF = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorF+"\",\"type\":\"pression-sensor\",\"name\":\"Sensor Fluxo\"},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_CREATE\"}";
  String createRele = "{\"mac_addres\":\""+macESP+"\",\"Actor\":{\"port\":\""+rele+"\",\"type\":\"engine\",\"name\":\"Bomba\"},\"to\":\"Actor\",\"from\":\"Device\",\"event\":\"ACTOR_CREATE\"}";
  
  createDevice.toCharArray(json, 300);
  client.publish("server", json);
  
  createSN1.toCharArray(json, 300);
  client.publish("server", json);

  createSN2.toCharArray(json, 300);
  client.publish("server", json);

  createSN3.toCharArray(json, 300);
  client.publish("server", json);

  createSF.toCharArray(json, 300);
  client.publish("server", json);

  createRele.toCharArray(json, 300);
  client.publish("server", json);
}

void upload(String pnome, float valorF, bool valorB)
{
  char json[600];
  String texto;
  String macESP = (String)WiFi.macAddress();
  int op = 0;
  if(pnome == "sensorN1")
  {
     texto = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorN1+"\",\"type\":\"wather-sensor\",\"name\":\"Sensor Nivel 1\",\"value\":{\"data\": "+valorB+",\"entity\":\"boolean\"}},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_UPDATE\"}";
     texto.toCharArray(json, 600);
     client.publish("server", json);
  }
  else if(pnome == "sensorN2")
  {
     texto = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorN2+"\",\"type\":\"wather-sensor\",\"name\":\"Sensor Nivel 1\",\"value\":{\"data\": "+valorB+",\"entity\":\"boolean\"}},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_UPDATE\"}";
     texto.toCharArray(json, 600);
     client.publish("server", json);
  }
  else if(pnome == "sensorN3")
  {
      texto = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorN3+"\",\"type\":\"wather-sensor\",\"name\":\"Sensor Nivel 1\",\"value\":{\"data\": "+valorB+",\"entity\":\"boolean\"}},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_UPDATE\"}";
      texto.toCharArray(json, 600);
      client.publish("server", json);
  }
  else if(pnome == "sensorF - Enchimento")
  {
      texto = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorF+"\",\"type\":\"pression-sensor\",\"name\":\"Sensor Fluxo\",\"value\":{\"data\":"+valorF+",\"entity\":\"number\",\"unity\":\"L/min\"}},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_UPDATE\"}";
      texto.toCharArray(json, 600);
      client.publish("server", json);
  }
  else if(pnome == "sensorF - Vazao")
  {
      texto = "{\"mac_addres\":\""+macESP+"\",\"Sensor\":{\"port\":\""+sensorF+"\",\"type\":\"pression-sensor\",\"name\":\"Sensor Fluxo\",\"value\":{\"data\":"+valorF+",\"entity\":\"number\",\"unity\":\"L/min\"}},\"to\":\"Sensor\",\"from\":\"Device\",\"event\":\"SENSOR_UPDATE\"}";
      texto.toCharArray(json, 600);
      client.publish("server", json);
  }
  else if(pnome == "Bomba")
  {
      texto = "{\"mac_addres\":\""+macESP+"\",\"Actor\":{\"name\":\"Bomba\",\"port\":\""+rele+"\",\"type\":\"motor\",\"value\":{\"data\":"+valorB+",\"entity\":\"boolean\"}},\"from\":\"Device\",\"to\":\"Actor\",\"event\":\"ACTOR_UPDATE\"}";
      texto.toCharArray(json, 600);
      client.publish("server", json);
  }

}
