#include <ESP8266WiFi.h>
//#include <WiFiClientSecure.h>
#include "PubSubClient.h"
#include "sha256.h"
#include "Base64.h"
#include "DHT.h"

#include <ArduinoJson.h>

#define DHTTYPE DHT11
#define DHTPIN 5
DHT dht(DHTPIN, DHTTYPE);

//IPAddress timeServer(203, 56, 27, 253); // NTP Server au.pool.ntp.org

const char* ssid = "PrettyFlyForaWifi";
const char* password = "tintenfische1";


const char* iothostname = "uneidelIOT.azure-devices.net";
char* key = "<deviceKey>";
const char* IOT_HUB_END_POINT = "/messages/events"; ///messages/events?api-version=2015-08-15-preview

int expire = 1511104241;
char buffer[256];
//WiFiClient espClient;
WiFiClientSecure espClient;
PubSubClient client(espClient);
long lastMsg = 0;

int value = 0;



void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("***************************************************Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
                      // but actually the LED is on; this is because
                      // it is acive low on the ESP-01)
  }
  else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if (('a' <= *msg && *msg <= 'z')
      || ('A' <= *msg && *msg <= 'Z')
      || ('0' <= *msg && *msg <= '9')) {
      encodedMsg += *msg;
    }
    else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}
const char *GetStringValue(String value){
  int len = value.length() + 1;
  char *temp = new char[len];
  value.toCharArray(temp, len);
  return temp;
}

String createIotHubSas(char *key, String url) {
  String stringToSign = url + "\n" + expire;


  int keyLength = strlen(key);

  int decodedKeyLength = base64_dec_len(key, keyLength);
  char decodedKey[decodedKeyLength];  //allocate char array big enough for the base64 decoded key

  base64_decode(decodedKey, key, keyLength);  //decode key

  Sha256.initHmac((const uint8_t*)decodedKey, decodedKeyLength);
  Sha256.print(stringToSign);
  char* sign = (char*)Sha256.resultHmac();
  // END: Create signature

  // START: Get base64 of signature
 int encodedSignLen = base64_enc_len(HASH_LENGTH);
  char encodedSign[encodedSignLen];
 base64_encode(encodedSign, sign, HASH_LENGTH);

  // SharedAccessSignature
  return "sig=" + URLEncode(encodedSign) + "&se=" + expire + "&sr=" + url;
//  // END: create SAS  
}

String GeneratePassword() {
/*
  * SharedAccessSignature sig={signature-string}&se={expiry}&skn={policyName}&sr={URL-encoded-resourceURI}
  * Value   Description
  {signature}   An HMAC-SHA256 signature string of the form: {URL-encoded-resourceURI} + "\n" + expiry. Important: The key is decoded from base64 and used as key to perform the HMAC-SHA256 computation.
  {resourceURI}   URI prefix (by segment) of the endpoints that can be accessed with this token. For example, /events
  {expiry}  UTF8 strings for number of seconds since the epoch 00:00:00 UTC on 1 January 1970.
  {URL-encoded-resourceURI}   Lower case URL-encoding of the lower case resource URI
  {policyName}  The name of the shared access policy to which this token refers. Absent in the case of tokens referring to device-registry credentials.
  */
  //String url =  URLEncode(iothostname) + URLEncode(IOT_HUB_END_POINT);
  String url = URLEncode("uneidelIOT.azure-devices.net/messages/events");
  String fullSas = createIotHubSas(key, url);
  String FullSas = "SharedAccessSignature ";
  FullSas += fullSas;

  return FullSas;
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  Serial.print("Connection to Host: ");
  Serial.println(iothostname);
  Serial.print("Port: ");
  Serial.println(8883);
  client.setServer(iothostname, 8883);
  client.setCallback(callback);
}


String GetJson()
{
  Serial.println("GetJson");
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "SampleSensor";
  root["time"] = 1351824120;
  root["temp"] = dht.readTemperature();
  root["humi"] = dht.readHumidity();
  root.printTo(Serial);
  root.printTo(buffer, sizeof(buffer));

  return (String)buffer; 
}
void reconnect() {
   
  String Username = "uneidelIOT.azure-devices.net/SampleSensor";
  // Loop until we're reconnected
  String Password = GeneratePassword();
  //String Password = "SharedAccessSignature sr=uneidelIOT.azure-devices.net%2fdevices%2fSampleSensor&sig=sDT9756Fc3YPZlnlQpoJKNWBz58vjXqBLRKcoaYGToI%3d&se=1456752091";
  while (!client.connected()) {
    Serial.println(client.state());
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    Serial.print("ClientId : ");
    Serial.println("SampleSensor");
    Serial.print("UserName: ");
    Serial.println(GetStringValue(Username));
    Serial.print("Password: ");
    Serial.println(GetStringValue(Password));

    if (client.connect("SampleSensor", GetStringValue(Username), GetStringValue(Password))) {
      //   if (client.connect("SampleSensor")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("events", GetStringValue(GetJson()));
      // ... and resubscribe
      client.subscribe("devicebound");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(4000);
  client.publish("events", GetStringValue(GetJson()));
    
  
}


