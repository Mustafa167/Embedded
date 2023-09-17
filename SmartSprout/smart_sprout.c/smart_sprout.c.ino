/*This code is intended for the device known as SmartSprout*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "SmartSprout";  // SSID of the ESP8266 AP
const char* password = "";       // Empty password for the AP

ESP8266WebServer server(80);

const char* htmlPage = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wi-Fi Configuration</title>
         <style>
      body {
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
        margin: 0;
        flex-direction: column; /* Stack elements vertically */
      }

      .input-container {
        display: flex;
        flex-direction: column; /* Stack inputs and labels vertically */
        align-items: flex-end; /* Align labels to the right */
      }

      label {
        font-weight: bold;
        margin-right: 10px; /* Add spacing between labels and inputs */
      }

      input[type="text"] {
        padding: 10px;
        width: 300px;
        margin: 10px 0;
      }

      /* Style for the form container */
      form {
        display: flex;
        flex-direction: column;
        align-items: center; /* Center the form horizontally */
      }

      /* Style for the submit button */
      button[type="submit"] {
        padding: 10px 20px;
        background-color: #007bff;
        color: #fff;
        border: none;
        cursor: pointer;
      }
    </style>
</head>
<body>
    <h1>Welcome to SmartSprout Wi-Fi Configuration</h1>
    <h2>Enter your Wi-Fi SSID and Password</h2>
    <form action="/configure" method="POST">
        <div class="input-container">
            <div>
                <label for="ssidInputField">SSID:</label>
                <input type="text" id="ssidInputField" name="ssid" placeholder="Enter SSID here">
            </div>
            <div>
                <label for="passwordInputField">Password:</label>
                <input type="text" id="passwordInputField" name="password" placeholder="Enter password here">
            </div>
        </div>
        <!-- Submit button -->
        <button type="submit" id="submitButton" disabled>Submit</button>
    </form>
    
    <script>
        function checkInputs() {
            const ssidInput = document.getElementById('ssidInputField');
            const passwordInput = document.getElementById('passwordInputField');
            const submitButton = document.getElementById('submitButton');
            
            if (ssidInput.value.trim() !== '' && passwordInput.value.trim() !== '') {
                submitButton.removeAttribute('disabled');
            } else {
                submitButton.setAttribute('disabled', 'disabled');
            }
        }
    </script>
</body>
</html>
)";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleConfigure() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  // Process the received SSID and password
  // You can save them to memory or use them for configuration

  // Respond with a confirmation message
  String response = "Configuration received:<br>SSID: " + ssid + "<br>Password: " + password;
  server.send(200, "text/html", response);
}

void setup() {
  Serial.begin(115200);
  
  // Start the ESP8266 in AP mode with an empty password
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  Serial.println();
  Serial.println("ESP8266 is in Access Point (AP) mode");
  Serial.print("AP SSID: ");
  Serial.println(ssid);
  Serial.print("AP Password: ");
  Serial.println(password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define routes for handling root and configuration requests
  server.on("/", HTTP_GET, handleRoot);
  server.on("/configure", HTTP_POST, handleConfigure);

  server.begin();
}

void loop() {
  server.handleClient(); // Handle incoming client requests
}
