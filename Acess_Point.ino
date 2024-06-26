// ------------------------------------------------------------------ //
//                          INCLUDE E DEFINDE                         //
// ------------------------------------------------------------------ //
#include <WiFi.h>
#include <WebServer.h>  // Inclui a biblioteca do servidor web
#include <SoftwareSerial.h>

#define BOTAO     23

// ------------------------------------------------------------------ //
//                            VARIAVEIS                               //
// ------------------------------------------------------------------ //

// ----- Servidor
WebServer server(80);  // Cria uma instância do servidor web

const char* ssid = "Sistema de Alerta";
const char* password = "bloco9";

String numbers[5] = {"2149988702542", "", "", "", ""};
String messages[5] = {"Olha a msg", "", "", "", ""};

// ----- GPRS
#include <SoftwareSerial.h>

// SoftwareSerial RX 16 
//                TX 17
SoftwareSerial software_serial(16, 17);


// ----- Misc.
int _click = 1;

// ------------------------------------------------------------------ //
//                              SETUP                                 //
// ------------------------------------------------------------------ //
void setup() 
{
  // ----- Configuracao do Acess Point
  Serial.begin(9600);
  Serial.println();
  Serial.print("Configurando Acess Ponit...");
  
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Endereco IP do Acess Point: ");
  Serial.println(IP);
  
  // Iniciando o servidor web
  server.on("/", pagina_principal);
  server.on("/add", add_numero);
  server.on("/delete", deleta_numero);
  server.begin();
  Serial.println("server HTTP iniciado");

  // ----- Configuracao do SIM800l
  software_serial.begin(9600);
  delay(3000);

  Serial.println("Testando o GPRS:");
  teste_sim800_module();

  // ----- Botao
  pinMode(BOTAO, INPUT_PULLUP);
}

// ------------------------------------------------------------------ //
//                                LOOP                                //
// ------------------------------------------------------------------ //                      
void loop() 
{
  // Acess Point
  server.handleClient();

  // GPRS
  int _click = digitalRead(BOTAO);

  if(_click == 0)
  {
    delay(300);
    Serial.println("Ligacao");

    faz_ligacao();
  }

  //software_serial.println("AT+CSQ");
  //update_serial();
}

