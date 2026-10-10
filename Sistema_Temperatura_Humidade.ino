
#include <Wire.h>                 // BIBLIOTECA PARA COMUNICACAO I2C
#include <Adafruit_GFX.h>         // BIBLIOTECA GRAFICA PARA O DISPLAY OLED
#include <Adafruit_SSD1306.h>     // BIBLIOTECA PARA CONTROLE DO DISPLAY SSD1306
#include <DHT.h>                  // BIBLIOTECA PARA LEITURA DO SENSOR DHT11

// ============================================================
// CONFIGURACOES DO DISPLAY OLED
// ============================================================

// DEFINE A RESOLUCAO DO DISPLAY EM PIXELS
#define LARGURA_TELA 128
#define ALTURA_TELA 64

// DEFINE O PINO DE RESET; -1 INDICA QUE NAO SERA UTILIZADO UM GPIO
#define OLED_RESET -1

// DEFINE O ENDERECO I2C DO DISPLAY
// OS ENDERECOS MAIS COMUNS SAO 0x3C E 0x3D
#define OLED_ENDERECO 0x3C

// CRIA O OBJETO RESPONSAVEL PELA COMUNICACAO COM O DISPLAY
Adafruit_SSD1306 display(
  LARGURA_TELA,
  ALTURA_TELA,
  &Wire,
  OLED_RESET
);

// ============================================================
// CONFIGURACOES DO SENSOR DHT11
// ============================================================

// DEFINE O GPIO CONECTADO AO PINO DATA DO SENSOR
#define DHTPIN 4

// DEFINE O MODELO DO SENSOR DE TEMPERATURA E UMIDADE
#define DHTTYPE DHT11

// CRIA O OBJETO RESPONSAVEL PELA LEITURA DO DHT11
DHT dht(DHTPIN, DHTTYPE);

// ============================================================
// CONFIGURACOES DOS COMPONENTES
// ============================================================

// DEFINE OS PINOS DOS LEDS E DO BUZZER
const int LED_VERDE = 27;  // INDICA FUNCIONAMENTO NORMAL
const int LED_AZUL = 26;   // INDICA QUE EXISTE UM ALERTA
const int BUZZER = 19;     // EMITE SINAIS SONOROS DE ALERTA

// ============================================================
// CONFIGURACAO DOS LIMITES AMBIENTAIS
// ============================================================

// ATIVA O ALERTA QUANDO A TEMPERATURA ULTRAPASSAR 30 GRAUS
const float TEMP_MAX = 30.0;

// ATIVA O ALERTA QUANDO A UMIDADE ULTRAPASSAR 70 POR CENTO
const float UMID_MAX = 70.0;

// ATIVA O ALERTA QUANDO A UMIDADE FICAR ABAIXO DE 30 POR CENTO
const float UMID_MIN = 30.0;

// DEFINE O LIMITE DE TEMPERATURA MUITO ALTA PARA O ALERTA COMBINADO
const float TEMP_MUITO_ALTA = 35.0;

// DEFINE O LIMITE DE UMIDADE MUITO BAIXA PARA O ALERTA COMBINADO
const float UMID_MUITO_BAIXA = 25.0;

// ============================================================
// VARIAVEIS DE CONTROLE
// ============================================================

// INDICA SE O SISTEMA ESTA EM ESTADO DE ALERTA
// TRUE = ALERTA ATIVO | FALSE = FUNCIONAMENTO NORMAL
bool estadoAlerta = false;

// ARMAZENA O MOMENTO DA ULTIMA LEITURA DO SENSOR
unsigned long ultimaLeitura = 0;

// ARMAZENA O MOMENTO EM QUE O ULTIMO BIP FOI INICIADO
unsigned long ultimoBip = 0;

// INDICA SE O BUZZER ESTA LIGADO
bool buzzerLigado = false;

// DEFINE O INTERVALO ENTRE AS LEITURAS DO SENSOR
// 2000 MILISSEGUNDOS EQUIVALEM A 2 SEGUNDOS
const unsigned long INTERVALO_LEITURA = 2000;

// DEFINE O INTERVALO ENTRE O INICIO DOS BIPS
const unsigned long INTERVALO_BIP = 1000;

// DEFINE A DURACAO DE CADA BIP
const unsigned long DURACAO_BIP = 150;

// ============================================================
// INICIALIZACAO DO SISTEMA
// EXECUTADA UMA VEZ QUANDO O ESP32 E LIGADO OU REINICIADO
// ============================================================

