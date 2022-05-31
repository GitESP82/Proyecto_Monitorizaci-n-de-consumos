//1) Librerías%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include<EthernetClient.h> //Uncomment this library to work with ESP8266
#include<ESP8266WiFi.h>
#include <SD.h>
extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}

//2) Defines%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#define RTCMEMORYSTART 65
#define RTCMEMORYLEN 127


//3) Estructura a almacenar en RTC Memory%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
typedef struct {
  float battery;
//  float other
} rtcStore;

rtcStore rtcMem;


//4) Variables%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Declaración WiFi
WiFiClient client;
//Tooggle Flag
bool toggleFlag=false;
//Buffer
char jsonBuffer[500] = "["; // Initialize the jsonBuffer to hold data
//Wifi

const char* server = "api.thingspeak.com";

int rtcPos;
int buckets=4;

const int CS = D8; // Para la lectura del MicroSD en el NodeMcu


//5) Empezamos%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Start");
  //Leemos el toggleFlag
  system_rtc_mem_read(64, &toggleFlag, 4);
  Serial.print("Toggle Flag actual: ");
  Serial.println(toggleFlag);
  system_rtc_mem_read(65, &rtcPos, 4);
  Serial.print("Posición actual: ");
  Serial.println(rtcPos);
  Serial.print("\n");

//Leemos la toogleFlag para ver si hemos terminado el ciclo o no, y reseteamos el contador    
  if (!toggleFlag) {
    Serial.print("Primera ejecución \n");
    int rtcPos = 66;
    rtcMem.battery = analogRead(A0)/1023*3;
    system_rtc_mem_write(rtcPos, &rtcMem, 4);
    toggleFlag = true;
    system_rtc_mem_write(64, &toggleFlag, 4);
    rtcPos++;
    system_rtc_mem_write(65, &rtcPos, 4);

    Serial.print("Battery level: ");
    Serial.println(rtcMem.battery);
    Serial.print("Next position: ");
    Serial.println(rtcPos);
    yield();
    ESP.deepSleep(5000000);
  }

//Bucle para leer y almacenar valores
  else {
    Serial.print("Segunda condición\n");
    //Por si rtcPos toma valores fuera de los marcados
    if (rtcPos < 67 || rtcPos > 80){
    rtcPos = 67;
    system_rtc_mem_write(65, &rtcPos, 4);
    Serial.print("Posición corregida");
    }
    //Leemos y almacenamos
    rtcMem.battery = analogRead(A0);
    system_rtc_mem_write(rtcPos, &rtcMem, 4);
    Serial.print("Battery level: ");
    Serial.println(rtcMem.battery);
    system_rtc_mem_read(rtcPos-1, &rtcMem, 4);
    Serial.print("Anterior battery level: ");
    Serial.println(rtcMem.battery);
    rtcPos++;
    system_rtc_mem_write(65, &rtcPos, 4);
    Serial.print("Next position: ");
    Serial.println(rtcPos);
    yield();

    //Fin del paquete
    if (rtcPos == 80){
    toggleFlag = false;
    system_rtc_mem_write(64, &toggleFlag, 4);
    Serial.print("Nueva toggleFlag declarada: ");
    Serial.println(toggleFlag);
    Serial.print("Vamos a meter los datos de la memoria a un solo paquete\n");

    char jsonBuffer[500] = "["; // Initialize the jsonBuffer to hold data

    //Transpaso de datos de RTC Memory a la flash%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    updatesJson(jsonBuffer);

    //Leemos los datos de la MicroSD%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //Declaramos los strings donde vamos a volcar los datos de la sd
    
    String ssid_sdcard = ""; // Texto de la SdCard.
    String password_sdcard = ""; // Texto de la SdCard.
    String channel_sdcard = ""; // Texto de la SdCard.
    String wapikey_sdcard = ""; // Texto de la SdCard.

    const char* archivo_ssid = "ssid.txt";
    const char* archivo_password = "password.txt";
    const char* archivo_channel = "channel.txt";
    const char* archivo_wapikey = "wapikey.txt";

    Serial.println("Iniciando SdCard...");
     if (!SD.begin(CS)) {
        Serial.println("Error al iniciar.");
        return;
    }
    Serial.println("SdCard iniciada.");

    //Traspasamos el dato "Red Wifi"
    
    File miarchivoA = SD.open("ssid.txt");
    if (miarchivoA) {
      while (miarchivoA.available()) {
        //Serial.write(miarchivoA.read());
        ssid_sdcard = ssid_sdcard + miarchivoA.readString();
        delay(100);
      }
      miarchivoA.close();
      Serial.println("Leido el archivoA de la SdCard.");
    }
    else {
      Serial.println("Error al abrir el archivoA.");
    }
    Serial.println(ssid_sdcard);

    
    //Traspasamos el dato "Contraseña Wifi"
    
    File miarchivoB = SD.open("password.txt");
    if (miarchivoB) {
      while (miarchivoB.available()) {
        //Serial.write(miarchivoB.read());
        password_sdcard = password_sdcard + miarchivoB.readString();
        delay(100);
      }
      miarchivoB.close();
      Serial.println("Leido el archivoB de la SdCard.");
    }
    else {
      Serial.println("Error al abrir el archivoB.");
    }
    Serial.println(password_sdcard);    


    //Traspasamos el dato "Channel"
    
    File miarchivoC = SD.open("channel.txt");
    if (miarchivoC) {
      while (miarchivoC.available()) {
        //Serial.write(miarchivoC.read());
        channel_sdcard = channel_sdcard + miarchivoC.readString();
        delay(100);
      }
      miarchivoC.close();
      Serial.println("Leido el archivoC de la SdCard.");
    }
    else {
      Serial.println("Error al abrir el archivoC.");
    }
    Serial.println(channel_sdcard);   


    //Traspasamos el dato "Write API Key"
    
    File miarchivoD = SD.open("wapikey.txt");
    if (miarchivoD) {
      while (miarchivoD.available()) {
        //Serial.write(miarchivoD.read());
        wapikey_sdcard = wapikey_sdcard + miarchivoD.readString();
        delay(100);
      }
      miarchivoD.close();
      Serial.println("Leido el archivoD de la SdCard.");
    }
    else {
      Serial.println("Error al abrir el archivoD.");
    }
    Serial.println(wapikey_sdcard); 


    //Inicializamos la librería WiFi %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    Serial.print("Vamos a encender la WiFi\n");
          //WiFiClient client;
        WiFi.begin(ssid_sdcard, password_sdcard);
          Serial.println();
          Serial.println();
          Serial.print("Connecting to ");
          Serial.println(ssid_sdcard);
        WiFi.begin(ssid_sdcard, password_sdcard);
        while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        }
      Serial.println("");
      Serial.println("WiFi connected");
      
    //Mandamos los datos a ThingSpeak%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  

    httpRequest(jsonBuffer);
    
    Serial.print("\nEnviado\n");
    
    Serial.print("\nMe voy a dormir\n");
    }
    ESP.deepSleep(5000000,WAKE_RFCAL);
  }

}

