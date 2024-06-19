#include <WiFi.h>
#include <WebServer.h>  // Inclui a biblioteca do servidor web

WebServer server(80);  // Cria uma instância do servidor web

const char* ssid = "Sistema De Alerta";
const char* password = "cobaias123";

String numbers[5] = {"", "", "", "", ""};
String messages[5] = {"", "", "", "", ""};

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  
  // Configurando o ponto de acesso
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Iniciando o servidor web
  server.on("/", handleRoot);
  server.on("/add", handleAdd);
  server.on("/delete", handleDelete);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><title>Sistema de Alerta</title>";
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
  html += "<h1>SISTEMA DE ALERTA PARA PROTECAO DE COBAIAS</h1>";
  html += "<div class='input-group'>";
  html += "<label for='operator'>Operadora:</label>";
  html += "<select id='operator'><option value='41'>Tim</option><option value='15'>Vivo</option><option value='31'>Oi</option><option value='21'>Claro</option></select>";
  html += "</div>";
  html += "<div class='input-group'>";
  html += "<label for='number'>Numero:</label>";
  html += "<input type='text' id='number'>";
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

void handleAdd() {
  if (server.hasArg("operator") && server.hasArg("number") && server.hasArg("message")) {
    String op = server.arg("operator");
    String number = server.arg("number");
    String message = server.arg("message");
    number = op + number;

    bool added = false;
    for (int i = 0; i < 5; i++) {
      if (numbers[i] == "") {
        numbers[i] = number;
        messages[i] = message;
        added = true;
        break;
      }
    }
    if (!added) {
      server.send(200, "text/plain", "FULL");
      return;
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleDelete() {
  if (server.hasArg("index")) {
    int index = server.arg("index").toInt();
    if (index >= 0 && index < 5) {
      numbers[index] = "";
      messages[index] = "";
    }
  }
  server.send(200, "text/plain", "OK");
}
