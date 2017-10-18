#include <Wire.h>
#include "rgb_lcd.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Keypad.h>

rgb_lcd lcd;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys_1[ROWS][COLS] = { // other side
  {'A','B','C','D'},
  {'3','6','9','#'},
  {'1','4','7','*'},
  {'2','5','8','0'}
};
//23 - 53 Total 16 pins for two keypads
byte rowPins1[ROWS] = {37, 35, 33, 31}; // no arduino side
byte colPins1[COLS] = {23, 25, 27, 29}; 
byte rowPins2[ROWS] = {53, 51, 49, 47};
byte colPins2[COLS] = {39, 41, 43, 45}; 
char keys_2[ROWS][COLS] = { // arduino side
  {'A','B','C','D'},
  {'3','6','9','#'},
  {'2','5','8','0'},
  {'1','4','7','*'}
};
Keypad keypad1 = Keypad( makeKeymap(keys_1), rowPins1, colPins1, ROWS, COLS );
Keypad keypad2 = Keypad( makeKeymap(keys_2), rowPins2, colPins2, ROWS, COLS );
#define P1_LED_STRIP 6
#define P2_LED_STRIP 5
#define X_MAX 4
#define Y_MAX 4
#define MAX_PIX 159
uint32_t max_pix = MAX_PIX;
int P1_shipboard[X_MAX][Y_MAX];
int P2_shipboard[X_MAX][Y_MAX];
int P1_bombboard[X_MAX][Y_MAX];
int P2_bombboard[X_MAX][Y_MAX];
int P1_gameboard[X_MAX][Y_MAX];
int P2_gameboard[X_MAX][Y_MAX];
int P1_tempboard[X_MAX][Y_MAX];
int P2_tempboard[X_MAX][Y_MAX];
int Empty_board[X_MAX][Y_MAX];
int shipCnt[2];
Adafruit_NeoPixel strip_1 = Adafruit_NeoPixel(max_pix, P1_LED_STRIP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_2 = Adafruit_NeoPixel(max_pix, P2_LED_STRIP, NEO_GRB + NEO_KHZ800);
uint32_t c_rgb1[9] = {strip_1.Color(0, 0, 0),strip_1.Color(155, 0, 0), strip_1.Color(0, 155, 0), strip_1.Color(0, 0, 155), strip_1.Color(100,100,100), strip_1.Color(100,100,0), strip_1.Color(0,100,100), strip_1.Color(100,0,100), strip_1.Color(100,100,100),};
uint32_t c_rgb2[9] = {strip_2.Color(0, 0, 0),strip_2.Color(155, 0, 0), strip_2.Color(0, 155, 0), strip_2.Color(0, 0, 155), strip_2.Color(100,100,100), strip_2.Color(100,100,0), strip_2.Color(0,100,100), strip_2.Color(100,0,100), strip_2.Color(100,100,100),}; 
// 0 - Off, 1 - R, 2 - G, 3 - B, 4 - White, 5 - Yellow, 6 - Light Blue, 7 - Purple, 8 - Grey
// R, G, B, Yellow, Light Blue, Purple

unsigned long startResetButton1[3]; // 1 - other side - 13, 12, 11, 2 - arduino side - 10, 9, 8
int startButton_1_ID = 13;
unsigned long rotateButton1[3];
int rotateButton_1_ID = 12;
unsigned long selectButton1[3];
int selectButton_1_ID = 11;
unsigned long startResetButton2[3];
int startButton_2_ID = 10;
unsigned long rotateButton2[3];
int rotateButton_2_ID = 9;
unsigned long selectButton2[3];
int selectButton_2_ID = 8;
bool RESET = false;
bool STARTGAME = false;
int TURN = 0; // 0 - both player, 1 - P1. 2 - P2
bool P1_rotate = false; // 0 - verticle, 1 - horizontal
bool P2_rotate = false;

void setup() 
{
    lcd.begin(16, 2);
    lcd.setRGB(0,0, 255);
    lcd.print("Setting Up...");
    strip_1.begin();
    strip_1.show(); // Initialize all pixels to 'off'
    strip_2.begin();
    strip_2.show();
    Serial.begin(9600);
    initialize();
    lcd.setRGB(0, 255, 0);
    initializeButton(startResetButton1, startButton_1_ID); // other side
    initializeButton(startResetButton2, startButton_2_ID);
    initializeButton(rotateButton1, rotateButton_1_ID);
    initializeButton(rotateButton2, rotateButton_2_ID);
    initializeButton(selectButton1, selectButton_1_ID);
    initializeButton(selectButton2, selectButton_2_ID);

    int board[X_MAX][Y_MAX] = {
                                {2,2,3,4},
                                {5,6,7,1},
                                {1,2,3,4},
                                {4,3,2,1}
                              };
    int led_strip_temp1[MAX_PIX];
    int led_strip_temp2[MAX_PIX];
    board_2_strip(board, led_strip_temp1, 1);
    board_2_strip(board, led_strip_temp2, 2);
    display_LEDStrip(led_strip_temp1, led_strip_temp2);
    delay(1000);
    

}

void loop() 
{
   allOff();
   STARTGAME = RESET;
   RESET = false;
   delay(200);
   while (!STARTGAME) {
      //IDLE
      STARTGAME = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
      if(!STARTGAME){
        STARTGAME = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
      }
      theaterChaseRainbow(25);
      //rainbow();
  }
  shipCnt[0] = 5;
  shipCnt[1] = 5; //************************************
  initialize();
  TURN = 1;
  start();
}

void start(){
  RESET = false;
  allOff();
  
  int cnt_ship_place = 0;
  while( cnt_ship_place < 3){
    if(!RESET){
      RESET = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
      if(!RESET){
        RESET = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
      }
      else{
        return;
      }
    }
    else{
        return;
    }
    if (cnt_ship_place == 0){
      place_ship_three(cnt_ship_place + 1);
    }
    else{
      place_ship_one(cnt_ship_place + 1);
    }
    delay(2000);
    cnt_ship_place++;
  }
  delay(3000);
  allOff();

  // Fire
  while( (!RESET ) &&((shipCnt[0] != 0) && (shipCnt[1] !=0)) ){
    place_bomb();
  }
  
  int cnt = 5;
  while((!RESET ) && ( cnt > 0)){
        cnt--;
        if(!RESET){
          RESET = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
          if(!RESET){
            RESET = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
          }
          else{
            break;
          }
        }
        else{
            break;
        }
        rainbow();           
        unsigned long start_time2 = millis();
        unsigned long max_time2 = 5000; // 1 sec = 1000
        unsigned long time_left2 = 5000; // 1 sec = 1000
        time_left2 = max_time2 - (millis() - start_time2);
        if (shipCnt[0] == 0) {
            copy_board(P1_gameboard, Empty_board);
            copy_board(P2_gameboard, P1_shipboard);
            display_gameboards();
          }
          else if (shipCnt[1] == 0){
            copy_board(P1_gameboard, P2_shipboard);
            copy_board(P2_gameboard, Empty_board);
            display_gameboards();
          }
          while((time_left2 /1000) > 0){
            if(!RESET){
              RESET = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
            }
            else{
              break;
            }
            time_left2 = max_time2 - (millis() - start_time2);
          }      
  }
  STARTGAME = false;
  return;
}

void initialize() {
  allOff();
  for(int i=0; i<X_MAX; i++) {
    for(int j=0; j<Y_MAX; j++) {
      // 0 - Off, 1 - R, 2 - G, 3 - B, 4 - White, 5 - Yellow, 6 - Light Blue, 7 - Purple, 8 - Grey
      P1_shipboard[i][j] = 0; // Shipboard  - 4 for ship (white), 0 for water (off), 1 for hit (red), 5 for miss (yellow), 8 for placed ship (grey), 7 for sunked ship (purple)
      P2_shipboard[i][j] = 0;
      P1_bombboard[i][j] = 0; // Bombboard  - 0 for water (off), 1 for hit (red), 5 for miss (yellow), 7 for sunked ship (purple)
      P2_bombboard[i][j] = 0;
      P1_gameboard[i][j] = 0;
      P2_gameboard[i][j] = 0;
      P1_tempboard[i][j] = 0;
      P2_tempboard[i][j] = 0;
      Empty_board[i][j] = 0;
    }
  }
  shipCnt[0] = 5;
  shipCnt[1] = 5;
  P1_rotate = false;
  P2_rotate = false;
  TURN = 1;
  allOff();
}

void three_ship_convertor(int P1_temp_h[4][2], int P2_temp_h[4][2], int P2_temp_v[2][4], int P1_temp_v[2][4]){
  copy_board(P1_tempboard, P1_shipboard);
  copy_board(P2_tempboard, P2_shipboard);
  if(P1_rotate){ // Horizontal
    for(int i=0; i<4; i++) {
      for(int j=0; j<2; j++) {
        if (P1_temp_h[i][j] == 4){
          P1_tempboard[i][j] = 4;
          P1_tempboard[i][j+1] = 4;
          P1_tempboard[i][j+2] = 4;
          break;
        }
      }
    }
  }
  else{ // verticle
    for(int i=0; i<2; i++) {
      for(int j=0; j<4; j++) {
        if (P1_temp_v[i][j] == 4){
          P1_tempboard[i][j] = 4;
          P1_tempboard[i+1][j] = 4;
          P1_tempboard[i+2][j] = 4;
          break;
        }
      }
    }
  }
  if(P2_rotate){ // Horizontal
    for(int i=0; i<4; i++) {
      for(int j=0; j<2; j++) {
        if (P2_temp_h[i][j] == 4){
          P2_tempboard[i][j] = 4;
          P2_tempboard[i][j+1] = 4;
          P2_tempboard[i][j+2] = 4;
          break;
        }
      }
    }
  }
  else{ // verticle
    for(int i=0; i<2; i++) {
      for(int j=0; j<4; j++) {
        if (P2_temp_v[i][j] == 4){
          P2_tempboard[i][j] = 4;
          P2_tempboard[i+1][j] = 4;
          P2_tempboard[i+2][j] = 4;
          break;
        }
      }
    }
  }
}

void place_ship_three(int ship_num) {
  allOff();
  P1_rotate = false;
  P2_rotate = false;
  bool board_blink = true;
  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);
  lcd.print("Place Ship  ");
  lcd.print(ship_num);
  int P1_temp_v[2][4] = { {0,0,0,0},
                       {0,0,0,0} };
  int P2_temp_v[2][4] = { {0,0,0,0},
                       {0,0,0,0} };
  int P2_temp_h[4][2] = { {0,0},
                       {0,0},
                       {0,0},
                       {0,0}};
  int P1_temp_h[4][2] = { {0,0},
                       {0,0},
                       {0,0},
                       {0,0}};
  unsigned long start_time = millis();
  unsigned long max_time = 20000; // 1 sec = 1000
  unsigned long time_left = 20000; // 1 sec = 1000
  unsigned long time_int = 20;
  unsigned long previous_time = start_time;
  unsigned long previous_time2 = start_time;
  time_left = max_time - (millis() - start_time);
  Serial.print("Start time: ");
  Serial.print("\n");
  Serial.print(start_time);
  Serial.print("\n");
  Serial.print("Time left: ");
  Serial.print("\n");
  Serial.print(time_left/1000);
  Serial.print("\n");
  char key_1, key_2;
  int key_num_1, key_num_2;
  bool placed[2] = {false,false};
  bool selected[2] = {false,false};
  copy_board(P1_gameboard, P1_shipboard);
  copy_board(P2_gameboard, P2_shipboard);
  display_gameboards();
  copy_board(P1_tempboard, P1_shipboard);
  copy_board(P2_tempboard, P2_shipboard);
  while((time_left /1000) > 0){
    // Break if Reset / Start button is pressed
    if(!RESET){
      RESET = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
      if(!RESET){
        RESET = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
      }
      else{
        return;
      }
    }
    else{
      return;
    }
    time_left = max_time - (millis() - start_time);
    writeTime(time_left);
    // if not placed, place it
    if (!placed[0]){
      P1_temp_v[0][0] = 4;
      P1_tempboard[0][0] = 4;
      P1_tempboard[1][0] = 4;
      P1_tempboard[2][0] = 4;
      placed[0] = true;
      //copy_board(P1_shipboard, P1_tempboard);
    }
    if (!placed[1]){
      P2_temp_v[0][0] = 4;
      P2_tempboard[0][0] = 4;
      P2_tempboard[1][0] = 4;
      P2_tempboard[2][0] = 4;
      placed[1] = true;
      //copy_board(P2_shipboard, P2_tempboard);
    }
    // Blinking / no blinking
    if ((millis() - previous_time) > 300){
      previous_time = millis();
      if (board_blink){
        copy_board(P1_gameboard, P1_shipboard);
        copy_board(P2_gameboard, P2_shipboard);
        display_gameboards();
      }
      else{
        copy_board(P1_gameboard, P1_tempboard);
        copy_board(P2_gameboard, P2_tempboard);
        display_gameboards();
      }
      board_blink = !board_blink;
    }      
      // Player 1 get key
    key_1 = keypad1.getKey();
    if (selected[0] == false){
      if(key_1){
        key_num_1 = char_to_num(key_1) - 1;
        if(P1_rotate){ // Horizontal
          if (P1_temp_h[key_num_1/4][(key_num_1 % 4)/2] == 0){
            for(int i=0; i<4; i++) {
              for(int j=0; j<2; j++) {
                P1_temp_h[i][j] = 0;
              }
            }
            P1_temp_h[key_num_1/4][(key_num_1 % 4)/2] = 4;
            copy_board(P1_tempboard, P1_shipboard);
            three_ship_convertor(P1_temp_h, P2_temp_h, P2_temp_v, P1_temp_v);
            placed[0] = true;
          }
        }
        else{ // verticle
          if (P1_temp_v[(key_num_1/4)/2][(key_num_1 % 4)] == 0){
            for(int i=0; i<2; i++) {
              for(int j=0; j<4; j++) {
                P1_temp_v[i][j] = 0;
              }
            }
            P1_temp_v[(key_num_1/4)/2][(key_num_1 % 4)] = 4;
            copy_board(P1_tempboard, P1_shipboard);
            three_ship_convertor(P1_temp_h, P2_temp_h, P2_temp_v, P1_temp_v);
            placed[0] = true;
          }
        }
      }
    }
    key_2 = keypad2.getKey();
    if (selected[1] == false){
      if(key_2){
        key_num_2 = char_to_num(key_2) - 1;
        if(P2_rotate){ // Horizontal
          if (P2_temp_h[key_num_2/4][(key_num_2 % 4)/2] == 0){
            for(int i=0; i<4; i++) {
              for(int j=0; j<2; j++) {
                P2_temp_h[i][j] = 0;
              }
            }
            P2_temp_h[key_num_2/4][(key_num_2 % 4)/2] = 4;
            copy_board(P2_tempboard, P2_shipboard);
            three_ship_convertor(P1_temp_h, P2_temp_h, P2_temp_v, P1_temp_v);
            placed[1] = true;
          }
        }
        else{ // verticle
          if (P2_temp_v[(key_num_2/4)/2][(key_num_2 % 4)] == 0){
            for(int i=0; i<2; i++) {
              for(int j=0; j<4; j++) {
                P2_temp_v[i][j] = 0;
              }
            }
            P2_temp_v[(key_num_2/4)/2][(key_num_2 % 4)] = 4;
            copy_board(P2_tempboard, P2_shipboard);
            three_ship_convertor(P1_temp_h, P2_temp_h, P2_temp_v, P1_temp_v);
            placed[1] = true;
          }
        }
      }
    }
    // Select
    if (switchToggled(selectButton1[0], &selectButton1[1], &selectButton1[2])) {
      if (placed[0]){
        selected[0] = true;
        copy_board(P1_shipboard, P1_tempboard);
      }
    }
    if (switchToggled(selectButton2[0], &selectButton2[1], &selectButton2[2])) {
      if (placed[1]){
        selected[1] = true;
        copy_board(P2_shipboard, P2_tempboard);
      }
    }
    //Rotate
    if (switchToggled(rotateButton1[0], &rotateButton1[1], &rotateButton1[2])) {
      if (!selected[0]){
        rotate(1);
      }
      if (!selected[0] && placed[0]){
        if(P1_rotate){ // V -> Horizontal
            P1_temp_h[0][0] = P1_temp_v[0][0];
            P1_temp_h[0][1] = P1_temp_v[0][3];
            P1_temp_h[1][0] = P1_temp_v[0][1];
            P1_temp_h[1][1] = P1_temp_v[0][2];
            P1_temp_h[2][0] = P1_temp_v[1][1];
            P1_temp_h[2][1] = P1_temp_v[1][2];
            P1_temp_h[3][0] = P1_temp_v[1][0];
            P1_temp_h[3][1] = P1_temp_v[1][3];
              for(int i=0; i<2; i++) {
                for(int j=0; j<4; j++) {
                  P1_temp_v[i][j] = 0;
                }
              }
          }
          else{ //H -> V
            P1_temp_v[0][0] = P1_temp_h[0][0];
            P1_temp_v[0][3] = P1_temp_h[0][1];
            P1_temp_v[0][1] = P1_temp_h[1][0];
            P1_temp_v[0][2] = P1_temp_h[1][1];
            P1_temp_v[1][1] = P1_temp_h[2][0];
            P1_temp_v[1][2] = P1_temp_h[2][1];
            P1_temp_v[1][0] = P1_temp_h[3][0];
            P1_temp_v[1][3] = P1_temp_h[3][1];
              for(int i=0; i<4; i++) {
                for(int j=0; j<2; j++) {
                  P1_temp_h[i][j] = 0;
                }
              }
          }
          copy_board(P1_tempboard, P1_shipboard);
          three_ship_convertor(P1_temp_h, P2_temp_h, P2_temp_v, P1_temp_v);
      }
    }
    if (switchToggled(rotateButton2[0], &rotateButton2[1], &rotateButton2[2])) {
      if (!selected[1]){
        rotate(2);
      }
      if (!selected[1] && placed[1]){
        if(P2_rotate){ // V -> Horizontal
            P2_temp_h[0][0] = P2_temp_v[0][0];
            P2_temp_h[0][1] = P2_temp_v[0][3];
            P2_temp_h[1][0] = P2_temp_v[0][1];
            P2_temp_h[1][1] = P2_temp_v[0][2];
            P2_temp_h[2][0] = P2_temp_v[1][1];
            P2_temp_h[2][1] = P2_temp_v[1][2];
            P2_temp_h[3][0] = P2_temp_v[1][0];
            P2_temp_h[3][1] = P2_temp_v[1][3];
              for(int i=0; i<2; i++) {
                for(int j=0; j<4; j++) {
                  P2_temp_v[i][j] = 0;
                }
              }
          }
          else{ //H -> V
            P2_temp_v[0][0] = P2_temp_h[0][0];
            P2_temp_v[0][3] = P2_temp_h[0][1];
            P2_temp_v[0][1] = P2_temp_h[1][0];
            P2_temp_v[0][2] = P2_temp_h[1][1];
            P2_temp_v[1][1] = P2_temp_h[2][0];
            P2_temp_v[1][2] = P2_temp_h[2][1];
            P2_temp_v[1][0] = P2_temp_h[3][0];
            P2_temp_v[1][3] = P2_temp_h[3][1];
              for(int i=0; i<4; i++) {
                for(int j=0; j<2; j++) {
                  P2_temp_h[i][j] = 0;
                }
              }
          }
          copy_board(P1_tempboard, P1_shipboard);
          three_ship_convertor(P1_temp_h, P2_temp_h, P2_temp_v, P1_temp_v);
      }
    }
  }
  // if placed but not selected, select
  if (placed[0] && !selected[0]){
    selected[0] = true;
    copy_board(P1_shipboard, P1_tempboard);
  }
  if (placed[1] && !selected[1]){
    selected[1] = true;
    copy_board(P2_shipboard, P2_tempboard);
  }
  // if not placed, place it
    if (!placed[0]){
      P1_tempboard[0][0] = 4;
      P1_tempboard[1][0] = 4;
      P1_tempboard[2][0] = 4;
      copy_board(P1_shipboard, P1_tempboard);
    }
    if (!placed[1]){
      P2_tempboard[0][0] = 4;
      P2_tempboard[1][0] = 4;
      P2_tempboard[2][0] = 4;
      copy_board(P2_shipboard, P2_tempboard);
    }
  display_gameboards();
}