//Void para cargar los datos de la memoria en el buffer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void updatesJson(char* jsonBuffer){
  /* JSON format for updates paramter in the API
   *  This examples uses the relative timestamp as it uses the "delta_t". You can also provide the absolute timestamp using the "created_at" parameter
   *  instead of "delta_t".
   *   "[{\"delta_t\":0,\"field1\":-70},{\"delta_t\":3,\"field1\":-66}]"
   */
   //Primero, inicializamos la SD y leemos el dato del campo en el que queremos escribir
       //Declaramos los strings donde vamos a volcar los datos de la sd
    
    String field_sdcard = ""; // Texto de la SdCard.

    const char* archivo_field = "field.txt";

    Serial.println("Iniciando SdCard...");
     if (!SD.begin(CS)) {
        Serial.println("Error al iniciar.");
        return;
    }
    Serial.println("SdCard iniciada.");

    //Traspasamos el dato "Field"
    
    File miarchivoE = SD.open("field.txt");
    if (miarchivoE) {
      while (miarchivoE.available()) {
        //Serial.write(miarchivoE.read());
        field_sdcard = field_sdcard + miarchivoE.readString();
        delay(100);
      }
      miarchivoE.close();
      Serial.println("Leido el archivoE de la SdCard.");
    }
    else {
      Serial.println("Error al abrir el archivoE.");
    }
    Serial.println(field_sdcard);

// %%%%%%%% PREPARAMOS EL DATO DE CAMPO DONDE ESCRIBIR%%%%%%%%%%%%%%
    char field_char[20];
    size_t lengthZ = String(field_sdcard).length();
    field_sdcard.toCharArray(field_char,lengthZ+1);
    
  // Format the jsonBuffer as noted above
  for (int i = 66; i < 81; i++){
    strcat(jsonBuffer,"{\"delta_t\":");
    unsigned long deltaT = 5; //Tomamos datos cada 5 seg
    size_t lengthT = String(deltaT).length();
    char temp[4];
    String(deltaT).toCharArray(temp,lengthT+1);
    strcat(jsonBuffer,temp);
    strcat(jsonBuffer,",");

    //Aquí leemos lo que tenemos en RTC Memory
    system_rtc_mem_read(i, &rtcMem, 4);
    float lectura = rtcMem.battery; 
    strcat(jsonBuffer,"\"field");
    strcat(jsonBuffer,field_char);
    strcat(jsonBuffer,"\":");


    lengthT = String(lectura).length();
    String(lectura).toCharArray(temp,lengthT+1);
    strcat(jsonBuffer,temp);
    strcat(jsonBuffer,"},");
    Serial.println("este es el dato de la memoria");
    Serial.println(rtcMem.battery);
  }
    // If posting interval time has reached 2 minutes, update the ThingSpeak channel with your data
    // Después de sacar todos los datos, enviamos
    
  size_t len = strlen(jsonBuffer);
  jsonBuffer[len-1] = ']';
  //httpRequest(jsonBuffer);
  Serial.println(jsonBuffer);    
   // lastUpdateTime = millis(); // Update the last update time
  Serial.println("\nProceso transpaso memorya RTC a flash para envío, completado\n");
  
}

