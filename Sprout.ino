// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <atomic>
#include "Core.h"
#if __has_include("Config.h")
#include "Config.h"
#else
#include "Config.example.h"
#endif
#if !SPROUT_SIMULATION
#include "Audio.h"
#endif
// Wokwi's screen is 128x64; UI still occupies the exact 128x32 top region.
Adafruit_SSD1306 display(128,SPROUT_SIMULATION?64:32,&Wire,-1);
sprout::App app;
struct Request {sprout::Action action;unsigned preset;char audio[33];};
struct Result {char heading[22];char text[1025];char audio[33];bool answer;};
QueueHandle_t requests,results;
std::atomic<bool> stopAudio{false},audioPlaying{false};
bool screenOK=false;
#if !SPROUT_SIMULATION
AudioOutput audio;
SemaphoreHandle_t i2cMutex;
#endif
void logScreen(){char lines[3][22];app.lines(lines);Serial.printf("SCREEN %s [%u/%u] | %s | %s | %s\n",app.title,unsigned(app.page+1),unsigned(app.pages()),lines[0],lines[1],lines[2]);}
void render(){
 if(!screenOK)return;
#if !SPROUT_SIMULATION
 if(xSemaphoreTake(i2cMutex,pdMS_TO_TICKS(50))!=pdTRUE)return;
#endif
 display.clearDisplay();display.setTextColor(SSD1306_WHITE);display.setTextSize(1);display.setTextWrap(false);display.setCursor(0,0);
 if(app.busy)display.print("SPROUT / working...");else if(app.playing)display.print("AUDIO / press to stop");else display.print(app.title);
 char lines[3][22];app.lines(lines);for(int r=0;r<3;r++){display.setCursor(0,8*(r+1));display.print(lines[r]);}display.display();
#if !SPROUT_SIMULATION
 xSemaphoreGive(i2cMutex);
#endif
}
#if !SPROUT_SIMULATION
bool beginHttp(HTTPClient& http,WiFiClient& client,const String& path){
 if(WiFi.status()!=WL_CONNECTED)return false;
 if(!http.begin(client,String(SPROUT_BRIDGE_URL)+path))return false;
 http.setConnectTimeout(5000);http.setTimeout(60000);http.addHeader("Authorization",String("Bearer ")+SPROUT_BRIDGE_TOKEN);return true;
}
void fetch(Request req,Result& res){
 HTTPClient http;WiFiClient client;
 if(!beginHttp(http,client,req.action==sprout::Action::Weather?"/v1/weather":"/v1/ask")){snprintf(res.text,sizeof(res.text),"Wi-Fi offline. Check settings and laptop.");return;}
 int status;
 if(req.action==sprout::Action::Weather)status=http.GET();else{http.addHeader("Content-Type","application/json");JsonDocument body;body["preset"]=req.preset;String json;serializeJson(body,json);status=http.POST(json);}
 if(status!=200){snprintf(res.text,sizeof(res.text),"Bridge error %d. Check laptop bridge and token.",status);http.end();return;}
 // Bound payload before allocation; bridge always sends Content-Length.
 int len=http.getSize();if(len<0||len>8192){snprintf(res.text,sizeof(res.text),"Invalid bridge response size");http.end();return;}
 String payload=http.getString();JsonDocument doc;auto err=deserializeJson(doc,payload);http.end();
 if(err||!doc["text"].is<const char*>()){snprintf(res.text,sizeof(res.text),"Invalid bridge JSON");return;}
 snprintf(res.text,sizeof(res.text),"%s",doc["text"].as<const char*>());
 snprintf(res.heading,sizeof(res.heading),"%s",req.action==sprout::Action::Weather?"WEATHER":"WORKSHOP");
 res.answer=req.action==sprout::Action::Ask;snprintf(res.audio,sizeof(res.audio),"%s",doc["audio_id"]|"");
}
void playback(const char* id){
 // Only accept opaque hexadecimal ids, never a URL supplied by a response.
 if(strlen(id)!=32||strspn(id,"0123456789abcdef")!=32||!audio.ready)return;
 HTTPClient http;WiFiClient client;if(!beginHttp(http,client,String("/v1/audio/")+id))return;
 const char* headers[]={"Content-Type"};http.collectHeaders(headers,1);
 if(http.GET()!=200||http.header("Content-Type")!="application/octet-stream"){http.end();return;}
 int remaining=http.getSize();if(remaining<=0||remaining>48000*4*60||remaining%4){http.end();return;}
 uint8_t buffer[1024];size_t used=0;uint32_t last=millis();
 xSemaphoreTake(i2cMutex,portMAX_DELAY);audio.mute(false);xSemaphoreGive(i2cMutex);
 WiFiClient* stream=http.getStreamPtr();
 while(remaining>0&&!stopAudio.load()){
  int avail=stream->available();if(avail){int n=stream->read(buffer+used,min(size_t(avail),min(sizeof(buffer)-used,size_t(remaining))));if(n<=0)break;used+=n;remaining-=n;last=millis();size_t frames=used&~size_t(3);if(frames){size_t sent=0;while(sent<frames&&!stopAudio.load()){size_t nwrite=audio.write(buffer+sent,frames-sent);if(!nwrite){stopAudio=true;break;}sent+=nwrite;}memmove(buffer,buffer+frames,used-frames);used-=frames;}}
  else{if(!stream->connected()||uint32_t(millis()-last)>5000)break;vTaskDelay(pdMS_TO_TICKS(1));}
 }
 xSemaphoreTake(i2cMutex,portMAX_DELAY);audio.mute(true);xSemaphoreGive(i2cMutex);http.end();
}
#endif
void worker(void*){
#if !SPROUT_SIMULATION
 xSemaphoreTake(i2cMutex,portMAX_DELAY);bool ok=audio.begin();xSemaphoreGive(i2cMutex);Serial.printf("AUDIO codec=%s\n",ok?"ready":"not detected/configuration failed");
#endif
 Request req;
 for(;;){if(xQueueReceive(requests,&req,portMAX_DELAY)!=pdTRUE)continue;
  if(req.action==sprout::Action::Audio){
#if SPROUT_SIMULATION
   Serial.println("SIM_AUDIO_REQUEST (codec not modeled)");vTaskDelay(pdMS_TO_TICKS(1000));
#else
   playback(req.audio);
#endif
   audioPlaying=false;continue;
  }
  Result r{};snprintf(r.heading,sizeof(r.heading),"CONNECTION");
#if SPROUT_SIMULATION
  vTaskDelay(pdMS_TO_TICKS(150));
  if(req.action==sprout::Action::Weather){snprintf(r.heading,sizeof(r.heading),"WEATHER / DEMO");snprintf(r.text,sizeof(r.text),"21 C / light clouds. Wind 8 km/h. Fixture weather, not live.");}
  else{r.answer=true;snprintf(r.heading,sizeof(r.heading),"WORKSHOP / DEMO");snprintf(r.text,sizeof(r.text),"Prompt %u: Disconnect power first. Inspect connectors, document the symptom, and test one change at a time. This is a simulator fixture, not an LLM response.",req.preset);memset(r.audio,'a',32);r.audio[32]=0;}
#else
  fetch(req,r);
#endif
  xQueueSend(results,&r,portMAX_DELAY);
 }
}
void setup(){
 Serial.begin(115200);delay(250);Wire.begin(sprout::SDA_PIN,sprout::SCL_PIN);Wire.setClock(400000);
 for(int pin:{sprout::WEATHER_PIN,sprout::PREV_PIN,sprout::ASK_PIN})pinMode(pin,INPUT_PULLUP);
 screenOK=display.begin(SSD1306_SWITCHCAPVCC,0x3c,false,false);if(!screenOK)Serial.println("OLED init failed");
 requests=xQueueCreate(1,sizeof(Request));results=xQueueCreate(1,sizeof(Result));
#if !SPROUT_SIMULATION
 i2cMutex=xSemaphoreCreateMutex();WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);WiFi.begin(SPROUT_WIFI_SSID,SPROUT_WIFI_PASSWORD);
