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
#define BUTTON_BUILTIN 0
#define LED_BUILTIN 2
#define DATO_SSID 100
#define DATO_G_SSID 119
#define DATO_PASSWORD 120
#define DATO_G_PASSWORD 139
#define DATO_CHANNEL 144
#define DATO_G_CHANNEL 148
#define DATO_WAPIKEY 150
#define DATO_G_WAPIKEY 159
#define DATO_FIELD 160
#define DATO_G_FIELD 161

//3) Estructuras para almacenar en RTC Memory%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
typedef struct {
  float battery;
//  float other
} rtcStore;

rtcStore rtcMem;

typedef struct rtcDatosSD{
  char dato[4];
//  float other
} rtcDatosSD;

rtcDatosSD rtcSD;

typedef struct rtcLongitud{
  int longitud;
//  float other
} rtcLongitud;

rtcLongitud rtcLong;


//4) Variables%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Declaración WiFi
WiFiClient client;

//Tooggle Flag
bool toggleFlag=false;

//Buffer
char jsonBuffer[500] = "["; // Initialize the jsonBuffer to hold data

//Servidor ThingSpeak
const char* server = "api.thingspeak.com";

int rtcPos;
int buckets=4;

const int CS = D8; // Para la lectura del MicroSD en el NodeMcu


//5) Empezamos%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Start");

 /* //Configuración de pin de alimentación de la SDCard y del LED de chequeo
  pinMode(D2, OUTPUT);
 // pinMode(BUTTON_BUILTIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(3000);*/

  //Comprobamos si queremos cargar datos de la SDCard con el boton Flash

  //int estado = digitalRead(BUTTON_BUILTIN); 

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%   SI PUEDE LEER LA SD, GUARDA SUS DATOS, SI NO, SALTA A LA LECTURA DE CONSUMO   %%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


