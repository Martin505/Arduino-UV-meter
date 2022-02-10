//UV meter - Neo pixel

#include <Adafruit_NeoPixel.h>

#define DECODE_NEC
#include <IRremote.hpp>


//NEO LED definitions
#define VU_LED_PIN_1 PORTD3 //VU meter 1 datapin
#define VU_PIX_NUM 16 //VU meter segment/pixel number
#define ANALOG_R_PIN A2 //Audio Right channel pin ADC2
#define ANALOG_L_PIN A3 //Audio Left channel pin ADC3

#define DEFAULT_EFFECT_TICK 50 //Default effect timming in milliseconds
#define EFFECT_MAX_TICK 1000
#define EFFECT_MIN_TICK 25

#define MEASURE_TICK 10 //Measure timing in milliseconds

#define ANALOG_SAMPLES 5 //Number of samples from analog measurements to average

#define IR_RECEIVE_PIN PORTD2

Adafruit_NeoPixel pixels1(VU_PIX_NUM, VU_LED_PIN_1, NEO_RGB + NEO_KHZ800);

unsigned int effect_tick;
unsigned long time_effect;
unsigned long time_measure;
unsigned int led_flag;

unsigned int x;
byte VU_brightness[VU_PIX_NUM];
byte Red , Green , Blue;

unsigned int analog_r[ANALOG_SAMPLES]; //Analog right channel variable
unsigned int analog_l[ANALOG_SAMPLES]; //Analog left channel variable

unsigned int average_r;
unsigned int average_l;


enum IR_CODES{
  ON=0x40,
  OFF=0x41,
  RED=0x14,
  GREEN=0x15,
  BLUE=0x16,
  FASTER=0x5D,
  SLOWER=0x5C,
  MUSIC=0x58,
  MODE=0x44,
  BRIGHTER=0x0F,
  DIMMER=0x0B,
  W_WW=0x13,
};
// Example: IR_CODES::BLUE

void setup() {
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the IR receiver

  pinMode(ANALOG_R_PIN  , INPUT);
  pinMode(ANALOG_L_PIN  , INPUT);

  Serial.begin(115200);
  Serial.println("Connecting");

  pixels1.begin();
  Serial.println("Initialize NEO pixels");
  
  pixels1.clear();
  Serial.println("Clear NEO pixels");
  
  pixels1.show();
  
  effect_tick=DEFAULT_EFFECT_TICK;
  time_effect=millis()+effect_tick;
  time_measure=millis()+MEASURE_TICK;
  
  led_flag=1;
  
  x=0;
  
  Red=0;
  Green=0;
  Blue=50;

  for (int i=0; i<ANALOG_SAMPLES; i++){
    analog_r[i]=0;
    analog_l[i]=0;
  }
}


