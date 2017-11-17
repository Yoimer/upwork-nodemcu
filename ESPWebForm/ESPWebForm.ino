/*
 * Demonstrate using an http server and an HTML form to control an LED.
 * The http server runs on the ESP8266.
 *
 * Connect to "http://esp8266WebForm.local" or "http://<IP address>"
 * to bring up an HTML form to control the LED connected GPIO#0. This works
 * for the Adafruit ESP8266 HUZZAH but the LED may be on a different pin on
 * other breakout boards.
 *
 * Imperatives to turn the LED on/off using a non-browser http client.
 * For example, using wget.
 * $ wget http://esp8266webform.local/ledon
 * $ wget http://esp8266webform.local/ledoff
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

// define timeout for expected answer from PIC
#define TIMEOUT 2000

// Start DNS svr
const byte DNS_PORT = 53;
IPAddress apIP(192,168,4,1);
DNSServer dnsServer;

// Start Web svr
ESP8266WebServer server(80);


const char INDEX_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
	"<head>"
	"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
	"<title>ESP8266 Web Form Demo</title>"
	"<style>"
		"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
	"</style>"
	"</head>"
	"<body>"
		"<h1>ESP8266 Web Form Demo</h1>"
		"<FORM action=\"/\" method=\"post\">"
			"<P>"
				"LED<br>"
				"<INPUT type=\"radio\" name=\"LED\" value=\"1\">On<BR>"
				"<INPUT type=\"radio\" name=\"LED\" value=\"0\">Off<BR>"
				"<INPUT type=\"radio\" name=\"DUMP\" value=\"DUMP\">Go to TRANSMIT.NET<BR>"
				"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
			"</P>"
		"</FORM>"
	"</body>"
"</html>";

const char PICKUP_POLARITY[] =
"<tr>"
	"<td>"
		"<FORM action=\"/\" method=\"get\">"
			"<font color=blue>PICKUP POLARITY:</font><br>"
			"<INPUT type=\"radio\" name=\"pos_00_adr_13_val\" value=\"00\">NP<font color=grey>(0)</font>"
			"<INPUT type=\"radio\" name=\"pos_00_adr_13_val\" value=\"01\" checked>PN<font color=grey>(1)</font>"
			"&nbsp; &nbsp; <INPUT type=\"submit\" value=\"Send to CDI\">"
		"</FORM>"
	"</td>"
"</tr>"

"<tfoot>"
	"<td>"
		"<div id=\"formularie\">"
			"<FORM action=\"/dump\" method=\"post\">"
				"<INPUT type=\"submit\" value =\"Dump Eprom\">"
			"</FORM>"
			"<FORM action=\"/\" method=\"post\">"
				"<INPUT type=\"submit\" value =\"Home\">"
			"</FORM>"
			"<FORM action=\"/clear\" method=\"post\">"
				"<INPUT type=\"submit\" value =\"Clear display\">"
			"</FORM>"
		"</div>"
	"</td>"
"</tfoot>";

String page = "";

String raw_data = "";

// GPIO#0 is for Adafruit ESP8266 HUZZAH board. Your board LED might be on 13.
//const int LED_BUILTIN = HIGH;

void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  writeLED(true);

  Serial.begin(2400);
  Serial.println("");

  // reply with IP to "*" domain name request
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/ledon", handleLEDon);
  server.on("/ledoff", handleLEDoff);
  //server.onNotFound(handleNotFound);
  server.onNotFound(handleRoot);
  server.begin();

}

/////////////////////////////////////////////////////
void loop(void)
{
  dnsServer.processNextRequest();
  server.handleClient();
}

/////////////////////////////////////////////////////
void handleRoot()
{
  if (server.hasArg("LED") || server.hasArg("DUMP") ) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", INDEX_HTML);
  }
}

//////////////////////////////////////////////////////
void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}

/////////////////////////////////////////////////////
void handleSubmit()
{
    String LEDvalue;
  String DUMPvalue;

  if (server.hasArg("LED"))
  {
    LEDvalue = server.arg("LED");
    if (LEDvalue == "1") {
      writeLED(true);
      server.send(200, "text/html", INDEX_HTML);
    }
    else if (LEDvalue == "0") {
      writeLED(false);
      server.send(200, "text/html", INDEX_HTML);
    }
    else {
      returnFail("Bad LED value");
    }
  }
  else if(server.hasArg("DUMP"))
  {
    DUMPvalue = server.arg("DUMP");
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");

    uint8_t answer = 0;
    // check empty eeprom
    ////answer = sendPICcommand("99 000", "500: 255-", TIMEOUT, 0) || sendPICcommand("99 000", "1000: 255-", TIMEOUT, 0);
    
    if (answer == 1) {
      Serial.println("eeprom is empty");
    }else {
        Serial.println("reading eeprom");
        // show data saved in PIC
        // E is the expected last char
        // it reads every char before getting to
        // E and displays it on console and browser too
        ////sendPICcommand("99 000", "E", TIMEOUT, 1);
		////page = "<h1>Values saved on eeprom </h1><h3>Raw Data:</h3> <h4>"+raw_data+"</h4>";
		page = PICKUP_POLARITY;
		server.send(200, "text/html", page);
      }
  }
}

///////////////////////////////////////////////////
void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK\r\n");
}

///////////////////////////////////////////////////
/*
 * Imperative to turn the LED on using a non-browser http client.
 * For example, using wget.
 * $ wget http://esp8266webform/ledon
 */
void handleLEDon()
{
  writeLED(true);
  returnOK();
}

///////////////////////////////////////////////////
/*
 * Imperative to turn the LED off using a non-browser http client.
 * For example, using wget.
 * $ wget http://esp8266webform/ledoff
 */
void handleLEDoff()
{
  writeLED(false);
  returnOK();
}

//////////////////////////////////////////////////
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

///////////////////////////////////////////////
void writeLED(bool LEDon)
{
  // Note inverted logic for Adafruit HUZZAH board
  if (LEDon)
    digitalWrite(LED_BUILTIN, LOW);
  else
    digitalWrite(LED_BUILTIN, HIGH);
}

/////////////////////////////////////////////////////////////////
int8_t sendPICcommand(char* PICcommand, char* expected_answer, unsigned int timeout, int show_response) {

  uint8_t x = 0,  answer = 0;
  char response[150];
  unsigned long previous;

  memset(response, '\0', 150);    // Initialize the string

  delay(100);

  //while ( Serial.available() > 0) Serial.read();   // Clean the input buffer

  while (Serial.available()) { //Cleans the input buffer
    Serial.read();
  }

  Serial.println(PICcommand);    // Send the PIC command
  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    if (Serial.available() != 0) {
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = Serial.read();
      x++;
      // check if the desired answer  is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  } while ((answer == 0) && ((millis() - previous) < timeout));
  
  if (show_response == 1) {
    Serial.println(response);
    Serial.println(answer);
	raw_data = "";
	raw_data = String(response);
	Serial.println("Printing raw data: ");
	Serial.println(raw_data);
  }

  return answer;
}

