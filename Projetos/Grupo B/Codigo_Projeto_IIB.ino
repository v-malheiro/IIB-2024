/*Código do Projeto Final da disciplina Introdução à Instrumentação Biomédica - Dispositivo que controla a entrega de comprimidos ao paciente (Grupo B)*/

/* O atual código apresenta algumas limitações que devem ser levadas em consideração para garantir o correto funcionamento do dispositivo. Por exemplo, 
ao programar a abertura de um compartimento em um horário próximo da meia noite, o compartimento só ficará aberto até meia noite, o que pode ser um problema 
caso o paciente precise tomar algum remédio nesse horário. Evitar horários próximos de 00:00h. Não programar nenhuma abertura para 00:00h*/


#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


///////////////// WiFi e hora ///////////////


// Configuração do Wi-Fi
const char* ssid = "nome_da_rede";  // Colocar nome da rede
const char* password = "senha";  // Colocar senha da rede


// Configuração do NTPClient
const long utcOffsetInSeconds = -10800; //ajustado para o fuso horário
const char* timeServer = "pool.ntp.org"; // servidor NTP


WiFiUDP udp;
NTPClient timeClient (udp, timeServer, utcOffsetInSeconds);


//////// Definição dos horários ///////////


// Definir os horários para revelar cada compartimento, no formato hh:mm (horas:minutos)
int hora2 = 22;  // Hora para revelar compartimento 2
int minuto2 = 13; // Minuto para revelar compartimento 2
int hora3 = 22;  // Hora para revelar compartimento 3
int minuto3 = 14; // Minuto para revelar compartimento 3
int hora4 = 22;  // Hora para revelar compartimento 4
int minuto4 = 15; // Minuto para revelar compartimento 4


int horaAtual=0;
int minutoAtual=0;
int segundoAtual=0;


//////////// Definições dos LEDs e LDRs //////////////


const int led2 = 14; // LED do compartimento 2
const int led3 = 26; // LED do compartimento 3
const int led4 = 27; // LED do compartimento 4
const int ledaviso = 33; // LED que notifica o horário de tomar os remédios


const int ldr2 = 34; // LDR do compartimento 2
const int ldr3 = 35; // LDR do compartimento 3
const int ldr4 = 25; // LDR do compartimento 4 (é o que está dando problema). Só dá certo quando conectamos ao pino GPIO34 ou GPIO35. Nenhum outro pino funcionou.


int luminosidade2 = 0;
int luminosidade3 = 0;
int luminosidade4 = 0;


int limite2 = 1500; //determinar a sensibilidade de cada ldr e colocar os valores aqui, acima dos quais o sensor
int limite3 = 1500; // recebe luz o suficiente pra considerar o compartimento vazio
int limite4 = 1500;


//////// Variável auxiliar para verificar se o compartimento foi esvaziado ////////
int compartimento2vazio = 0;
int compartimento3vazio = 0;
int compartimento4vazio = 0;

// compartimentovazio = 0 => paciente não tomou remédio
// compartimentovazio = 1 => paciente tomou remédio, não precisa mais entrar no if do compartimento
// À meia noite, todas as variáveis de compartimentovazio voltam a ser 0 (paciente não tomou remédio), para que o programa possa rodar por mais de um dia

int i=0;

///////// Função para avisar paciente ////////// implementar buzzer aqui depois

void piscarledaviso() {

    for(i=0;i<4;i++){
      digitalWrite(ledaviso, HIGH);
      delay(500);
      digitalWrite(ledaviso, LOW);
      delay(500);
    }

}


/////////// Controle do Motor ///////////////


// Definir os pinos de controle do motor conectados ao driver ULN2003
const int motorPin1 = 18;  // GPIO 18 para IN1
const int motorPin2 = 19;  // GPIO 19 para IN2
const int motorPin3 = 21;  // GPIO 21 para IN3
const int motorPin4 = 22;  // GPIO 22 para IN4


int motorSpeed = 1000;  //determina a velocidade do motor
int count = 0;          // contagem de passos
int stepsPerRevolution = 2048/4; // número de passos por volta
int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001}; //não mexer aqui


const int stepsToMove2 = stepsPerRevolution/4; //Definir corretamente o sentido do giro com as funções clockwise e anticlockwise
const int stepsToMove3 = stepsPerRevolution/4;
const int stepsToMove4 = stepsPerRevolution/2;


void setOutput(int out) //não mexer aqui
{
  digitalWrite(motorPin1, bitRead(lookup[out], 0));
  digitalWrite(motorPin2, bitRead(lookup[out], 1));
  digitalWrite(motorPin3, bitRead(lookup[out], 2));
  digitalWrite(motorPin4, bitRead(lookup[out], 3));
  delayMicroseconds(motorSpeed * 2);
}


void anticlockwise(int volta) //não mexer aqui
{
  int count = volta;
  while (count > 0) {
    for(int i = 0; i < 8; i++)
    {
      setOutput(i);
    }
  count--;
  }
}


void clockwise(int volta) //não mexer aqui
{
  int count = 0;
  while (count < volta) {
    for(int i = 7; i >= 0; i--)
    {
      setOutput(i);
    }
    count++;
  }
}


