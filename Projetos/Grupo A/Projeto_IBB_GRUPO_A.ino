//PROJETO TERMÔMETRO IOT
//GRUPO A: Giovanna S. Ragassi, Hugo Costa, José Elias Morato e Pedro H. Paixão

//Disciplina de Instrodução à Instrumentação Biomédica

//Bibliotecas
#include <WiFi.h>
#include <WebServer.h>

// Credenciais Wi-Fi
const char *ssid = "AndroidAP2c03";
const char *password = "zzpc0258";
const int analogPin = 32;
const int adcRes = 4095;

float calcTemp(float volt) {
  const float A = 0.00362;
  const float B = 4.3e-4;
  const float C = -1.4747e-5;
  float vRazao = volt / 3.3;

  if (vRazao <= 0) {
    return -999;
  }

  float lnRazao = log(vRazao);
  float tempK = 1 / (A + (B * lnRazao) + (C * pow(lnRazao, 3)));
  float tempC = tempK - 273.15;
  return tempC;
}

// Variável dinâmica
int temperatura = 0; // inicia var

WebServer server(80);

// Função para lidar com a página raiz
void handleRoot() {
  server.send(200, "text/plain", String(temperatura));
}

void setup(void) {
  Serial.begin(115200);
  analogReadResolution(12);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Configura o servidor
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop(void) {
  int valor = analogRead(analogPin);
  float volt = (valor / float(adcRes)) * 3.3;
  temperatura = calcTemp(volt); //calcTemp(volt)
  server.handleClient();
  delay(1000);
}