void place_bomb(){
  allOff();
  if (TURN == 1) {
      TURN = 2;
      lcd.begin(16, 2);
      lcd.setRGB(0, 255,0);
      lcd.print("Player 1 Fire");
  }
  else if (TURN == 2) {
      TURN = 1;
      lcd.begin(16, 2);
      lcd.setRGB(0, 255,0);
      lcd.print("Player 2 Fire");
  }
  unsigned long start_time = millis();
  unsigned long max_time = 10000; // 1 sec = 1000
  unsigned long time_left = 10000; // 1 sec = 1000
  unsigned long time_int = 10;
  time_left = max_time - (millis() - start_time);
  Serial.print("Start time: ");
  Serial.print("\n");
  Serial.print(start_time);
  Serial.print("\n");
  Serial.print("Time left: ");
  Serial.print("\n");
  Serial.print(time_left/1000);
  Serial.print("\n");
  char key_1, key_2;
  int key_num_1, key_num_2;
  bool placed = false;
  while((time_left /1000) > 0){
    // Break if Reset / Start button is pressed
    if(!RESET){
      RESET = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
      if(!RESET){
        RESET = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
      }
      else{
        return;
      }
    }
    else{
      return;
    }
    if(time_left != 0){
      time_left = max_time - (millis() - start_time);
      writeTime(time_left);
      if ( (time_left /1000) < time_int){
        time_int = time_left /1000;
        Serial.print("Time left: ");
        Serial.print("\n");
        Serial.print(time_left/1000);
        Serial.print("\n");
      }
    }
    // TURN
    if (TURN == 2) {
      // Update gameboard
      copy_board(P1_gameboard, P2_bombboard);
      copy_board(P2_gameboard, P2_shipboard);
      display_gameboards();
      // Player 1 get key
      key_1 = keypad1.getKey();
      if(key_1){
        key_num_1 = char_to_num(key_1) - 1;
        Serial.println(String(key_num_1));
        Serial.println("\n");
        if (P2_shipboard[key_num_1 / 4][key_num_1 % 4] == 4){
          //hit
          time_left = 0;
          P2_shipboard[key_num_1 / 4][key_num_1 % 4] = 1;
          P2_bombboard[key_num_1 / 4][key_num_1 % 4] = 1;
          placed = true;
          copy_board(P1_gameboard, P2_bombboard);
          copy_board(P2_gameboard, P2_shipboard);
          display_gameboards();
          hit();
          shipCnt[1]--;
          return;
        }
        else if (P2_shipboard[key_num_1 / 4][key_num_1 % 4] == 0){
          //miss
          time_left = 0;
          P2_shipboard[key_num_1 / 4][key_num_1 % 4] = 5;
          P2_bombboard[key_num_1 / 4][key_num_1 % 4] = 5;
          placed = true;
          
          // else (hit or miss already, do nothing)
          copy_board(P1_gameboard, P2_bombboard);
          copy_board(P2_gameboard, P2_shipboard);
          display_gameboards();
          miss();
          return;
        }
      }   
    }
    else if (TURN == 1){
      // Update gameboard
      copy_board(P1_gameboard, P1_shipboard);
      copy_board(P2_gameboard, P1_bombboard);
      display_gameboards();
      // Player 1 get key
      key_2 = keypad2.getKey();
      if(key_2){
        key_num_2 = char_to_num(key_2) - 1;
        Serial.println(String(key_num_2));
        Serial.println("\n");
        if (P1_shipboard[key_num_2 / 4][key_num_2 % 4] == 4){
          //ship - hit
          time_left = 0;
          P1_shipboard[key_num_2 / 4][key_num_2 % 4] = 1;
          P1_bombboard[key_num_2 / 4][key_num_2 % 4] = 1;
          placed = true;
          copy_board(P1_gameboard, P1_shipboard);
          copy_board(P2_gameboard, P1_bombboard);
          display_gameboards();
          hit();
          shipCnt[0]--;
          return;
        }
        else if (P1_shipboard[key_num_2 / 4][key_num_2 % 4] == 0){
          //miss
          time_left = 0;
          P1_shipboard[key_num_2 / 4][key_num_2 % 4] = 5;
          P1_bombboard[key_num_2 / 4][key_num_2 % 4] = 5;
          placed = true;
          
          // else (hit or miss already, do nothing)
          copy_board(P1_gameboard, P1_shipboard);
          copy_board(P2_gameboard, P1_bombboard);
          display_gameboards();
          miss();
          return;
         }
      }
    }
    else {
      Serial.println("Wrong Turn value for bomb placement");
    }
  }// endWhile
  if (!placed) {
    // Miss
    if (TURN == 2) { //P1 switched turn
      for(int i=0; i<16; i++) {
        if (P2_shipboard[i/4][i%4] == 0){
          //miss
          P2_shipboard[i/4][i%4] = 5;
          P2_bombboard[i/4][i%4]= 5;
          placed = true;
          copy_board(P1_gameboard, P2_bombboard);
          copy_board(P2_gameboard, P2_shipboard);
          display_gameboards();
          miss();
          return;
        }
      }
    }
    else{
      for(int i=0; i<16; i++) {
        if (P1_shipboard[i/4][i%4] == 0){
          //miss
          P1_shipboard[i/4][i%4] = 5;
          P1_bombboard[i/4][i%4]= 5;
          placed = true;
          copy_board(P1_gameboard, P1_shipboard);
          copy_board(P2_gameboard, P1_bombboard);
          display_gameboards();
          miss();
          return;
        }
      }
    }
  }
  if (!placed) {
    //Hit
    if (TURN == 2) {
      for(int i=0; i<16; i++) {
        if (P2_shipboard[i/4][i%4] == 4){
          //miss
          P2_shipboard[i/4][i%4] = 1;
          P2_bombboard[i/4][i%4]= 1;
          placed = true;
          copy_board(P1_gameboard, P2_bombboard);
          copy_board(P2_gameboard, P2_shipboard);
          display_gameboards();
          hit();
          shipCnt[1]--;
          return;
        }
      }
    }
    else{
      for(int i=0; i<16; i++) {
        if (P1_shipboard[i/4][i%4] == 4){
          //miss
          P1_shipboard[i/4][i%4] = 1;
          P1_bombboard[i/4][i%4]= 1;
          placed = true;
          copy_board(P1_gameboard, P1_bombboard);
          copy_board(P2_gameboard, P1_shipboard);
          display_gameboards();
          hit();
          shipCnt[0]--;
          return;
        }
      }
    }
  }
} // end function