void setup() {

  // INICIA A COMUNICACAO SERIAL PARA DEPURACAO
  Serial.begin(115200);

  // CONFIGURA OS PINOS DOS LEDS E DO BUZZER COMO SAIDAS
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // DEFINE O ESTADO INICIAL DOS COMPONENTES
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AZUL, LOW);
  digitalWrite(BUZZER, LOW);

  // INICIALIZA O SENSOR DHT11
  dht.begin();

  // INICIALIZA O DISPLAY OLED UTILIZANDO O ENDERECO CONFIGURADO
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ENDERECO)) {

    // INFORMA O ERRO NO MONITOR SERIAL
    Serial.println("Falha ao iniciar o OLED!");

    // INTERROMPE A EXECUCAO SE O DISPLAY NAO FOR INICIALIZADO
    while (true) {
      delay(100);
    }
  }

  // LIMPA O CONTEUDO ANTERIOR DO DISPLAY
  display.clearDisplay();

  // DEFINE A COR DO TEXTO
  display.setTextColor(SSD1306_WHITE);

  // DEFINE O TAMANHO DO TEXTO
  display.setTextSize(1);

  // POSICIONA O CURSOR NO INICIO DO DISPLAY
  display.setCursor(0, 0);

  // EXIBE O TITULO DO PROJETO
  display.println("Estacao ambiental");

  // ADICIONA UMA LINHA EM BRANCO
  display.println();

  // INFORMA QUE O SISTEMA ESTA INICIALIZANDO
  display.println("Iniciando sensores...");

  // ENVIA O CONTEUDO PARA O DISPLAY FISICO
  display.display();

  // AGUARDA 1,5 SEGUNDO PARA EXIBIR A MENSAGEM INICIAL
  delay(1500);
}

// ============================================================
// LACO PRINCIPAL DO SISTEMA
// EXECUTADO REPETIDAMENTE ENQUANTO O ESP32 ESTIVER LIGADO
// ============================================================

