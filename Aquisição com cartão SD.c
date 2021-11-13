#include <EmonLib.h>
#include <SdFat.h>
#include <Wire.h>
#include "rgb_lcd.h"

// --- Mapeamento de Hardware ---
#define butUp    10
#define butDown  9
#define butP     8
#define butM     7

// --- Protótipo das Funções Auxiliares ---
void changeMenu();
void dispMenu();
void LCD();
void intervalo();
void calc_potencia();

// --- Variáveis Globais ---
char menu = 0x01;
char set1 = 0x00, set2 = 0x00;
boolean t_butUp, t_butDown, t_butP, t_butM; //Flags dos botões, para evitar o acionamento descontrolado

int rede = 127;
double sensor = A0; //Pino em que o sensor foi ligado
double potencia;
double Irms;
const int cartao_sd = 4;
int i = 0;
int leituras = 1;

EnergyMonitor emon1;

// --- Hardware do LCD ---
rgb_lcd lcd;

// --- Hardware do SD ---
SdFat sdCard;
SdFile consumo;

// --- Configurações Iniciais ---
void setup()
{
  lcd.begin(16, 2);

  if (!sdCard.begin(cartao_sd, SPI_HALF_SPEED)) //Verifica se há erros no cartão, ou se o cartão esta inserido
  {
    sdCard.initErrorHalt(lcd.print("Erro SD")); //Caso tenha algum problema, informa o usuario
  }
  if (!consumo.open("CON0001.txt", O_RDWR | O_CREAT | O_AT_END)) //Cria o arquivo no cartão SD
  {
    sdCard.errorHalt("Erro na abertura do arquivo CON001.TXT!");
    lcd.print("Erro na abertura do arquivo");
  }

  lcd.setCursor(0, 0);
  lcd.print("Aguardando...");
  delay(2000);
  lcd.clear();

  for (char i = 7; i < 11; i++) pinMode(i, INPUT_PULLUP);

  //Zera as flags
  t_butUp   = 0x00;
  t_butDown = 0x00;
  t_butP    = 0x00;
  t_butM    = 0x00;

} //end setup


// --- Loop Infinito ---
void loop()
{
  changeMenu();
  dispMenu();
}

// --- Desenvolvimento das Funções Auxiliares ---
void changeMenu() //Nesse bloco é possivel adicionar mais itens ao menu
{
  if (!digitalRead(butUp))   t_butUp   = 0x01; //Se apertado o botão Cima, aumenta a flag para 1
  if (!digitalRead(butDown)) t_butDown = 0x01; //Se apertado o botão Baixo, aumenta a flag para 1

  if (digitalRead(butUp) && t_butUp) //Movimenta o menu pra cima
  {
    t_butUp = 0x00;
    lcd.clear();
    menu++;

    if (menu > 0x02) menu = 0x01;

  } //end butUp

  if (digitalRead(butDown) && t_butDown) //Movimenta o menu para baixo
  {
    t_butDown = 0x00;
    lcd.clear();
    menu--;

    if (menu < 0x01) menu = 0x02;

  } //end butDown

} //end changeMenu

void dispMenu() //Escolhe os itens do menu
{
  switch (menu)
  {
    case 0x01:
      intervalo();
      break;

    case 0x02:
      LCD();
      calc_potencia();
      break;
  } //end switch menu
} //end dispMenu

void calc_potencia() //Calcula a potencia que vai ser mostrada no visor
{
  emon1.current(sensor, 60); //Calibra o sensor
  Irms = emon1.calcIrms(1480);
  potencia = Irms * rede;
}

void LCD () //Função do menu do LCD
{
  if (potencia > 30) //Filtra os "lixos" do sensor
  {
    if (i > 0)
    {
      lcd.setCursor(0, 0); //Coloca o cursor no ponto (0,0)
      lcd.print("Potencia:"); //Escreve Potencia
      lcd.setCursor(0, 1); //Muda o cursor para o ponto (0,1)
      lcd.print(i); //Numero de leituras atual
      lcd.print(" - ");
      lcd.print(potencia); //Imprime o que estiver na função potencia
      lcd.print("W");
      consumo.println(potencia); //Grava a potencia calculada no cartao SD
      delay (1000); //Intervalo de medições
    }
    i++;

    if (i == leituras) //Fecha arquivo do SD e encerra o loop se i for igual ao leituras
    {
      consumo.close();
      delay(1000);
      lcd.clear();
      lcd.setCursor (0, 0);
      lcd.print("Retire SD");
      lcd.setCursor(0, 1);
      lcd.print("Gravado!");
      while (1) {}; //Encerra o loop do arduino, fazendo com que o codigo pause
    }
  }
}

void intervalo() //Parte do menu que pode-se aumentar o numero de leitura atraves dos botões do LCD
{
  lcd.setCursor(0, 0);
  lcd.print("Leituras:");
  lcd.setCursor(1, 1);
  if (!digitalRead(butP)) t_butP = 0x01;
  if (!digitalRead(butM)) t_butM = 0x01;

  if (digitalRead(butP) && t_butP) //Aumenta o numero de leituras
  {
    t_butP = 0x00;
    leituras ++;
  }
  if (digitalRead(butM) && t_butM) //Diminui o numero de leituras
  {
    t_butM = 0x00;
    leituras --;
  }
  lcd.print(leituras - 1); //Volta no Menu do LCD
}
