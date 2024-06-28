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
const char* password = "bloco9alerta";

String numero_ligacao[5] = {"", "", "", "", ""}; 
String numeros_originais[5] = {"", "", "", "", ""};
String messages[5] = {"", "", "", "", ""};

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
  // ----- Configuração do Acess Point
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

  // ----- Configuração do SIM800l
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
    Serial.println("Ligação");

    faz_ligacao();
  }
}

// ------------------------------------------------------------------ //
//                                 HTML                               //
// ------------------------------------------------------------------ //
void pagina_principal() 
{
  String html = "<!DOCTYPE html><htmllang=\"pt-BR\">";
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
  html += "<div style='display: flex; flex-direction: column; align-items: center;'>";
  html += "<div class='input-group' style='width: 80%; display: flex; justify-content: flex-start; align-items: center; text-align: start;'>";
  html += "<label for='operator' style='width: 30%;'>Operadora:</label>";
  html += "<select id='operator' style='width: 70%;'><option value='41'>Tim</option><option value='15'>Vivo</option><option value='31'>Oi</option><option value='21'>Claro</option></select>";
  html += "</div>";
  html += "<div class='input-group' style='width: 80%; display: flex; justify-content: flex-start; align-items: center; text-align: start;'>";
  html += "<label for='number' style='width: 30%;'>Número:</label>";
  html += "<input type='number' id='number' style='width: 70%;'>";
  html += "</div>";
  html += "<div class='input-group' style'width: 80%; display: flex; justify-content: flex-start; align-items: center; text-align: start;'>";
  html += "<label for='message' style='width: 30%;'>Mensagem:</label>";
  html += "<input type='text' id='message' style='width: 70%;'>";
  html += "</div>";
  html += "<button class='btn' onclick='addEntry()'>Adicionar</button>";
  html += "<hr>";
  html += "<table>";
  html += "<tr><th>ID</th><th>Número</th><th>Mensagem</th><th>Ação</th></tr>";
  for (int i = 0; i < 5; i++) {
    html += "<tr><td>" + String(i + 1) + "</td><td>" + numeros_originais[i] + "</td><td>" + messages[i] + "</td><td><button class='btn btn-danger' onclick='deleteEntry(" + String(i) + ")'>X</button></td></tr>";
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
  html += "        alert('Máximo de 5 números atingido. Por favor, exclua um número antes de adicionar outro.');";
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
    String call_number = op + number;

    bool adicionado = false;
    for (int i = 0; i < 5; i++) 
    {
      if (numeros_originais[i] == "") 
      {
        numeros_originais[i] = number;
        numero_ligacao[i] = call_number;
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
      numeros_originais[index] = "";
      numero_ligacao[index] = "";
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
    if (numeros_originais[i] != "") 
    {
      Serial.println("Iniciando os Trabalhos, Fazendo Ligação!");

      String numero = "ATD0" + numero_ligacao[i] + ";";
      // Faz a ligacao
      software_serial.println(numero);
      update_serial();
      delay(20000);

      software_serial.println("ATH"); // Desliga
      update_serial();

      software_serial.println("AT+CMGF=1"); // Configura o modo de Texto
      update_serial();

      // Configura numero da mensagem
      numero = "AT+CMGS=\"+55" + numeros_originais[i] + "\"";
      software_serial.println(numero);
      update_serial();

      // Envia a Mensagem
      software_serial.print(messages[i]);
      update_serial();

      software_serial.write(26);
      update_serial();

      delay(2000);

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