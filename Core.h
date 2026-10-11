#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
namespace sprout {
constexpr int WEATHER_PIN=1, PREV_PIN=2, ASK_PIN=4, SDA_PIN=5, SCL_PIN=6;
constexpr int BCLK_PIN=7, WSEL_PIN=8, DATA_PIN=9, RESET_PIN=44;
constexpr size_t MAX_TEXT=1024, COLS=21, ROWS=3, PAGE_CHARS=COLS*ROWS;
constexpr const char* PROMPTS[]={"Next repair step?","Explain this circuit","Workshop checklist"};
enum class Action { None, Weather, Ask, Audio, StopAudio };
struct Button {
 bool raw=false, stable=false, held=false; uint32_t changed=0, pressed=0;
 // 0 none, 1 short release, 2 hold. Unsigned subtraction handles millis wrap.
 int update(bool down,uint32_t now){
  if(down!=raw){raw=down;changed=now;}
  if(raw!=stable && uint32_t(now-changed)>=30){stable=raw;if(stable){pressed=now;held=false;}else if(!held)return 1;}
  if(stable&&!held&&uint32_t(now-pressed)>=700){held=true;return 2;}
  return 0;
 }
};
struct App {
 Button buttons[3]; char text[MAX_TEXT+1]="Press WEATHER or ASK";
 char title[22]="SPROUT"; char audio[33]="";
 size_t page=0; unsigned preset=0; bool answer=false,busy=false,playing=false;
 void set(const char* heading,const char* body,bool isAnswer=false,const char* id=""){
  snprintf(title,sizeof(title),"%.21s",heading);size_t j=0;
  // ASCII font: convert UTF-8 glyph to one '?' rather than counting continuation bytes.
  for(size_t i=0;body[i]&&j<MAX_TEXT;++i){unsigned char c=body[i];if(c>=128){if((c&0xc0)!=0x80)text[j++]='?';}else text[j++]=(c=='\n'||c=='\r'||c=='\t')?' ':((c>=32&&c<127)?c:'?');}
  text[j]=0; page=0;answer=isAnswer;snprintf(audio,sizeof(audio),"%.32s",id);
 }
 size_t pages()const {size_t n=strlen(text);return n?(n+PAGE_CHARS-1)/PAGE_CHARS:1;}
 void lines(char out[ROWS][COLS+1])const{
  size_t n=strlen(text),start=page*PAGE_CHARS;
  for(size_t r=0;r<ROWS;++r){size_t pos=start+r*COLS,len=pos<n?(n-pos<COLS?n-pos:COLS):0;memcpy(out[r],text+(pos<n?pos:n),len);out[r][len]=0;}
 }
 Action update(bool weather,bool prev,bool ask,uint32_t now){
  int w=buttons[0].update(weather,now),p=buttons[1].update(prev,now),a=buttons[2].update(ask,now);
  if(playing&&(w||p||a))return Action::StopAudio;
  if(busy)return Action::None;
  if(w==1){busy=true;return Action::Weather;}
  if(p==1){preset=(preset+1)%3;set("Choose / ASK",PROMPTS[preset]);return Action::None;}
  if(a==2&&answer&&audio[0]){playing=true;return Action::Audio;}
  if(a==1){if(answer){page=(page+1)%pages();return Action::None;}busy=true;return Action::Ask;}
  return Action::None;
 }
 void complete(const char* heading,const char* body,bool isAnswer=false,const char* id=""){busy=false;set(heading,body,isAnswer,id);}
};
}
