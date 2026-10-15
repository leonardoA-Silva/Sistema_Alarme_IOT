#include <Arduino.h>
#include "AdafruitIO_WiFi.h"
#include <NewPing.h>
#include "secrets.h"

const int LED_ALARME = 13;
const int BUZZER_PIN = 27;
const int BOTAO_FISICO = 26;

// Pinos do sensor ultra sonico
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;

// Distancia máxima
const int MAX_DISTANCE = 50;
const int LIMITE_DISTANCIA = 15;

NewPing sonar(
    TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

AdafruitIO_WiFi io(
    IO_USERNAME,
    IO_KEY,
    WIFI_SSID,
    WIFI_PASS);

AdafruitIO_Feed *feedbotao = io.feed("botao");

// -----------------------------------
// Estado do sistema
bool alarmeAtivo = false;
unsigned long ultimaLeituraSensor = 0;
const unsigned long INTERVALO_SENSOR = 500; // 0.5 segundo
// -----------------------------------

// Controle do botão
byte ultimaLeituraBotao = HIGH;
byte estadobotaoestavel = HIGH;
unsigned long ultimamudancabotao = 0;
const unsigned long TEMPO_DEBOUNCE = 150;
// -----------------------------------

void verificarBotao();

void testeGeral();

void handleAlarme(AdafruitIO_Data *data);

void verificarSensor();

void ativarAlerta();

void desligarAlerta();

void setup()
{
  Serial.begin(115200);

  pinMode(LED_ALARME, OUTPUT);
  digitalWrite(LED_ALARME, LOW);

  pinMode(BOTAO_FISICO, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  desligarAlerta();

  feedbotao->onMessage(handleAlarme);

  Serial.println("");
  Serial.println("Iniciando a ESP...");

  io.connect();

  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  Serial.println(io.statusText());

  Serial.println("Adafruit IO Conectado");

  feedbotao->get();

  Serial.println("Sistema de alarme iniciado...");
  delay(2000);
}

void loop()
{
  io.run();

  // --- Aula:

  verificarBotao();
  verificarSensor();

  // --- Código de teste geral
  // testeGeral();
}

void verificarSensor()
{
  if(millis() - ultimaLeituraSensor < INTERVALO_SENSOR){
    return;
  }

  ultimaLeituraSensor = millis();
  unsigned int distancia = sonar.ping_cm();

  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  bool intrusao = alarmeAtivo && distancia > 0 && distancia < LIMITE_DISTANCIA;

  if(intrusao){
    ativarAlerta();
  }
  else{
    desligarAlerta();
  }
}

void ativarAlerta(){
  digitalWrite(LED_ALARME, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
};

void verificarBotao()
{
  int leituraAtual = digitalRead(BOTAO_FISICO);

  if (leituraAtual != ultimaLeituraBotao)
  {
    ultimamudancabotao = millis();
    ultimaLeituraBotao = leituraAtual;
  }

  if (millis() - ultimamudancabotao > TEMPO_DEBOUNCE)
  {
    estadobotaoestavel = leituraAtual;
    if (estadobotaoestavel == LOW)
    {
      alarmeAtivo = !alarmeAtivo;
      feedbotao->save(String(alarmeAtivo ? "true" : "false"));
      Serial.println(alarmeAtivo ? "Alarme ARMADO pelo botão físico" : "Alarme DESARMADO pelo botão físico");

      if (!alarmeAtivo)
      {
        desligarAlerta();
      }
    }
  }
}

void handleAlarme(AdafruitIO_Data *data)
{
  String valor = data->toString();

  valor.trim();
  valor.toLowerCase();

  if (valor == "true")
  {
    alarmeAtivo = true;
    Serial.println("Alarme ARMADO remotamente");
  }
  else if (valor == "false")
  {
    alarmeAtivo = false;
    Serial.println("Alarme DESARMADO remotamente");
  }
  else
  {
    Serial.print("Valor recebido inválido");
  }
}

void desligarAlerta()
{
  digitalWrite(LED_ALARME, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void testeGeral()
{
  // teste do ultrassônico
  unsigned int distancia = sonar.ping_cm();
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // teste do botão
  Serial.print("Estado do botão: ");
  Serial.println(digitalRead(BOTAO_FISICO));

  // teste led / buzzer
  digitalWrite(LED_ALARME, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ALARME, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
}