void writeTime(unsigned long longtime){
  // convert time from ms to s as an integer
  // don't bother rounding, too much effort
  int inttime = (int)(longtime/1000);
  // write the time to second line of lcd screen
  lcd.setCursor(0, 1);
  lcd.print(inttime);
  lcd.print(" seconds left");
}


int P1_strip_translation[32] = {5,5,5,5,5,5,5,5,5,5,5,4,5,5,5,5,4,5,5,6,5,5,5,5,4,5,5,6,5,5,5,5};
int P1_board_numbering[32] = {16,15,14,13,13,14,15,16,12,11,10,9,9,10,11,12,8,7,6,5,5,6,7,8,4,3,2,1,1,2,3,4};
int P2_strip_translation[32] = {4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};
int P2_board_numbering[32] = {13,14,15,16,16,15,14,13,9,10,11,12,12,11,10,9,5,6,7,8,8,7,6,5,1,2,3,4,4,3,2,1};
void board_2_strip(int board[X_MAX][Y_MAX], int led_strip_temp[MAX_PIX], int player_num) {
  // player_num = 1 or 2
  for(int i=0; i<max_pix; i++) {
    led_strip_temp[i] = 0;
  }

  int strip_translation[32];
  int board_numbering[32];  if (player_num == 1){
    for(int i=0; i<32; i++) {
      strip_translation[i] =  P1_strip_translation[i];
      board_numbering [i]  =  P1_board_numbering[i];
    }
  }
  else if (player_num == 2){
    for(int i=0; i<32; i++) {
      strip_translation[i] =  P2_strip_translation[i];
      board_numbering [i]  =  P2_board_numbering[i];
    }
  }
  int cnt = 0;
  for(int j=0; j<32; j++) {
      int num_per_board = strip_translation[j];
      int board_num = board_numbering[j] - 1;
      for(int i=0; i<num_per_board; i++) {
          led_strip_temp[cnt] = board[board_num / 4][board_num % 4];
          cnt++;
      }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);
  lcd.print("  Press Start");
  lcd.setCursor(0,1);
  lcd.print("  To Begin");
  
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    //if (val == HIGH){
    if (STARTGAME){
      break;
    }
    for (int q=0; q < 1; q++) {
      if (STARTGAME){
        return;
      }
      STARTGAME = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
      if(!STARTGAME){
        STARTGAME = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
      }
      if (STARTGAME){
        return;
      }
      for (uint16_t i=0; i < strip_1.numPixels(); i=i+1) {
        strip_1.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip_1.show();
      //delay(wait);
      for (uint16_t i=0; i < strip_1.numPixels(); i=i+1) {
        strip_1.setPixelColor(i+q, 0);        //turn every third pixel off
      }
      for (uint16_t i=0; i < strip_2.numPixels(); i=i+1) {
        strip_2.setPixelColor(i+q, Wheel2( (i+j) % 255));    //turn every third pixel on
      }
      strip_2.show();
      delay(wait);
      for (uint16_t i=0; i < strip_2.numPixels(); i=i+1) {
        strip_2.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Turn off all leds 
void allOff() {
  for(uint32_t i=0; i<strip_1.numPixels(); i++) {
    strip_1.setPixelColor(i, strip_1.Color(0, 0, 0));   
  }
  for(uint32_t i=0; i<strip_2.numPixels(); i++) {
    strip_2.setPixelColor(i, strip_2.Color(0, 0, 0));
    
  }
  strip_1.show();
  strip_2.show();
}

// Fill the dots one after the other with a color
void display_LEDStrip(int led_strip1[MAX_PIX],int led_strip2[MAX_PIX]) {
  for(uint16_t i=0; i<MAX_PIX; i++) {
    strip_1.setPixelColor(i, c_rgb1[led_strip1[i]]); 
  }
  for(uint16_t i=0; i<MAX_PIX; i++) {
    strip_2.setPixelColor(i, c_rgb2[led_strip2[i]]); 
  }
  strip_1.show(); 
  strip_2.show(); 
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip_1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip_1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip_1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
uint32_t Wheel2(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip_2.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip_2.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip_2.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

int char_to_num(char key) {
  //
  if (key == 'A'){
    return 1;
  }
  if (key == 'B'){
    return 2;
  }
  if (key == 'C'){
    return 3;
  }
  if (key == 'D'){
    return 4;
  }
  if (key == '3'){
    return 5;
  }
  if (key == '6'){
    return 6;
  }
  if (key == '9'){
    return 7;
  }
  if (key == '#'){
    return 8;
  }
  if (key == '2'){
    return 9;
  }
  if (key == '5'){
    return 10;
  }
  if (key == '8'){
    return 11;
  }
  if (key == '0'){
    return 12;
  }
  if (key == '1'){
    return 13;
  }
  if (key == '4'){
    return 14;
  }
  if (key == '7'){
    return 15;
  }
  if (key == '*'){
    return 16;
  }
  return 0;
};

void hit(){
  lcd.begin(16, 2);
  lcd.setRGB(255, 0, 0);
  lcd.clear();
  lcd.print("     Hit!");
  int three_sec = 3000;
  while(three_sec > 0 ){
    if (switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2])) {
      Serial.print(startResetButton1[0]);
      RESET = true;
      break;
    }   
    delay(300);
    three_sec = three_sec - 300;
  }
}
void miss(){
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 0);
  lcd.clear();
  lcd.print("     Miss!");
  int three_sec = 3000;
  while(three_sec > 0 ){
    if (switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2])) {
      Serial.print(startResetButton1[0]);
      RESET = true;
      break;
    }    
      delay(300);
      three_sec = three_sec - 300;
  }
}
void winner(int winner_num){
  
  if (winner_num == 1){
    lcd.begin(16, 2);
    lcd.setRGB(0, 255, 255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Winner is:");
    lcd.setCursor(0, 1);
    lcd.print("Player 1!!!");
  }
  if (winner_num == 2){
    lcd.begin(16, 2);
    lcd.setRGB(0, 255, 255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Winner is:");
    lcd.setCursor(0, 1);
    lcd.print("Player 2!!!");
  }
  int three_sec = 3000;
  while(three_sec > 0 ){
    if (switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2])) {
      Serial.print(startResetButton1[0]);
      RESET = true;
      break;
    }    
      delay(300);
      three_sec = three_sec - 300;
  }
}

/*
 * Instantiates the initial state of the switch (think of an RC capacitor)
 */
void initializeButton(unsigned long *buttonObject, unsigned long pinID){
  buttonObject[0] = pinID;
  buttonObject[1] = 0;
  buttonObject[2] = millis();
  //note: default value is HIGH because of the INPUT_PULLUP state
  Serial.print(pinID);
  pinMode(pinID, INPUT_PULLUP);
}

bool switchToggled(unsigned long switchPin, unsigned long* lastValue, unsigned long* lastActivated){
  unsigned long now = millis();
  //Serial.print(switchPin);
  int currentValue = digitalRead(switchPin) == HIGH ? 0 : 1;
  if( (now - *lastActivated) < 200){
    return false;
    
  }
  if(*lastValue != currentValue){
    // inverted because it's pulled HIGH by default
    *lastValue = currentValue;
    *lastActivated = now;
    return currentValue;
  }
  return false;
}

void rainbow() {
  lcd.begin(16, 2);
  lcd.setRGB(255, 0, 255);
  lcd.print("  Winner is:");
  lcd.setCursor(0,1);
  if (shipCnt[0] == 0) {
    lcd.print("  Player 2!");
  }
  else if (shipCnt[1] == 0){
    lcd.print("  Player 1!");
  }
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<MAX_PIX; i++) {
      if (RESET){
        return;
      }
      RESET = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
      if(!RESET){
        RESET = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
      }
      if (RESET){
        return;
      }
      strip_1.setPixelColor(i, Wheel((i+j) & 255));
      strip_2.setPixelColor(i, Wheel2((i+j) & 255));
    }
    if (shipCnt[0] == 0) {
      strip_2.show();
    }
    else if (shipCnt[1] == 0) {
      strip_1.show();
    }
    delay(1);
  }
}

