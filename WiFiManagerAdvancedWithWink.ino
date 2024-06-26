/**
 * File: WiFiManagerAdvancedWithWink
 * Modified by: Forrest Lee Erickson
 * Date: 20201201
 * 
 * WiFiManager advanced demo, contains advanced configurartion options
 * Implements TRIGGEN_PIN button press, press for on demand configportal, hold for 3 seconds for reset settings.
 */
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// Once in main loop, press and hold this pin to reset stored settings for wireless AP.
#define TRIGGER_PIN 25 

//For on board blue LED.
const int led_gpio = 2;
int lastLEDtime = 0;
int nextLEDchange = 100; //time in ms.

//AP Station SSID and password
const char* SSID_AP = "Amused_Scientist";  // Amused Scientist AP.
const char* passwordAP = "cautionAS";  // Amused Scientist pw.
//const int32_t APCHANNEL = 1;          // AP channel.
const int32_t APCHANNEL = 6;          // AP channel default.
const char* COUNTRY = "US";           // CN and JP also possible.
const int32_t setupTimeOut = 120;      //Generous two minuets for setup to WiFi LAN

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )

void setup() {
  pinMode(led_gpio, OUTPUT);      // set the LED pin mode
  digitalWrite(led_gpio, HIGH);   // turn the LED on (HIGH is the voltage level)
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  delay(500);
  Serial.println("\n Starting WiFiManagerAdvancedWithWink");

    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  // wm.resetSettings(); // wipe settings

  // add a custom input field
  int customFieldLength = 40;
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");
  
  // test custom html input type(checkbox)
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); // custom html type
  
  // test custom html(radio)
  const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input
  
  wm.addParameter(&custom_field);
  Serial.print("Save Parameters count: ");
  Serial.println(wm.getParametersCount());
//  Serial.println(&&wm.getParameters(1));
  wm.setSaveParamsCallback(saveParamCallback);

  wm.setWiFiAPChannel(APCHANNEL);   //FLE added 20201201
  wm.setCountry(COUNTRY);   //FLE added 20201201  must be defined in WiFiSetCountry, US, JP, CN

  // Custom web page menu via array or vector  
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"}; 
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  wm.setMenu(menu);

  // set web page dark theme
  wm.setClass("invert");

  //set static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  // wm.setShowStaticFields(true); // force show static ip fields
  // wm.setShowDnsFields(true);    // force show dns field always

    wm.setConfigPortalTimeout(setupTimeOut); // auto close configportal after n seconds
  // wm.setCaptivePortalEnable(false); // disable captive portal redirection
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons
  
  // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

  int startConnectTime = 0;
  startConnectTime = millis();
  Serial.print("Start connection at: "); Serial.println(startConnectTime);  

  bool res;                         //Catch return results.
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap   
//  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap hard coded.
  res = wm.autoConnect(SSID_AP,passwordAP); // password protected ap hard coded.
  IPAddress apIP(192, 168, 1, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
//  wm.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  
//  res = wm.autoConnect(SSID_AP,passwordAP); // password protected ap
  delay(10);
  Serial.print("Finished autoConnect at: ");
  Serial.println(millis());  
  Serial.print("Elaped time: "); 
  Serial.println((millis()-startConnectTime));  
    
  if(!res) {
    Serial.print("Failed to connect to ");
//    Serial.print(SSID_AP);
    Serial.print(wm.getConfigPortalSSID());
//    Serial.print(wm.getID();
    Serial.println(" hit timeout");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.print("Connect to ");
    Serial.println(wm.getConfigPortalSSID());    
  }
  Serial.println("!!!! Continuing on to loop.");
}// End setup

void checkButton(){
  // check for button press
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    if( digitalRead(TRIGGER_PIN) == LOW ){
      Serial.println("Button Pressed");
      // still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000); // reset delay hold
      if( digitalRead(TRIGGER_PIN) == LOW ){
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        wm.resetSettings();
        ESP.restart();
      }
      
      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);
      
//      if (!wm.startConfigPortal("OnDemandAP","password")) {
      if (!wm.startConfigPortal(SSID_AP,passwordAP)) {
        Serial.println("failed to connect 'SSID_AP' or hit timeout");
        delay(3000);
        // ESP.restart();
      } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected to WiFi AP:)");
        Serial.println(SSID_AP);
        Serial.println("!!!!!!");
      }
    }// button pressed debounced success
  }
}// end check button.


String getParam(String name){
  //read parameter from server, for customhmtl input
  Serial.println("getParam fired with value: ");
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
    Serial.println(value);
  }
  return value;
}

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}

void loop() {
  checkButton();
  // put your main code here, to run repeatedly:
  
  //Wink the LED
  if ((millis()-lastLEDtime)>nextLEDchange){
//    Serial.print("LED test >=0. "); Serial.print(lastLEDtime); Serial.print(" ");  Serial.println(millis());
    if(digitalRead(led_gpio)==LOW){
     digitalWrite(led_gpio, HIGH);   // turn the LED on (HIGH is the voltage level)
     lastLEDtime = millis();
     nextLEDchange = 900; 
    }else{
      digitalWrite(led_gpio, LOW);   // turn the LED on (HIGH is the voltage level)
     lastLEDtime = millis();
     nextLEDchange = 100; 
    }     
  }//LED change

}// end loop
