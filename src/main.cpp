#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WebServer.h>

#define BOT_TOKEN "TOKEN_BOT_TELEGRAM"
#define Rele 2
#define Rele2 12

bool ledState = LOW;
String groupChatId = "-1002465459990"; // Reemplaza con el chat_id del grupo


const unsigned long BOT_MTBS = 1000; // Tiempo medio entre revisiones de mensajes
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

unsigned long botLastTime = 0; // Última vez que se revisaron mensajes

// Lista de redes WiFi
const char* wifiNetworks[][2] = {
    {"Iphone Cristhian", "29874879"},
    {"Familia Morales Z", "Isabella1825*"},
    {"RedTerciaria", "PasswordTerciario"}
};

void connectToWiFi() {
    int networkCount = sizeof(wifiNetworks) / sizeof(wifiNetworks[0]);
    bool connected = false;

    for (int i = 0; i < networkCount && !connected; i++) {
        Serial.print("Conectando a la red: ");
        Serial.println(wifiNetworks[i][0]);
        WiFi.begin(wifiNetworks[i][0], wifiNetworks[i][1]);

        int wifiTimeout = 0;
        while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
            delay(500);
            Serial.print(".");
            wifiTimeout++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            Serial.println("\nConexión a WiFi exitosa");
            Serial.print("Dirección IP: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\nNo se pudo conectar a esta red.");
        }
    }

    if (!connected) {
        Serial.println("No se pudo conectar a ninguna red WiFi.");
    }
}

void handleNewMessages(int numNewMessages) {
    Serial.println("Procesando nuevos mensajes...");
    Serial.println(String(numNewMessages));

    for (int i = 0; i < numNewMessages; i++) {
        String chatId = bot.messages[i].chat_id;
        String text = bot.messages[i].text;

        String fromName = bot.messages[i].from_name;
        if (fromName == "") fromName = "Invitado";

        if (text == "/start") {
            String welcome = "Bienvenido Al sistema de Control, " + fromName + ".\n";
            welcome += "Ejemplo de acciones de chat.\n\n";
            welcome += "/led_on : Encender LED\n";
            welcome += "/led_off : Apagar LED\n";
            welcome += "/state : Ver estado del LED\n";
             welcome += chatId;
            welcome += "***Desarrollador Por Quantum Dynamics***\n";
            bot.sendMessage(chatId, welcome);
        }

        if (text == "/led_on") {
            if (ledState == HIGH) {
                bot.sendMessage(chatId, "El LED ya está ENCENDIDO.", "");
            } else {
                bot.sendMessage(chatId, "El LED ha sido encendido.", "");
                ledState = HIGH;
                digitalWrite(Rele, ledState);

                String notificationMessage = "La máquina ha sido prendida";
                bot.sendMessage(groupChatId, notificationMessage, "");
            }
        }

        if (text == "/led_off") {
              if (ledState == LOW) {
                  bot.sendMessage(chatId, "El LED ya está APAGADO.", "");
              } else {
                  bot.sendMessage(chatId, "El LED ha sido apagado.", "");
                  ledState = LOW;
                  digitalWrite(Rele, ledState);

                   // Enviar notificación a los operarios (grupo) sobre el apagado
                  String notificationMessage = "La máquina ha sido apagada.";
                  bot.sendMessage(groupChatId, notificationMessage, "");
              }
        }

        if (text == "/state") {
            String stateMessage = "El LED externo está " + String(ledState ? "ENCENDIDO" : "APAGADO");
            bot.sendMessage(chatId, stateMessage, "");
        }
    }
}

void setup() {
    Serial.begin(115200);

    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Agregar certificado raíz para Telegram
    connectToWiFi();

    pinMode(Rele, OUTPUT);
    pinMode(Rele2, OUTPUT);

    // No se usa Preferences ni se guarda estado
    ledState = LOW;
    digitalWrite(Rele, ledState);

}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi desconectado, intentando reconectar...");
        connectToWiFi();
    }

    if (millis() - botLastTime > BOT_MTBS) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages) {
            Serial.println("Nuevo mensaje recibido");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }

        botLastTime = millis();
    }
}