void copy_board(int dst_board[X_MAX][Y_MAX], int src_board[X_MAX][Y_MAX]){
  for(int i=0; i<X_MAX; i++) {
    for(int j=0; j<Y_MAX; j++) {
      dst_board[i][j] = src_board[i][j];
    }
  }
}

void display_gameboards() {
    int led_strip_temp1[MAX_PIX];
    int led_strip_temp2[MAX_PIX];
    board_2_strip(P1_gameboard, led_strip_temp1, 1);
    board_2_strip(P2_gameboard, led_strip_temp2, 2);
    display_LEDStrip(led_strip_temp1, led_strip_temp2);
}

void rotate(int player_num){
  if (player_num == 1){
    P1_rotate = !P1_rotate;
    Serial.print("Player 1 rotate");
    Serial.print(P1_rotate);
    Serial.print("\n");
  }
  if (player_num == 2){
    P2_rotate = !P2_rotate;
    Serial.print("Player 2 rotate");
    Serial.print(P2_rotate);
    Serial.print("\n");
  }
}

void place_ship_one(int ship_num) {
  allOff();
  P1_rotate = false;
  P2_rotate = false;
  bool board_blink = true;
  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);
  lcd.print("Place Ship  ");
  lcd.print(ship_num);
  unsigned long start_time = millis();
  unsigned long max_time = 15000; // 1 sec = 1000
  unsigned long time_left = 15000; // 1 sec = 1000
  unsigned long time_int = 15;
  unsigned long previous_time = start_time;
  unsigned long previous_time2 = start_time;
  time_left = max_time - (millis() - start_time);
  Serial.print("Start time: ");
  Serial.print("\n");
  Serial.print(start_time);
  Serial.print("\n");
  Serial.print("Time left: ");
  Serial.print("\n");
  Serial.print(time_left/1000);
  Serial.print("\n");
  char key_1, key_2;
  int key_num_1, key_num_2;
  bool placed[2] = {false,false};
  bool selected[2] = {false,false};
  copy_board(P1_gameboard, P1_shipboard);
  copy_board(P2_gameboard, P2_shipboard);
  display_gameboards();
  copy_board(P1_tempboard, P1_shipboard);
  copy_board(P2_tempboard, P2_shipboard);
  while((time_left /1000) > 0){
    // Break if Reset / Start button is pressed
    if(!RESET){
      RESET = switchToggled(startResetButton2[0], &startResetButton2[1], &startResetButton2[2]);
      if(!RESET){
        RESET = switchToggled(startResetButton1[0], &startResetButton1[1], &startResetButton1[2]);
      }
      else{
        return;
      }
    }
    else{
      return;
    }
    time_left = max_time - (millis() - start_time);
    writeTime(time_left);

    // if not placed, place it
    if (!placed[0]){
      for(int i=0; i<16; i++) {
          if (P1_shipboard[i/4][i%4] == 0){
            P1_tempboard[i/4][i%4] = 4;
            placed[0] = true;
            break;
          }
      }
    }
    if (!placed[1]){
      for(int i=0; i<16; i++) {
          if (P2_shipboard[i/4][i%4] == 0){
            P2_tempboard[i/4][i%4] = 4;
            placed[1] = true;
            break;
          }
      }
    }

    
    // Blinking / no blinking
    if ((millis() - previous_time) > 300){
      previous_time = millis();
      if (board_blink){
        copy_board(P1_gameboard, P1_shipboard);
        copy_board(P2_gameboard, P2_shipboard);
        display_gameboards();
      }
      else{
        copy_board(P1_gameboard, P1_tempboard);
        copy_board(P2_gameboard, P2_tempboard);
        display_gameboards();
      }
      board_blink = !board_blink;
    }      
      // Player 1 get key
    key_1 = keypad1.getKey();
    if (selected[0] == false){
      if(key_1){
        key_num_1 = char_to_num(key_1) - 1;
        if (P1_shipboard[key_num_1 / 4][key_num_1 % 4] == 0){
          // placed (not selected)
          copy_board(P1_tempboard, P1_shipboard);
          P1_tempboard[key_num_1 / 4][key_num_1 % 4] = 4;
          placed[0] = true;
        }
      }
    }
    key_2 = keypad2.getKey();
    if (selected[1] == false){
      if(key_2){
        key_num_2 = char_to_num(key_2) - 1;
        if (P2_shipboard[key_num_2 / 4][key_num_2 % 4] == 0){
          // placed (not selected)
          copy_board(P2_tempboard, P2_shipboard);
          P2_tempboard[key_num_2 / 4][key_num_2 % 4] = 4;
          placed[1] = true;
        }
      }
    }
    // Select
    if (switchToggled(selectButton1[0], &selectButton1[1], &selectButton1[2])) {
      if (placed[0]){
        selected[0] = true;
        copy_board(P1_shipboard, P1_tempboard);
      }
    }
    if (switchToggled(selectButton2[0], &selectButton2[1], &selectButton2[2])) {
      if (placed[1]){
        selected[1] = true;
        copy_board(P2_shipboard, P2_tempboard);
      }
    }
    //Rotate
    if (switchToggled(rotateButton1[0], &rotateButton1[1], &rotateButton1[2])) {
      if (placed[0]){
        rotate(1);
      }
    }
    if (switchToggled(rotateButton2[0], &rotateButton2[1], &rotateButton2[2])) {
      if (placed[1]){
        rotate(2);
      }
    }
  }
  // if placed but not selected, select
  if (placed[0] && !selected[0]){
    selected[0] = true;
    copy_board(P1_shipboard, P1_tempboard);
  }
  if (placed[1] && !selected[1]){
    selected[1] = true;
    copy_board(P2_shipboard, P2_tempboard);
  }
  
  display_gameboards();
}