if(SD.begin(CS)){

//%%%%%%%%%%%%%DECLARAMOS LAS VARIABLES%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
    String ssid_sdcard = ""; // Texto de la SdCard.
    String password_sdcard = ""; // Texto de la SdCard.
    String channel_sdcard = ""; // Texto de la SdCard.
    String wapikey_sdcard = ""; // Texto de la SdCard.
    String field_sdcard = ""; //Texto de la SdCard

    const char* archivo_ssid = "ssid.txt";
    const char* archivo_password = "password.txt";
    const char* archivo_channel = "channel.txt";
    const char* archivo_wapikey = "wapikey.txt";
    const char* archivo_field = "field.txt";

//%%%%%%%%%%%%%COMPROBAMOS QUE SE HA INICIADO LA SDCARD%%%%%%%%%%%%%%%%%%%
    Serial.println("Iniciando SdCard...");
    
    if (!SD.begin(CS)) {
        Serial.println("Error al iniciar.");
        return;
    }
    
    Serial.println("SdCard iniciada.");   

    //Traspasamos el dato "Red Wifi"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
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

    
    //Traspasamos el dato "Contraseña Wifi"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
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


    //Traspasamos el dato "Channel"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
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


    //Traspasamos el dato "Write API Key"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
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


    //Traspasamos el dato "Field"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
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

    //Datos leídos   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

//%%%%%%%%%%%%     GUARDAMOS EL DATO DE LONGITUD DEL DATO PRINCIPAL          %%%%%%%

//&&&&&&&&&&&&       Medimos la longitud del dato       &&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
    size_t lengthA = String(ssid_sdcard).length();
    size_t lengthB = String(password_sdcard).length();
    size_t lengthC = String(channel_sdcard).length();
    size_t lengthD = String(wapikey_sdcard).length();
    size_t lengthE = String(field_sdcard).length();

    Serial.println("\nTamaño datos: ");
    Serial.println(lengthA);
    Serial.println(lengthB);
    Serial.println(lengthC);
    Serial.println(lengthD);
    Serial.println(lengthE);
    Serial.println("\n");

        
    char ssid_char[lengthA];
    char password_char[lengthB];
    char channel_char[lengthC];
    char wapikey_char[lengthD];
    char field_char[lengthE];


    int grupo_ssid=(lengthA/4)+1;
    int grupo_password=(lengthB/4)+1;
    int grupo_channel=(lengthC/4)+1;
    int grupo_wapikey=(lengthD/4);
    int grupo_field=(lengthE/4)+1;

    Serial.println("Numero de grupos necesarios para cada dato: ");
    Serial.println(grupo_ssid);
    Serial.println(grupo_password);
    Serial.println(grupo_channel);
    Serial.println(grupo_wapikey);
    Serial.println(grupo_field);
    Serial.println("\n");

    
    ssid_sdcard.toCharArray(ssid_char,lengthA+1);
    password_sdcard.toCharArray(password_char,lengthB+1);
    wapikey_sdcard.toCharArray(wapikey_char,lengthD+1);
    channel_sdcard.toCharArray(channel_char,lengthC+1);
    field_sdcard.toCharArray(field_char,lengthE+1);

    
    int G_SSID = DATO_G_SSID;
    int G_PASSWORD = DATO_G_PASSWORD;
    int G_CHANNEL = DATO_G_CHANNEL;
    int G_WAPIKEY = DATO_G_WAPIKEY;
    int G_FIELD = DATO_G_FIELD;

    
    rtcLong.longitud = lengthA;
    system_rtc_mem_write(G_SSID, &rtcLong, 4);

    rtcLong.longitud = lengthB;
    system_rtc_mem_write(G_PASSWORD, &rtcLong, 4);

    rtcLong.longitud = lengthC;
    system_rtc_mem_write(G_CHANNEL, &rtcLong, 4);

    rtcLong.longitud = lengthD;
    system_rtc_mem_write(G_WAPIKEY, &rtcLong, 4);

    rtcLong.longitud = lengthE;
    system_rtc_mem_write(G_FIELD, &rtcLong, 4);

   

//%%%%%%%%%%%%%   Bucles para guardar los datos en los Arrays y en la memoria RTC   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


int PosDataA=DATO_SSID;
int k=0;  
for(int i=0; i<grupo_ssid; i++){

    char ssidRTC[4];

    for(int j=0; j<4; j++){
      ssidRTC[j]=ssid_char[k];
      k++;
      }
      memcpy(rtcSD.dato, ssidRTC, 4);
      system_rtc_mem_write(PosDataA, &rtcSD, 4);
      PosDataA++;
} 

int PosDataB=DATO_PASSWORD;
k=0;  
for(int i=0; i<grupo_password; i++){

    char passwordRTC[4];

    for(int j=0; j<4; j++){
      passwordRTC[j]=password_char[k];
      k++;
      }
      memcpy(rtcSD.dato, passwordRTC, 4);
      system_rtc_mem_write(PosDataB, &rtcSD, 4);
      PosDataB++;
} 

int PosDataC=DATO_CHANNEL;
k=0;  
for(int i=0; i<grupo_channel; i++){

    char channelRTC[4];

    for(int j=0; j<4; j++){
      channelRTC[j]=channel_char[k];
      k++;
      //Serial.println(channelRTC[j]);
      }
      memcpy(rtcSD.dato, channelRTC, 4);
      system_rtc_mem_write(PosDataC, &rtcSD, 4);
      PosDataC++;
} 

int PosDataD=DATO_WAPIKEY;
k=0;  
for(int i=0; i<4; i++){

    char wapikeyRTC[4];

    for(int j=0; j<4; j++){
      wapikeyRTC[j]=wapikey_char[k];
      //Serial.println(wapikeyRTC[j]);
      //Serial.println(wapikey_char[k]);
      k++;
      }
      memcpy(rtcSD.dato, wapikeyRTC, 4);
      system_rtc_mem_write(PosDataD, &rtcSD, 4);
      PosDataD++;
} 

int PosDataE=DATO_FIELD;
k=0;  
for(int i=0; i<grupo_field; i++){

    char fieldRTC[4];

    for(int j=0; j<4; j++){
      fieldRTC[j]=field_char[k];
      k++;
      }
      memcpy(rtcSD.dato, fieldRTC, 4);
      system_rtc_mem_write(PosDataE, &rtcSD, 4);
      PosDataE++;
} 
Serial.println("Guardados todos los datos en la memoria");
  
}else{

  Serial.println("No es necesario leer la tarjeta microSD");
  //Leemos el toggleFlag
  system_rtc_mem_read(64, &toggleFlag, 4);
  Serial.println("Toggle Flag actual: ");
  Serial.println(toggleFlag);
  system_rtc_mem_read(65, &rtcPos, 4);
  Serial.print("Posición actual: ");
  Serial.println(rtcPos);
  Serial.print("\n");

//Leemos la toogleFlag para ver si hemos terminado el ciclo o no, y reseteamos el contador    
  if (!toggleFlag) {
    Serial.print("Primera ejecución \n");
    int rtcPos = 66;
    rtcMem.battery = analogRead(A0)/1023*3*30*230;
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
    ESP.deepSleep(360000000);
  }

//Bucle para leer y almacenar valores
  else {
    Serial.print("Segunda condición\n");
    //Por si rtcPos toma valores fuera de los marcados
    if (rtcPos < 67 || rtcPos > 72){
    rtcPos = 67;
    system_rtc_mem_write(65, &rtcPos, 4);
    Serial.print("Posición corregida");
    }
    //Leemos y almacenamos
    rtcMem.battery = analogRead(A0)/1023*3*30*230;
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
    if (rtcPos == 72){
    toggleFlag = false;
    system_rtc_mem_write(64, &toggleFlag, 4);
    Serial.print("Nueva toggleFlag declarada: ");
    Serial.println(toggleFlag);
    Serial.print("Vamos a meter los datos de la memoria a un solo paquete\n");

    char jsonBuffer[500] = "["; // Initialize the jsonBuffer to hold data

    updatesJson(jsonBuffer);

//%%%%%%%%%%%%         LEEMOS LOS DATOS DE LA RTC MEMORY          %%%%%%%%%%%%%%%%%%%%%%%%%%%


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    SSID    %%%%%%%%%%%%%%%%%%%%%%%%%%%%

    int G_SSID = DATO_G_SSID;
    int G_PASSWORD = DATO_G_PASSWORD;
    
system_rtc_mem_read(G_SSID, &rtcLong, 4);
int long_ssid = rtcLong.longitud;
int grupos_ssid = long_ssid/4+1;
char ssidRTC[4];
//char ssidSUM[long_ssid];
String ssidData;
int PosData=DATO_SSID;
int k=0;
for(int i=0; i<grupos_ssid; i++){
  system_rtc_mem_read(PosData, &rtcSD, 4);
  memcpy(ssidRTC, rtcSD.dato, 4);

  for(int j=0; j<4; j++){

      ssidData += ssidRTC[j];
      k++;
  }
  PosData++;
}

//Serial.println("Dato ssid: ");
//Serial.println(ssidData);


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    PASSWORD    %%%%%%%%%%%%%%%%%%%%%%%%%%%%

system_rtc_mem_read(G_PASSWORD, &rtcLong, 4);
int long_password = rtcLong.longitud;
int grupos_password = long_password/4+1;
char passwordRTC[4];
//char passwordSUM[long_password];
String passwordData;
PosData=DATO_PASSWORD;
k=0;
for(int i=0; i<grupos_password; i++){
  system_rtc_mem_read(PosData, &rtcSD, 4);
  memcpy(passwordRTC, rtcSD.dato, 4);

  for(int j=0; j<4; j++){

      passwordData += passwordRTC[j];
      k++;
  }
  PosData++;
}

    //Inicializamos la librería WiFi %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    Serial.print("Vamos a encender la WiFi\n");
          //WiFiClient client;
        WiFi.begin(ssidData, passwordData);
          Serial.println();
          Serial.println();
          Serial.print("Connecting to ");
          Serial.println(ssidData);
        WiFi.begin(ssidData, passwordData);
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
    ESP.deepSleep(360000000,WAKE_RFCAL);
  }
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

// %%%%%%%%    LEEMOS EL DATO FIELD    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    int G_FIELD = DATO_G_FIELD;
    
system_rtc_mem_read(G_FIELD, &rtcLong, 4);
int long_field = rtcLong.longitud;
int grupos_field = long_field/4+1;
char fieldRTC[4];
//char fieldSUM[long_field];
String fieldData;
int PosData=DATO_FIELD;
int k=0;
for(int i=0; i<grupos_field; i++){
  system_rtc_mem_read(PosData, &rtcSD, 4);
  memcpy(fieldRTC, rtcSD.dato, 4);

  for(int j=0; j<4; j++){

      fieldData += fieldRTC[j];
      k++;
  }
  PosData++;
}



// %%%%%%%% PREPARAMOS EL DATO DE CAMPO%%%%%%%%%%%%%%
    char field_char[20];
    size_t lengthZ = String(fieldData).length();
    fieldData.toCharArray(field_char,lengthZ+1);
    
  // Format the jsonBuffer as noted above
  for (int i = 66; i < 72; i++){
    strcat(jsonBuffer,"{\"delta_t\":");
    unsigned long deltaT = 3600; //Tomamos datos cada 5 seg
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
  }
    // If posting interval time has reached 2 minutes, update the ThingSpeak channel with your data
    // Después de sacar todos los datos, enviamos
    
  size_t len = strlen(jsonBuffer);
  jsonBuffer[len-1] = ']';
  //httpRequest(jsonBuffer);
  Serial.println(jsonBuffer);    
   // lastUpdateTime = millis(); // Update the last update time
  Serial.println("\nProceso transpaso memoria RTC a flash para envío, completado\n");
  
}

//9) Define the httpRequest method to send data to ThingSpeak and to print the response code from the server. A response code 202 indicates that the server has accepted the request for precessing.
// Updates the ThingSpeakchannel with data
void httpRequest(char* jsonBuffer) {
  /* JSON format for data buffer in the API
   *  This examples uses the relative timestamp as it uses the "delta_t". You can also provide the absolute timestamp using the "created_at" parameter
   *  instead of "delta_t".
   *   "{\"write_api_key\":\"YOUR-CHANNEL-WRITEAPIKEY\",\"updates\":[{\"delta_t\":0,\"field1\":-60},{\"delta_t\":15,\"field1\":200},{\"delta_t\":15,\"field1\":-66}]
   */

// %%%%%%%%    LEEMOS EL DATO WAPIKEY    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    
    int G_CHANNEL = DATO_G_CHANNEL;
    int G_WAPIKEY = DATO_G_WAPIKEY;
    
  system_rtc_mem_read(G_WAPIKEY, &rtcLong, 4);
  int long_wapikey = rtcLong.longitud;
  int grupos_wapikey = long_wapikey/4;
  char wapikeyRTC[4];
  //char wapikeySUM[long_wapikey];
  String wapikeyData;
  int PosData=DATO_WAPIKEY;
  int k=0;
  for(int i=0; i<grupos_wapikey; i++){
    system_rtc_mem_read(PosData, &rtcSD, 4);
    memcpy(wapikeyRTC, rtcSD.dato, 4);
  
    for(int j=0; j<4; j++){
  
      wapikeyData += wapikeyRTC[j];
      k++;
    }
    PosData++;
  }


// %%%%%%%%    LEEMOS EL DATO CHANNEL    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  system_rtc_mem_read(G_CHANNEL, &rtcLong, 4);
  int long_channel = rtcLong.longitud;
  int grupos_channel = long_channel/4+1;
  //Serial.println("Grupos channel: ");
  //Serial.println(grupos_channel);
  char channelRTC[4];
  //char channelSUM[long_channel];
  String channelData;
  PosData=DATO_CHANNEL;
  k=0;
  for(int i=0; i<grupos_channel; i++){
    system_rtc_mem_read(PosData, &rtcSD, 4);
    memcpy(channelRTC, rtcSD.dato, 4);
  
    for(int j=0; j<4; j++){
  
      channelData += channelRTC[j];
      k++;
    }
    PosData++;
  }

  char data[500] = "{\"write_api_key\":\"";


  
  size_t lengthS = String(wapikeyData).length();
  char wapikey_char[20];
  wapikeyData.toCharArray(wapikey_char,lengthS+1);
  strcat(data,wapikey_char); //Asignar la variable wapikey
  strcat(data,"\",\"updates\":"); // Replace YOUR-CHANNEL-WRITEAPIKEY with your ThingSpeak channel write API key
  strcat(data,jsonBuffer);
  strcat(data,"}");

  
  char dataChannel[500] = "POST /channels/";
  //strcat(dataChannel,"POST /channels/");
  size_t lengthU = String(channelData).length();
  char channel_char[20];
  String(channelData).toCharArray(channel_char,lengthU+1);
  strcat(dataChannel,channel_char);
  /*String datodos = "/bulk_update.json HTTP/1.1";
  size_t lengthR = String(datodos).length();
  String(datodos).toCharArray(dataChannel,lengthR+1);*/
  strcat(dataChannel,"/bulk_update.json HTTP/1.1");
  client.println(dataChannel);  //Sustituto del "POST /channels/1680501/bulk_update.json HTTP/1.1"
  
  // Close any connection before sending a new request
  client.stop();
  String data_length = String(strlen(data)+1); //Compute the data buffer length
  
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