void loop() {

  // OBTEM O TEMPO DE FUNCIONAMENTO DO ESP32 EM MILISSEGUNDOS
  // ESSA ABORDAGEM PERMITE CONTROLAR INTERVALOS SEM USAR DELAYS LONGOS
  unsigned long agora = millis();

  // ==========================================================
  // LEITURA DO SENSOR
  // ==========================================================

  // VERIFICA SE JA PASSOU O INTERVALO DEFINIDO PARA UMA NOVA LEITURA
  if (agora - ultimaLeitura >= INTERVALO_LEITURA) {

    // REGISTRA O MOMENTO DA LEITURA ATUAL
    ultimaLeitura = agora;

    // LE A UMIDADE RELATIVA DO AR EM PORCENTAGEM
    float umidade = dht.readHumidity();

    // LE A TEMPERATURA EM GRAUS CELSIUS
    float temperatura = dht.readTemperature();

    // VERIFICA SE O SENSOR RETORNOU VALORES INVALIDOS
    if (isnan(umidade) || isnan(temperatura)) {

      // INFORMA O ERRO NO MONITOR SERIAL
      Serial.println("Erro ao ler o DHT11!");

      // LIMPA O DISPLAY
      display.clearDisplay();

      // POSICIONA O CURSOR
      display.setCursor(0, 0);

      // EXIBE O TITULO
      display.println("ESTACAO AMBIENTAL");

      // ADICIONA UMA LINHA EM BRANCO
      display.println();

      // EXIBE A MENSAGEM DE ERRO
      display.println("Erro no DHT11!");
      display.println("Verifique conexoes.");

      // ATUALIZA O DISPLAY
      display.display();

      // DESATIVA O ESTADO DE ALERTA
      estadoAlerta = false;

      // INDICA ERRO DO SENSOR: APAGA OS DOIS LEDS
      // ISSO EVITA INDICAR NORMALIDADE QUANDO NAO HA LEITURA VALIDA
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AZUL, LOW);

      // DESLIGA O BUZZER
      digitalWrite(BUZZER, LOW);

      // ATUALIZA A VARIAVEL DE ESTADO DO BUZZER
      buzzerLigado = false;

      // SAI DESTA EXECUCAO DO LACO E TENTA NOVAMENTE NA PROXIMA ITERACAO
      return;
    }

    // ========================================================
    // VERIFICACAO DOS LIMITES AMBIENTAIS
    // ========================================================

    // VERIFICA SE A TEMPERATURA ESTA MUITO ALTA
    bool temperaturaMuitoAlta = temperatura >= TEMP_MUITO_ALTA;

    // VERIFICA SE A UMIDADE ESTA MUITO BAIXA
    bool umidadeMuitoBaixa = umidade <= UMID_MUITO_BAIXA;

    // ATIVA O ALERTA SE QUALQUER UMA DAS CONDICOES FOR VERDADEIRA:
    // 1. A TEMPERATURA ULTRAPASSAR O LIMITE MAXIMO
    // 2. A UMIDADE ULTRAPASSAR O LIMITE MAXIMO
    // 3. A UMIDADE FICAR ABAIXO DO LIMITE MINIMO
    // 4. A TEMPERATURA ESTIVER MUITO ALTA E A UMIDADE MUITO BAIXA AO MESMO TEMPO
    estadoAlerta =
      temperatura > TEMP_MAX ||
      umidade > UMID_MAX ||
      umidade < UMID_MIN ||
      (temperaturaMuitoAlta && umidadeMuitoBaixa);

    // ========================================================
    // CONTROLE DOS LEDS
    // ========================================================

    // APAGA O LED VERDE QUANDO EXISTIR ALERTA
    // MANTEM O LED VERDE ACESO QUANDO O AMBIENTE ESTIVER NORMAL
    digitalWrite(LED_VERDE, estadoAlerta ? LOW : HIGH);

    // ACENDE O LED AZUL QUANDO EXISTIR ALERTA
    // MANTEM O LED AZUL APAGADO QUANDO O AMBIENTE ESTIVER NORMAL
    digitalWrite(LED_AZUL, estadoAlerta ? HIGH : LOW);

    // ========================================================
    // EXIBICAO DOS DADOS NO MONITOR SERIAL
    // ========================================================

    // EXIBE A TEMPERATURA MEDIDA
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.print(" C | Umidade: ");

    // EXIBE A UMIDADE MEDIDA
    Serial.print(umidade);
    Serial.print("% | Estado: ");

    // EXIBE O ESTADO ATUAL DO SISTEMA
    Serial.println(estadoAlerta ? "ALERTA" : "NORMAL");

    // ========================================================
    // ATUALIZACAO DO DISPLAY OLED
    // ========================================================

    // LIMPA O CONTEUDO ANTERIOR DO DISPLAY
    display.clearDisplay();

    // DEFINE A COR DO TEXTO
    display.setTextColor(SSD1306_WHITE);

    // DEFINE O TAMANHO DO TEXTO
    display.setTextSize(1);

    // POSICIONA O CURSOR NO INICIO DO DISPLAY
    display.setCursor(0, 0);

    // EXIBE O TITULO DO SISTEMA
    display.println("ESTACAO AMBIENTAL");

    // DESENHA UMA LINHA PARA SEPARAR O TITULO DAS MEDICOES
    display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

    // EXIBE A TEMPERATURA COM UMA CASA DECIMAL
    display.setCursor(0, 19);
    display.print("Temperatura: ");
    display.print(temperatura, 1);
    display.println(" C");

    // EXIBE A UMIDADE COM UMA CASA DECIMAL
    display.setCursor(0, 32);
    display.print("Umidade: ");
    display.print(umidade, 1);
    display.println("%");

    // DESENHA UMA LINHA PARA SEPARAR AS MEDICOES DO ESTADO
    display.drawLine(0, 45, 127, 45, SSD1306_WHITE);

    // POSICIONA O CURSOR NA AREA DE STATUS
    display.setCursor(0, 51);

    // EXIBE SE O AMBIENTE ESTA NORMAL OU EM ALERTA
    if (estadoAlerta) {
      display.println("!! ALERTA AMBIENTAL");
    } else {
      display.println("Estado: NORMAL");
    }

    // ENVIA AS INFORMACOES ATUALIZADAS PARA O DISPLAY
    display.display();
  }

  // ==========================================================
  // CONTROLE SONORO DO ALERTA
  // ==========================================================

  // VERIFICA SE O SISTEMA ESTA EM ESTADO DE ALERTA
  if (estadoAlerta) {

    // INICIA UM BIP SE O BUZZER ESTIVER DESLIGADO
    // E SE O INTERVALO ENTRE OS BIPS JA TIVER SIDO ATINGIDO
    if (!buzzerLigado && agora - ultimoBip >= INTERVALO_BIP) {

      // LIGA O BUZZER
      digitalWrite(BUZZER, HIGH);

      // REGISTRA QUE O BUZZER ESTA LIGADO
      buzzerLigado = true;

      // ARMAZENA O MOMENTO EM QUE O BIP FOI INICIADO
      ultimoBip = agora;
    }

    // VERIFICA SE O BUZZER JA FICOU LIGADO PELO TEMPO DEFINIDO
    if (buzzerLigado && agora - ultimoBip >= DURACAO_BIP) {

      // DESLIGA O BUZZER PARA FINALIZAR O BIP
      digitalWrite(BUZZER, LOW);

      // REGISTRA QUE O BUZZER ESTA DESLIGADO
      buzzerLigado = false;
    }

  } else {

    // GARANTE QUE O BUZZER FIQUE DESLIGADO QUANDO NAO HOUVER ALERTA
    digitalWrite(BUZZER, LOW);

    // ATUALIZA A VARIAVEL DE CONTROLE DO BUZZER
    buzzerLigado = false;
  }
}