//9) Define the httpRequest method to send data to ThingSpeak and to print the response code from the server. A response code 202 indicates that the server has accepted the request for precessing.
// Updates the ThingSpeakchannel with data
void httpRequest(char* jsonBuffer) {
  /* JSON format for data buffer in the API
   *  This examples uses the relative timestamp as it uses the "delta_t". You can also provide the absolute timestamp using the "created_at" parameter
   *  instead of "delta_t".
   *   "{\"write_api_key\":\"YOUR-CHANNEL-WRITEAPIKEY\",\"updates\":[{\"delta_t\":0,\"field1\":-60},{\"delta_t\":15,\"field1\":200},{\"delta_t\":15,\"field1\":-66}]
   */
  // Format the data buffer as noted above

    String channel_sdcard = ""; // Texto de la SdCard.
    String wapikey_sdcard = ""; // Texto de la SdCard.
    
    //Traspasamos el dato "Channel"
    
    File miarchivoC = SD.open("channel.txt");
    if (miarchivoC) {
      while (miarchivoC.available()) {
        //Serial.write(miarchivoC.read());
        channel_sdcard = channel_sdcard + miarchivoC.readString();
        delay(100);
      }
      miarchivoC.close();
      Serial.println("Leido el archivoC de la SdCard.");
    }
    else {
      Serial.println("Error al abrir el archivoC.");
    }
    Serial.println(channel_sdcard);  

    //Traspasamos el dato "Write API Key"
    
    File miarchivoD = SD.open("wapikey.txt");
    if (miarchivoD) {
      while (miarchivoD.available()) {
        //Serial.write(miarchivoD.read());
        wapikey_sdcard = wapikey_sdcard + miarchivoD.readString();
        delay(100);
      }
      miarchivoD.close();
      Serial.println("Leido el archivoD de la SdCard.");
    }
    else {
      Serial.println("Error al abrir el archivoD.");
    }
    Serial.println(wapikey_sdcard); 
  
  Serial.println("estoy aquí");
  char data[500] = "{\"write_api_key\":\"";
  size_t lengthS = String(wapikey_sdcard).length();
  char wapikey_char[20];
  wapikey_sdcard.toCharArray(wapikey_char,lengthS+1);
  strcat(data,wapikey_char); //Asignar la variable wapikey
  strcat(data,"\",\"updates\":"); // Replace YOUR-CHANNEL-WRITEAPIKEY with your ThingSpeak channel write API key
  strcat(data,jsonBuffer);
  strcat(data,"}");
  Serial.println(data);

  
  char dataChannel[500] = "POST /channels/";
  //strcat(dataChannel,"POST /channels/");
  size_t lengthU = String(channel_sdcard).length();
  char channel_char[20];
  String(channel_sdcard).toCharArray(channel_char,lengthU+1);
  strcat(dataChannel,channel_char);
  strcat(dataChannel,"/bulk_update.json HTTP/1.1");
  Serial.println(dataChannel);
  client.println(dataChannel);
  
  // Close any connection before sending a new request
  client.stop();
  String data_length = String(strlen(data)+1); //Compute the data buffer length
  Serial.println(data);
  
  // POST data to ThingSpeak
  if (client.connect(server, 80)) {
    client.println(dataChannel); // Replace YOUR-CHANNEL-ID with your ThingSpeak channel ID
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: mw.doc.bulk-update (Arduino ESP8266)");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.println("Content-Length: "+data_length);
    client.println();
    client.println(data);
  }
  else {
    Serial.println("Failure: Failed to connect to ThingSpeak");
  }
  delay(250); //Wait to receive the response
  client.parseFloat();
  String resp = String(client.parseInt());
  Serial.println("Response code:"+resp); // Print the response code. 202 indicates that the server has accepted the response
  jsonBuffer[0] = '['; //Reinitialize the jsonBuffer for next batch of data
  jsonBuffer[1] = '\0';
  //lastConnectionTime = millis(); //Update the last conenction time
}


void loop() {
}