// ------------------------------------------------------------------ //
//                                 HTML                               //
// ------------------------------------------------------------------ //
void pagina_principal() 
{
  String html = "<!DOCTYPE html><html>";
  html += "<head><title>Sistema de Alerta</title>";
  html += "<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; color: #333; }";
  html += "h1 { color: #0056b3; }";
  html += "p { font-size: 18px; }";
  html += ".container { margin-top: 50px; }";
  html += ".content { display: inline-block; text-align: left; background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
  html += "table { width: 100%; border-collapse: collapse; }";
  html += "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }";
  html += "th { background-color: #f2f2f2; }";
  html += ".btn { padding: 10px 20px; margin: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }";
  html += ".btn-danger { background-color: #f44336; }";
  html += ".input-group { margin: 10px 0; }";
  html += "input, select { padding: 10px; margin: 5px; border: 1px solid #ccc; border-radius: 5px; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>SISTEMA DE ALERTA PARA PROTEÇÃO DE COBAIAS</h1>";
  html += "<div class='input-group'>";
  html += "<label for='operator'>Operadora:</label>";
  html += "<select id='operator'><option value='41'>Tim</option><option value='15'>Vivo</option><option value='31'>Oi</option><option value='21'>Claro</option></select>";
  html += "</div>";
  html += "<div class='input-group'>";
  html += "<label for='number'>Número:</label>";
  html += "<input type='number' id='number'>";
  html += "</div>";
  html += "<div class='input-group'>";
  html += "<label for='message'>Mensagem:</label>";
  html += "<input type='text' id='message'>";
  html += "</div>";
  html += "<button class='btn' onclick='addEntry()'>Adicionar</button>";
  html += "<hr>";
  html += "<table>";
  html += "<tr><th>ID</th><th>Numero</th><th>Mensagem</th><th>Acao</th></tr>";
  for (int i = 0; i < 5; i++) {
    html += "<tr><td>" + String(i + 1) + "</td><td>" + numbers[i] + "</td><td>" + messages[i] + "</td><td><button class='btn btn-danger' onclick='deleteEntry(" + String(i) + ")'>X</button></td></tr>";
  }
  html += "</table>";
  html += "</div>";
  html += "<script>";
  html += "function addEntry() {";
  html += "  var operator = document.getElementById('operator').value;";
  html += "  var number = document.getElementById('number').value;";
  html += "  var message = document.getElementById('message').value;";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      if (this.responseText == 'FULL') {";
  html += "        alert('Maximo de 5 numeros atingido. Por favor, exclua um numero antes de adicionar outro.');";
  html += "      } else {";
  html += "        location.reload();";
  html += "      }";
  html += "    }";
  html += "  };";
  html += "  xhttp.open('GET', '/add?operator=' + operator + '&number=' + number + '&message=' + message, true);";
  html += "  xhttp.send();";
  html += "}";
  html += "function deleteEntry(index) {";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.open('GET', '/delete?index=' + index, true);";
  html += "  xhttp.send();";
  html += "  setTimeout(function(){ location.reload(); }, 500);";
  html += "}";
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ------------------------------------------------------------------ //
//                           ADICIONAR NUMERO                         //
// ------------------------------------------------------------------ //
void add_numero() 
{
  if (server.hasArg("operator") && server.hasArg("number") && server.hasArg("message")) 
  {
    String op = server.arg("operator");
    String number = server.arg("number");
    String message = server.arg("message");
    number = op + number;

    bool adicionado = false;
    for (int i = 0; i < 5; i++) 
    {
      if (numbers[i] == "") 
      {
        numbers[i] = number;
        messages[i] = message;
        adicionado = true;
        break;
      }
    }

    if (!adicionado) 
    {
      server.send(200, "text/plain", "FULL");
      return;
    }
  }

  server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------ //
//                            APAGAR NUMERO                           //
// ------------------------------------------------------------------ //
void deleta_numero() 
{
  if (server.hasArg("index")) 
  {
    int index = server.arg("index").toInt();

    if (index >= 0 && index < 5) 
    {
      numbers[index] = "";
      messages[index] = "";
    }
  }
  server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------ //
//                            FAZ LIGAÇÃO                             //
// ------------------------------------------------------------------ //
void faz_ligacao()
{
  for(int i = 0; i < 5; i++)
  {
    if (numbers[i] != "") 
    {
      Serial.println("Iniciando os Trabalhos, Fazendo Ligação!");

      //software_serial.println("AT+CMGF=0"); // Configura o modo de Ligacao
      //update_serial();

      String numero = "ATD0" + numbers[i] + ";";
      // Faz a ligacao
      software_serial.println(numero);
      update_serial();

      delay(20000);
      software_serial.println("ATH"); // Desliga
      software_serial.println("AT+CMGF=1"); // Configura o modo de Texto
      update_serial();

      // Configura numero da mensagem
      Serial.println("Mandando MSG!");
      numero = "AT+CMGF=\"+55" + numbers[i] + "\"";
      software_serial.println(numero);
      update_serial();

      // Envia a Mensagem
      software_serial.println(messages[i]);
      update_serial();

      software_serial.write(26);
      update_serial();

      Serial.println("Finalizou os trabalhos!");
    }
  }
}

// ------------------------------------------------------------------ //
//                          ESPELHA AS SERIAIS                        //
// ------------------------------------------------------------------ //
void update_serial()
{
  delay(500);
  while (Serial.available())
  {
    // Envia o que foi recebido da Serial Principal para a Software Serial
    software_serial.write(Serial.read());
  }
  while (software_serial.available())
  {
    // Envia o que foi recebido pela Software Serial para a Serial Principal
    Serial.write(software_serial.read());
  }
}

// ------------------------------------------------------------------ //
//                          TESTE GERAL DO GPRS                       //
// ------------------------------------------------------------------ //
void teste_sim800_module()
{
  // Atenção
  software_serial.println("AT");
  update_serial();

  // Potência do Sinal
  software_serial.println("AT+CSQ");
  update_serial();

  // Número do Cartão
  software_serial.println("AT+CCID");
  update_serial();

  // Registro na Rede
  software_serial.println("AT+CREG?");
  update_serial();

  // Informações de Identificação
  software_serial.println("ATI");
  update_serial();
}