void loop() {
  //IR receiver program loop
  if (IrReceiver.decode()) {
    if(0==IrReceiver.decodedIRData.decodedRawData){
      IrReceiver.resume();
      return;
    }
    switch (IrReceiver.decodedIRData.command){
      case IR_CODES::RED:
        Red=255;
        Green=0;
        Blue=0;
        break;

      case IR_CODES::GREEN:
        Red=0;
        Green=255;
        Blue=0;
        break;

      case IR_CODES::BLUE:
        Red=0;
        Green=0;
        Blue=255;
        break;

      case IR_CODES::FASTER:
        if (effect_tick*2>EFFECT_MAX_TICK){effect_tick=EFFECT_MAX_TICK;}else{effect_tick=effect_tick*2;}
        break;

      case IR_CODES::SLOWER:
        if (effect_tick/2<EFFECT_MIN_TICK){effect_tick=EFFECT_MIN_TICK;}else{effect_tick=effect_tick/2;}
        break;

      default:
        Red=255;
        Green=255;
        Blue=255;
        break;
    }


    /*Serial.print("Raw data:");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    Serial.print("Address:");
    Serial.println(IrReceiver.decodedIRData.address, HEX);
    Serial.print("Command:");
    Serial.println(IrReceiver.decodedIRData.command, HEX);
    Serial.println();*/

    Serial.println();
    Serial.print("Effect tick: ");
    Serial.print(effect_tick);
    Serial.println("ms");
    Serial.println();
    
  
  IrReceiver.resume();
    }

  if(millis()>=time_measure){

    time_measure=time_measure+MEASURE_TICK;

    for (int i=0; i<(ANALOG_SAMPLES-1); i++){
      analog_r[i]=analog_r[i+1];
      analog_l[i]=analog_l[i+1];
    }
    
    analog_r[4]=abs(511-analogRead(ANALOG_R_PIN));
    analog_l[4]=abs(511-analogRead(ANALOG_L_PIN));

    average_r=0;
    average_l=0;
    
    for (int i=0; i<ANALOG_SAMPLES; i++){
      average_r=average_r+analog_r[i];
      average_l=average_l+analog_l[i];
    }

    average_r=average_r/ANALOG_SAMPLES;
    average_l=average_l/ANALOG_SAMPLES;
    
    /*Serial.print("Audio R:");
    Serial.println(average_r);
    Serial.print("Audio L:");
    Serial.println(average_l);
    Serial.println("");*/



  }


  if(millis()>=time_effect){
    //Serial.print("Time:");
    //Serial.println(time_effect);
    
    time_effect=time_effect+effect_tick;

    switch (led_flag){
      
      case 0:
        convert_VU(average_r*8);

        for(int i=0 ; i<VU_PIX_NUM ; i++){
          pixels1.setPixelColor(i , Red*VU_brightness[i]/255 , Green*VU_brightness[i]/255 , Blue*VU_brightness[i]/255);
        }

        pixels1.show();
        //led_flag=1;
        Serial.println("Red");
        break;  
    
      case 1:
        convert_VU(x*16);

        for(int i=0 ; i<VU_PIX_NUM ; i++){
          pixels1.setPixelColor(i , Red*VU_brightness[i]/255 , Green*VU_brightness[i]/255 , Blue*VU_brightness[i]/255 );
        }

        pixels1.show();
        if(x<255){x=x+1;}else{led_flag=2;}
        break;

      case 2:
        convert_VU(x*16);

        for(int i=0 ; i<VU_PIX_NUM ; i++){
          pixels1.setPixelColor(i , Red*VU_brightness[i]/255 , Green*VU_brightness[i]/255 , Blue*VU_brightness[i]/255 );
        }

        pixels1.show();
        if(x>0){x=x-1;}else{led_flag=3;}
        break;

      case 3:
        convert_VU(x*16);

        for(int i=0 ; i<VU_PIX_NUM ; i++){
          pixels1.setPixelColor(i , Red*VU_brightness[i]/255 , Green*VU_brightness[i]/255 , Blue*VU_brightness[i]/255 );
        }

        pixels1.show();
        if(x<255){x=x+1;}else{led_flag=4;}
        break;

      case 4:
        convert_VU(x*16);

        for(int i=0 ; i<VU_PIX_NUM ; i++){
          pixels1.setPixelColor(i , Red*VU_brightness[i]/255 , Green*VU_brightness[i]/255 , Blue*VU_brightness[i]/255 );
        }

        pixels1.show();
        if(x>0){x=x-1;}else{led_flag=5;}
        break;

      case 5:
        convert_VU(x*16);

        for(int i=0 ; i<VU_PIX_NUM ; i++){
          pixels1.setPixelColor(i , Red*VU_brightness[i]/255 , Green*VU_brightness[i]/255 , Blue*VU_brightness[i]/255 );
        }

        pixels1.show();
        if(x<255){x=x+1;}else{led_flag=6;}
        break;

      case 6:
        convert_VU(x*16);

        for(int i=0 ; i<VU_PIX_NUM ; i++){
          pixels1.setPixelColor(i , Red*VU_brightness[i]/255 , Green*VU_brightness[i]/255 , Blue*VU_brightness[i]/255 );
        }

        pixels1.show();
        if(x>0){x=x-1;}else{led_flag=1;}
        break;

      default:
        break;
    
    }
    //Serial.println(time_effect);
  }
}

void convert_VU(unsigned long VU_meter){
  long conversion_temp;
  for (unsigned int u=0; u<VU_PIX_NUM; u++){
  
    conversion_temp=VU_meter-(u*255);
    if(conversion_temp >=0){
      if(conversion_temp < 255){
        VU_brightness[u]=conversion_temp & 0xFF;
      }
      else{
        VU_brightness[u]=255;
      }
      }
      else{
        VU_brightness[u]=0;
      }
  }
}

/*
//Convert Pixel color to R G B
      
 unsigned int RGB_temp[3];
for (int p=0;p<VU_PIX_NUM; p++){
  RGB_temp[2]=((pixels1.getPixelColor(p) >> 16) & 0xFF);
  RGB_temp[1]=((pixels1.getPixelColor(p) >> 8) & 0xFF);
  RGB_temp[0]=(pixels1.getPixelColor(p) & 0xFF);
  Serial.print(RGB_temp[0]);
  Serial.print(",");
  Serial.print(RGB_temp[1]);
  Serial.print(",");
  Serial.println(RGB_temp[2]);
  }
Serial.println("");*/
