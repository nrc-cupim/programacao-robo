#include <Bluepad32.h>  //Biblioteca para o controle bluetooth

#define SENTIDO_MOTOR_ESQUERDO 27
#define VELOCIDADE_MOTOR_ESQUERDO 26

#define SENTIDO_MOTOR_DIREITO 25
#define VELOCIDADE_MOTOR_DIREITO 33

#define PINO_ARMA_1 32
#define PINO_ARMA_2 15

const int16_t toleranciaAnalogico = 20;
const int parado = 0, horario = 1, antihorario = 2;
const unsigned long tempoInversao = 1200;

ControllerPtr myControllers[1];

bool roboLigado;
int sentido;

void desligaRobo() {
  digitalWrite(PINO_ARMA_1, LOW);
  digitalWrite(PINO_ARMA_2, LOW);

  digitalWrite(SENTIDO_MOTOR_DIREITO, LOW);
  digitalWrite(SENTIDO_MOTOR_ESQUERDO, LOW);
  analogWrite(VELOCIDADE_MOTOR_ESQUERDO, 0);
  analogWrite(VELOCIDADE_MOTOR_DIREITO, 0);

  roboLigado = false;
  sentido = parado;
}

// Função para conctar o controle e garantir que somente um controle se conecte
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      ControllerProperties properties = ctl->getProperties();
      Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                    properties.product_id);
      myControllers[i] = ctl;
      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println("CALLBACK: Controller connected, but could not found empty slot");
  }
}

//Função para Disconectar o controle
void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }

  desligaRobo();
}