#endif
 xTaskCreatePinnedToCore(worker,"bridge_audio",12288,nullptr,1,nullptr,0);render();logScreen();Serial.println("SPROUT_READY");
}
void loop(){
 Result r;if(xQueueReceive(results,&r,0)==pdTRUE){app.complete(r.heading,r.text,r.answer,r.audio);logScreen();}
 app.playing=audioPlaying.load();sprout::Action action=app.update(digitalRead(1)==LOW,digitalRead(2)==LOW,digitalRead(4)==LOW,millis());
 if(action==sprout::Action::StopAudio){stopAudio=true;}
 else if(action!=sprout::Action::None){Request req{};req.action=action;req.preset=app.preset;snprintf(req.audio,sizeof(req.audio),"%s",app.audio);if(action==sprout::Action::Audio){stopAudio=false;audioPlaying=true;}if(xQueueSend(requests,&req,0)!=pdTRUE){app.busy=false;audioPlaying=false;app.set("BUSY","Please try again");}Serial.printf("ACTION %d preset=%u\n",int(action),app.preset);}
 static uint32_t drawn=0;static size_t oldpage=0;static unsigned oldpreset=0;
 if(oldpage!=app.page||oldpreset!=app.preset){logScreen();oldpage=app.page;oldpreset=app.preset;}
 if(uint32_t(millis()-drawn)>=100){render();drawn=millis();}delay(5);
}
