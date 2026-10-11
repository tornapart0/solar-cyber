#pragma once
#include <Adafruit_TLV320DAC3100.h>
#include <ESP_I2S.h>
#include "Core.h"
class AudioOutput {
 Adafruit_TLV320DAC3100 codec; I2SClass bus;
public:
 bool ready=false;
 bool begin(){
  pinMode(sprout::RESET_PIN,OUTPUT);digitalWrite(sprout::RESET_PIN,LOW);delay(100);digitalWrite(sprout::RESET_PIN,HIGH);delay(100);
  if(!codec.begin(0x18,&Wire))return false;
  bus.setPins(sprout::BCLK_PIN,sprout::WSEL_PIN,sprout::DATA_PIN);
  if(!bus.begin(I2S_MODE_STD,48000,I2S_DATA_BIT_WIDTH_16BIT,I2S_SLOT_MODE_STEREO))return false;
  // Stereo 16-bit: BCLK=32*48000. PLL / DAC divider math yields 48 kHz.
  bool ok=codec.setCodecInterface(TLV320DAC3100_FORMAT_I2S,TLV320DAC3100_DATA_LEN_16)
    &&codec.setCodecClockInput(TLV320DAC3100_CODEC_CLKIN_PLL)
    &&codec.setPLLClockInput(TLV320DAC3100_PLL_CLKIN_BCLK)
    &&codec.setPLLValues(1,2,32,0)&&codec.setNDAC(true,8)&&codec.setMDAC(true,2)
    &&codec.setDOSR(128)&&codec.powerPLL(true)
    &&codec.setDACDataPath(true,true,TLV320_DAC_PATH_NORMAL,TLV320_DAC_PATH_NORMAL,TLV320_VOLUME_STEP_1SAMPLE)
    &&codec.configureAnalogInputs(TLV320_DAC_ROUTE_MIXER,TLV320_DAC_ROUTE_MIXER,false,false,false,false)
    &&codec.setChannelVolume(false,-24)&&codec.setChannelVolume(true,-24)
    &&codec.setDACVolumeControl(true,true,TLV320_VOL_INDEPENDENT)
    &&codec.configureHeadphoneDriver(true,true,TLV320_HP_COMMON_1_35V,true)
    &&codec.configureHPL_PGA(0,true)&&codec.configureHPR_PGA(0,true)
    &&codec.setHPLVolume(true,32)&&codec.setHPRVolume(true,32)&&codec.enableSpeaker(false);
  ready=ok;if(!ok)bus.end();return ok;
 }
 void mute(bool yes){if(ready)codec.setDACVolumeControl(yes,yes,TLV320_VOL_INDEPENDENT);}
 size_t write(uint8_t* data,size_t len){return ready?bus.write(data,len):0;}
};