//Inputs do controle
void processControllers() {
  for (auto myController : myControllers) {

    if (myController && myController->isConnected() && myController->hasData() && myController->isGamepad()) {
      uint16_t botoesMisc = myController->miscButtons();

      if (botoesMisc == 0x04) {  //Start do controle para ligar o robo
        roboLigado = true;
        Serial.println("Robo iniciado");
      }

      else if (botoesMisc == 0x02) {  // Seletc do controle para desligar o robo
        roboLigado = false;
        Serial.println("Robo desligado");
      }

      if (roboLigado) {

        uint16_t botoesPressionados = myController->buttons();

        if (botoesPressionados == 0x0001) {  // se pressionou apenas X
          digitalWrite(PINO_ARMA_1, LOW);      // liga arma
          digitalWrite(PINO_ARMA_2, LOW); 
          Serial.print("Arma desligada ");
        }

        else if (botoesPressionados == 0x0008) {  // se pressionou apenas TRIÂNGULO
          digitalWrite(PINO_ARMA_1, HIGH);          // liga arma
          digitalWrite(PINO_ARMA_2, HIGH);
          Serial.print("Arma ligada ");
        }

        // Comportamento natural do controle em ambos os analógicos
        // Cima - Baixo +
        // Direita + Esquerda -

        int16_t valorAnalogicoDireito = -myController->axisRY();  // menos pra ficar com o mesmo sinal do plano cartesiano
        int16_t valorAnalogicoEsquerdo = myController->axisX();

        Serial.println(valorAnalogicoDireito);
        Serial.println(valorAnalogicoEsquerdo);

        // Pino de controle em HIGH: 255 (menor velocidade) a 0 (maior velocidade)
        // Pino de controle em LOW: 0(menor velocidade) a 255 (maior velocidade)

        int pwmMotorDireito = 0, pwmMotorEsquerdo = 0;

        if (valorAnalogicoDireito > toleranciaAnalogico) {  // Ir para frente

          if (sentido == antihorario) {
            sentido = horario;
            delay(tempoInversao);
          }

          digitalWrite(SENTIDO_MOTOR_ESQUERDO, HIGH);
          digitalWrite(SENTIDO_MOTOR_DIREITO, HIGH);

          if (valorAnalogicoEsquerdo > toleranciaAnalogico) {
            pwmMotorDireito = map(valorAnalogicoDireito - valorAnalogicoEsquerdo, -512, 512, 255, 0);
            pwmMotorEsquerdo = map(valorAnalogicoDireito + valorAnalogicoEsquerdo, 0, 1024, 255, 0);
          }

          else if (valorAnalogicoEsquerdo < -toleranciaAnalogico) {
            pwmMotorDireito = map(valorAnalogicoDireito - valorAnalogicoEsquerdo, 0, 1024, 255, 0);
            pwmMotorEsquerdo = map(valorAnalogicoDireito + valorAnalogicoEsquerdo, -512, 512, 255, 0);
          }

          else {
            pwmMotorDireito = map(valorAnalogicoDireito, toleranciaAnalogico, 512, 255, 0);
            pwmMotorEsquerdo = map(valorAnalogicoDireito, toleranciaAnalogico, 512, 255, 0);
          }
        }

        else if (valorAnalogicoDireito < -toleranciaAnalogico) {  // Ir para tras

          digitalWrite(SENTIDO_MOTOR_ESQUERDO, LOW);
          digitalWrite(SENTIDO_MOTOR_DIREITO, LOW);

          if (valorAnalogicoEsquerdo > toleranciaAnalogico) {
            pwmMotorDireito = map(valorAnalogicoEsquerdo + valorAnalogicoDireito, -512, 512, 0, 255);
            pwmMotorEsquerdo = map(valorAnalogicoEsquerdo - valorAnalogicoDireito, 0, 1024, 0, 255);
          }

          else if (valorAnalogicoEsquerdo < -toleranciaAnalogico) {
            pwmMotorDireito = map(valorAnalogicoEsquerdo + valorAnalogicoDireito, -512, 512, 0, 255);
            pwmMotorEsquerdo = map(valorAnalogicoEsquerdo - valorAnalogicoEsquerdo, 0, -1024, 0, 255);
          }

          else {
            pwmMotorDireito = map(valorAnalogicoDireito, -toleranciaAnalogico, -512, 0, 255);
            pwmMotorEsquerdo = map(valorAnalogicoDireito, -toleranciaAnalogico, -512, 0, 255);
          }
        }

        else {

          if (valorAnalogicoEsquerdo > toleranciaAnalogico) {
            digitalWrite(SENTIDO_MOTOR_ESQUERDO, HIGH);
            digitalWrite(SENTIDO_MOTOR_DIREITO, LOW);
            pwmMotorDireito = map(valorAnalogicoEsquerdo, toleranciaAnalogico, 512, 0, 255);
            pwmMotorEsquerdo = map(valorAnalogicoEsquerdo, toleranciaAnalogico, 512, 255, 0);
          }

          else if (valorAnalogicoEsquerdo < -toleranciaAnalogico) {
            digitalWrite(SENTIDO_MOTOR_ESQUERDO, LOW);
            digitalWrite(SENTIDO_MOTOR_DIREITO, HIGH);
            pwmMotorDireito = map(valorAnalogicoEsquerdo, -toleranciaAnalogico, -512, 255, 0);
            pwmMotorEsquerdo = map(valorAnalogicoEsquerdo, -toleranciaAnalogico, -512, 0, 255);
          }

          else {
            digitalWrite(SENTIDO_MOTOR_ESQUERDO, LOW);
            digitalWrite(SENTIDO_MOTOR_DIREITO, LOW);
          }
        }

        Serial.print("PWM Direito: ");
        Serial.println(pwmMotorDireito);
        Serial.print("PWM Esquerdo: ");
        Serial.println(pwmMotorEsquerdo);

        analogWrite(VELOCIDADE_MOTOR_DIREITO, pwmMotorDireito);
        analogWrite(VELOCIDADE_MOTOR_ESQUERDO, pwmMotorEsquerdo);
      }

      else
        desligaRobo();
    }
  }
}

void setup() {

  Serial.begin(115200);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);
  //BP32.forgetBluetoothKeys();

  pinMode(SENTIDO_MOTOR_ESQUERDO, OUTPUT);
  pinMode(VELOCIDADE_MOTOR_ESQUERDO, OUTPUT);

  pinMode(SENTIDO_MOTOR_DIREITO, OUTPUT);
  pinMode(VELOCIDADE_MOTOR_DIREITO, OUTPUT);

  pinMode(PINO_ARMA_1, OUTPUT);
  pinMode(PINO_ARMA_2, OUTPUT);

  desligaRobo();
}

void loop() {
  bool dataUpdated = BP32.update();
  if (dataUpdated)
    processControllers();
}