//////////// SETUP ///////////////


void setup() {
 //declarar os pinos do motor como output
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
 
  // Definir pinos de LED e LDR
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(ledaviso, OUTPUT);
  pinMode(ldr2, INPUT);
  pinMode(ldr3, INPUT);
  pinMode(ldr4, INPUT);


  Serial.begin(115200);


  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi");


  // Inicializar o cliente NTP
  timeClient.begin();
 
  Serial.println("Sistema de Controle do Motor de Passo Iniciado");
}


///////////// LOOP //////////////


void loop(){

 // Atualizar o tempo do cliente NTP
  timeClient.update();
 
  // Obter horário atual
  int horaAtual = timeClient.getHours();
  int minutoAtual = timeClient.getMinutes();
  int segundoAtual = timeClient.getSeconds();
 
  //Para conferir o horário
  Serial.print("Hora atual: ");
  Serial.print(horaAtual);
  Serial.print(":");
  Serial.print(minutoAtual);
  Serial.print(":");
  Serial.println(segundoAtual);

 
  // Verificar se é hora de revelar o compartimento 2
  if ((horaAtual*60 + minutoAtual) >= (hora2*60 + minuto2) && (horaAtual*60+minutoAtual)<(hora3*60+minuto3) && compartimento2vazio!=1) { //vai entrar nesse if até o paciente tomar o remédio (ou até dar a hora do próximo compartimento)
     
    if (horaAtual==hora2 && minutoAtual==minuto2 && segundoAtual==0){ //revela o compartimento na hora certa (uma única vez)
        digitalWrite(led2, HIGH); //o led só acende no horário específico do compartimento
        Serial.println("Revelando Compartimento 2");
        clockwise(stepsToMove2); //gira 90° (sentido horário) para revelar compartimento 2  
    }
     
    luminosidade2 = analogRead(ldr2);
   
    if(luminosidade2>limite2) {     //retorna para o compartimento 1 se o ldr detectar mais luz (paciente tomou remédio)
        Serial.println("Retornando para o Compartimento 1");
        digitalWrite(led2, LOW); //desliga o led
        anticlockwise(stepsToMove2); //gira -90° (sentido anti-horário) para retornar pro compartimento 1
        compartimento2vazio = 1;
    }  
   
    else {
        piscarledaviso();
    }
  }


  // Verificar se é hora de revelar o compartimento 3
  if ((horaAtual*60 + minutoAtual) >= (hora3*60 + minuto3) && (horaAtual*60+minutoAtual)<(hora4*60+minuto4) && compartimento3vazio!=1) {
     
    if (horaAtual==hora3 && minutoAtual==minuto3 && segundoAtual==0){
        digitalWrite(led3, HIGH);
        Serial.println("Revelando Compartimento 3");
        anticlockwise(stepsToMove3); //gira -90° (sentido anti-horário) para revelar compartimento 3  
    }
     
    luminosidade3 = analogRead(ldr3);
   
    if(luminosidade3>limite3) {
        Serial.println("Retornando para o Compartimento 1");
        digitalWrite(led3, LOW);
        clockwise(stepsToMove3); //gira 90° (sentido horário) para retornar pro compartimento 1
        compartimento3vazio = 1;
    }  
   
    else {
        piscarledaviso();
    }
  }

 
  // Verificar se é hora de revelar o compartimento 4
  if ((horaAtual*60 + minutoAtual) >= (hora4*60 + minuto4) && compartimento4vazio!=1) {
     
    if (horaAtual==hora4 && minutoAtual==minuto4 && segundoAtual==0){
        digitalWrite(led4, HIGH);
        Serial.println("Revelando Compartimento 4");
        clockwise(stepsToMove4); //gira 180° para revelar compartimento 4  
    }
     
    luminosidade4 = analogRead(ldr4);
    // Serial.println(luminosidade4); //imprime zero sempre. No teste do LDR (outro código), esse valor nunca aparece como zero. É onde está nosso problema
   
    if(luminosidade4>limite4) {
        //Como luminosidade4 está sempre sendo lida como zero (não sabemos o motivo), o programa não está entrando nesse if
        Serial.println("Retornando para o Compartimento 1");
        digitalWrite(led4, LOW); 
        anticlockwise(stepsToMove4); //gira -180° para retornar pro compartimento 1
        compartimento4vazio = 1;
    }  
   
    else {
        piscarledaviso();
    }
  }
  
  /////// Para o programa poder rodar por mais de um dia /////////
  
  if(horaAtual<hora2){
      compartimento2vazio=0;
  }
  
  if(horaAtual<hora3){
      compartimento3vazio=0;
  }
  
  if(horaAtual<hora4){
      compartimento4vazio=0;
  }
  
  ////// Caso o usuário tente programar algum horário como 00:00h //////

  if(hora2==0 || hora3==0 || hora4==0){
      Serial.println("O programa só aceita horários a partir de 00:01h Evite horários próximos de 00:00h, pois os compartimentos ficarão abertos por pouco tempo.");
      delay(5000);
      }
      
}