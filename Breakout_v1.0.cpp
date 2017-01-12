/**
  Vishal Patel, Max Scales - EA-1

  Breaker
  v.1.DONE

*/
  #include <Adafruit_GFX.h>    // Core graphics library
  #include <Adafruit_ST7735.h> // Hardware-specific library
  #include <SPI.h>
  #include <SD.h>
  #include <Arduino.h>
  #include <EEPROM.h>
  #include "stack.h"

  #include "lcd_image.h"

  // standard U of A library settings, assuming Atmel Mega SPI pins
  #define SD_CS    5  // Chip select line for SD card
  #define TFT_CS   6  // Chip select line for TFT display
  #define TFT_DC   7  // Data/command line for TFT
  #define TFT_RST  8  // Reset line for TFT (or connect to +5V)
  #define JOY_SEL 9
  #define JOY_VERT_ANALOG 0
  #define JOY_HORIZ_ANALOG 1

  #define TFT_WIDTH 128
  #define TFT_HEIGHT 160

  #define GAMEBOARD_HEIGHT 152
  #define PADDLE_HEIGHT 150

  #define JOY_DEADZONE 64 // Only care about joystick movement if
                          // position is JOY_CENTRE +/- JOY_DEADZONE

  #define MILLIS_PER_FRAME 25 // 40fps

  #define STEPS_PER_PIXEL 256

  //some RGB color definitions          */
  #define Black           0x0000      /*   0,   0,   0 */
  #define Navy            0x1A34     /*   0,   0, 128 */
  #define DarkGreen       0x03E0      /*   0, 128,   0 */
  #define DarkCyan        0x03EF      /*   0, 128, 128 */
  #define Maroon          0xC827      /* 128,   0,   0 */
  #define Purple          0x780F      /* 128,   0, 128 */
  #define Olive           0x7BE0      /* 128, 128,   0 */
  #define LightGrey       0xC618      /* 192, 192, 192 */
  #define DarkGrey        0x7BEF      /* 128, 128, 128 */
  #define Blue            0x001F      /*   0,   0, 255 */
  #define Green           0x07E0      /*   0, 255,   0 */
  #define Cyan            0x07FF      /*   0, 255, 255 */
  #define Red             0xF800      /* 255,   0,   0 */
  #define Magenta         0xF81F      /* 255,   0, 255 */
  #define Yellow          0xFFE0      /* 255, 255,   0 */
  #define White           0xFFFF      /* 255, 255, 255 */
  #define Orange          0xFD20      /* 255, 165,   0 */
  #define GreenYellow     0xAFE5      /* 173, 255,  47 */
  #define Pink            0xF81F
  #define LightOrange     0xFD80
  #define Aqua            0x3e19

  int colours[18] = {Navy, DarkGreen, DarkCyan, Maroon, Purple, Olive,
  Blue, Green, Cyan, Red, Magenta, Yellow, White, Orange,
  GreenYellow, Pink, LightOrange, Aqua};

  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

  lcd_image_t title_image = {"title.lcd", 128, 160};

  struct Bricks {
    int rectX;
    int rectY;
    int length;
    int colour;
  };

  //Added: a brick structure.
  Bricks bricks[16];
  int numBricks = 16;

  //Updated: Gave some constants names.
  int g_joyX = TFT_WIDTH/2; // X-position of cursor in pixels
  int g_joyY = PADDLE_HEIGHT; // Y-position of cursor in pixels
  int JOY_CENTRE;
  int g_cursorX = TFT_WIDTH/2; // Drawn cursor position
  int g_cursorY = PADDLE_HEIGHT;

  //Updated: Renamed initBallx/y to drawnBallx/y for clarity.
  //int drawnBallx = (TFT_WIDTH/2);
  int drawnBallx = 64;
  int drawnBally = 80;

  //Added: Keeps track of the fractional position of the ball.
  //float ballx = (TFT_WIDTH/2);
  float ballx = 64;
  float bally = 80;

  float ballXSpeed = 0;
  float ballYSpeed = 1;

  //For highscores
  bool defeatstate = false;
  int score = 0;
  char initial1;
  char initial2;
  char initial3;
  Stack highscore;
  Stack temp;

  void scanJoystick();
  void MoveLeft();
  void MoveRight();
  void spawnpaddle();
  void initializeGameBoard();
  void generateBricks(int yLevel);
  void destroyBrick(int xbrick, int ybrick);
  void ballTrajectory(int trajectory);
  void Score();
  void displayScore();
  void addAndUpdateScore();
  void instructions();
  void WriteText(String Text, int x, int y, int size,int color);
  void WriteText(String Text, int x, int y, int size, int color, int color2);
  void GameOver();
  void letterSelection();
  void updateSelection(int xCoordinate, int yCoordinate, char letterSelected, int color, int color2);
  void highscoreTable();

  void setup(void) {
    init();

    Serial.begin(9600);

    tft.initR(INITR_BLACKTAB);   // initialize a ST7735R chip, black tab
    tft.fillScreen(Black);

    Serial.print("Initializing SD card...");
    if (!SD.begin(SD_CS)) {
      Serial.println("failed!");
      return;
    }
    Serial.println("OK!");

    //Init joystick
    pinMode(JOY_SEL, INPUT);
    digitalWrite(JOY_SEL, HIGH); // enables pull-up resistor
    Serial.println("initialized!");
    JOY_CENTRE = analogRead(JOY_VERT_ANALOG);

  }

  //Added: A variation of WriteText, called through function overloading.
  void WriteText(String Text, int x, int y, int size, int color, int color2){
    tft.setCursor(x,y);
    tft.setTextColor(color, color2);
    tft.setTextSize(size);
    tft.print(Text);
  }

  //Vishal
  void updateBall() {
    tft.fillCircle(drawnBallx,drawnBally,3,ST7735_BLACK);
    ballx += ballXSpeed;
    bally += ballYSpeed;
    drawnBallx = (int)ballx;
    drawnBally = (int)bally;
    tft.fillCircle(drawnBallx,drawnBally,3,ST7735_WHITE);
  }

  //Added: Calculates the ball trajectory
  //Vishal
  void ballTrajectory(int trajectory) {

    if (trajectory == 1) { //0 degrees
      ballXSpeed = 0;
      ballYSpeed = -1;
    } else if (trajectory == 2) { // (+/-) 6.64 dregrees
      ballXSpeed = (float)1 / (float)9;
      ballYSpeed = -1;
    } else if (trajectory == 3) { // (+/-) 14.03 dregrees
      ballXSpeed = (float)2 / (float)8;
      ballYSpeed = -1;
    } else if (trajectory == 4) { // (+/-) 23.20 dregrees
      ballXSpeed = (float)3 / (float)7;
      ballYSpeed = -1;
    } else if (trajectory == 5) { // (+/-) 33.69 dregrees
      ballXSpeed = (float)4 / (float)6;
      ballYSpeed = -1;
    } else if (trajectory == 6) { //45 degrees
      ballXSpeed = 1;
      ballYSpeed = -1;
    }

  }

  //Added: Changes the current trajectory based on where the paddle is hit.
  //Vishal
  int changeTrajectory(int offset) {

    //Paddle is 19 pixels long, there is 1 center pixel and 9 pixels on
    //each side.
    if (offset == 0) {
      return 1;
    } else if (offset == 1) {
      return 2;
    } else if ((offset == 2) || (offset == 3)) {
      return 3;
    } else if ((offset == 4) || (offset == 5)) {
      return 4;
    } else if ((offset == 6) || (offset == 7)) {
      return 5;
    } else if ((offset == 8) || (offset == 9)) {
      return 6;
    }

  }

  //Added brick destruction
  //Vishal
  void destroyBrick(int i) {

    Bricks temp;

    tft.fillRect(bricks[i].rectX, bricks[i].rectY, bricks[i].length, 5, Black);

    for (int j = i; j < numBricks-1; j++) {
      temp = bricks[j+1];
      bricks[j] = temp;
    }

    numBricks -= 1;

    addAndUpdateScore();

  }


  //Updated: Calls destroyBrick when a brick is hit, checks all sides of a brick.
  //Unit collision detection for the paddle, walls and bricks
  //Vishal
  void unitCollisionDetection() {

    int brickXValue = 0;
    int trajectory = 0;

    if ((drawnBallx == 3) || (drawnBallx == 125)) {
      ballXSpeed = -ballXSpeed;
    }

    for (int i = 0; i < numBricks; i++) {

      if ((((drawnBally) <= bricks[i].rectY+5) && (drawnBally >= bricks[i].rectY)) && ((drawnBallx+3 == bricks[i].rectX)
      || (drawnBallx-3 == (bricks[i].rectX + bricks[i].length)))) {

        destroyBrick(i);
        ballXSpeed = -ballXSpeed;
        break;

      }

      if ((((drawnBally-3) == bricks[i].rectY+5) || (drawnBally+3 == bricks[i].rectY)) && (((drawnBallx) >= bricks[i].rectX-1)
      && ((drawnBallx) <= (bricks[i].rectX + bricks[i].length + 1)))) {

        destroyBrick(i);

        if ((((drawnBallx-1) == bricks[i].rectX) || ((drawnBallx+1) == (bricks[i].rectX + bricks[i].length)))
            && ((drawnBally-3) == bricks[i].rectY+5) || (drawnBally+3 == bricks[i].rectY)) {
            ballYSpeed = -ballYSpeed;
            //Serial.println("Edge case");
        } else {
            ballYSpeed = -ballYSpeed;
            //Serial.println("top/bot case");
        }
        break;

      }

      //Upper-right corner
      //{0, 32, 64, 96}
      if ((((drawnBallx+2) == bricks[i].rectX) && ((drawnBally-2) == bricks[i].rectY+5))
      || (((drawnBallx+3) == bricks[i].rectX) && ((drawnBally-1) == bricks[i].rectY+5))) {

        destroyBrick(i);

        //Serial.println("upperleft corner");

        ballXSpeed = -ballXSpeed;
        ballYSpeed = -ballYSpeed;

        break;

      }

      //Upper-left corner
      //{0, 32, 64, 96}
      if ((((drawnBallx-2) == (bricks[i].rectX + bricks[i].length)) && ((drawnBally-2) == bricks[i].rectY+5))
      || (((drawnBallx-3) == (bricks[i].rectX + bricks[i].length)) && ((drawnBally-1) == bricks[i].rectY+5))) {

        destroyBrick(i);

        //Serial.println("upperright corner");

        ballXSpeed = -ballXSpeed;
        ballYSpeed = -ballYSpeed;

        break;

      }

      //lower-right corner
      //{0, 32, 64, 96}
      if ((((drawnBallx+2) == bricks[i].rectX) && ((drawnBally+2) == bricks[i].rectY))
      || (((drawnBallx+3) == bricks[i].rectX) && ((drawnBally+1) == bricks[i].rectY))) {

        destroyBrick(i);

        //Serial.println("lowerright corner");

        ballXSpeed = -ballXSpeed;
        ballYSpeed = -ballYSpeed;

        break;

      }

      //lowerleft corner
      //{0, 32, 64, 96}
      if ((((drawnBallx-2) == (bricks[i].rectX + bricks[i].length)) && ((drawnBally+2) == bricks[i].rectY))
      || (((drawnBallx-3) == (bricks[i].rectX + bricks[i].length)) && ((drawnBally+1) == bricks[i].rectY))) {

        destroyBrick(i);

        //Serial.println("upperright corner");

        ballXSpeed = -ballXSpeed;
        ballYSpeed = -ballYSpeed;

        break;

      }

    }

    //Added game over
    if (drawnBally == GAMEBOARD_HEIGHT-3) {
      defeatstate = true;
    }

    if (drawnBally == 3) {
      ballYSpeed = -ballYSpeed;
    }

    if ((drawnBallx >= (g_cursorX - 9) && (drawnBallx <= (g_cursorX + 9))) && (drawnBally == g_joyY-4)){
        ballYSpeed = -ballYSpeed;
        trajectory = changeTrajectory(abs(drawnBallx - g_cursorX));
        ballTrajectory(trajectory);
        if ((drawnBallx - g_cursorX) <= 0) {
          ballXSpeed = -ballXSpeed;
        }
    }

  }

  //Spawns ball at position...
  //Max
  void spawnball() {
    tft.fillCircle(drawnBallx,drawnBally,3,ST7735_WHITE);
  }

  //Spawns paddle at position...
  //Max
  void spawnpaddle() {
    tft.drawRect(g_joyX-9, g_joyY, 19, 2, ST7735_WHITE);
  }

  //Added paddleMovement left
  //Max
  void MoveLeft() {

    tft.drawPixel(g_cursorX+10, g_cursorY, ST7735_BLACK);
    tft.drawPixel(g_cursorX+10, g_cursorY+1, ST7735_BLACK);

    tft.drawPixel(g_joyX-9, g_joyY, ST7735_WHITE);
    tft.drawPixel(g_joyX-9, g_joyY+1, ST7735_WHITE);


    g_cursorX = g_joyX;
    g_cursorY = g_joyY;
    }

  //Added paddleMovement right
  //Max
  void  MoveRight() {
    tft.drawPixel(g_cursorX-10, g_cursorY, ST7735_BLACK);
    tft.drawPixel(g_cursorX-10, g_cursorY+1, ST7735_BLACK);

    tft.drawPixel(g_joyX+9, g_joyY, ST7735_WHITE);
    tft.drawPixel(g_joyX+9, g_joyY+1, ST7735_WHITE);


    g_cursorX = g_joyX;
    g_cursorY = g_joyY;
  }

  //Updated: Re/Initialize the gameboard.
  //Vishal
  void initializeGameBoard() {

    Serial.print("InitializeGameBoard");
    tft.fillScreen(Black);

    g_joyX = TFT_WIDTH/2; // X-position of cursor in pixels
    g_joyY = PADDLE_HEIGHT; // Y-position of cursor in pixels
    g_cursorX = TFT_WIDTH/2; // Drawn cursor position
    g_cursorY = PADDLE_HEIGHT;
    numBricks = 0;

    //Updated: Renamed initBallx/y to drawnBallx/y for clarity.
    drawnBallx = (TFT_WIDTH/2);
    drawnBally = 140;

    //Added: Keeps track of the fractional position of the ball.
    ballx = 64;
    bally = 140;

    ballXSpeed = 0;
    ballYSpeed = 1;

    //numBricks = 16;

    for (int i = 0; i < 16; i++) {
      bricks[i].rectX = 0;
      bricks[i].rectY = 0;
      bricks[i].colour = 0;
      bricks[i].length = 0;
    }

    spawnball();
    spawnpaddle();
    Score();
    displayScore();

    generateBricks(0);
    generateBricks(5);
    generateBricks(10);
    generateBricks(15);
    for (int i = 0; i < 16; i++) {
      tft.fillRect(bricks[i].rectX, bricks[i].rectY, bricks[i].length, 5, bricks[i].colour);
    }

  }

  //Added brick generation.
  //Vishal
  void generateBricks(int yLevel) {

    int lengthCovered = 0;
      for (int i = 0; i < 4; i++) {
        bricks[numBricks].length = 32;
        bricks[numBricks].rectX += lengthCovered;
        lengthCovered += bricks[numBricks].length;
        bricks[numBricks].rectY = yLevel;
        bricks[numBricks].colour = colours[rand() % 18];
        numBricks += 1;
      }
      lengthCovered = 0;

  }

  //Scans the joystick and updates paddle.
  //Max
  void scanJoystick() {
    int horiz = analogRead(JOY_HORIZ_ANALOG);

    // Update g_joyX
    if (abs(horiz - JOY_CENTRE) > JOY_DEADZONE) {

      // number of pixels we wmat to move based on  the joystick position
      int delta = (horiz - JOY_CENTRE) / STEPS_PER_PIXEL;
      // move the cursor the given number of pixels


      if (delta<0) {
        g_joyX = constrain(g_joyX - 1, 10 , TFT_WIDTH-10);
        MoveLeft();
      } else {
        g_joyX = constrain(g_joyX + 1, 10 , TFT_WIDTH-10);
        MoveRight();
      }
    }

  }

  //Initializes the score.
  //Added: Writes score to bottom corner of screen
  //Max
  void Score() {
    tft.fillRect(0,153,TFT_WIDTH,7,Orange);
    tft.setCursor(0,153);
    tft.setTextColor(DarkGrey);
    tft.setTextSize(1);
    tft.print("SCORE: ");

    tft.setCursor(35,153);
    tft.setTextColor(Cyan);
    tft.setTextSize(1);
    tft.print(score);
  }

  //Update: Writes over just the score, instead of drawing a rectangle
  // updates and prints players curent score
  //Max
  void displayScore() {
    // tft.setCursor(35,153);
    // tft.setTextColor(Orange);
    // tft.setTextSize(1);
    // tft.print(score);

    //tft.fillRect(35,153,5,7,Orange);
    tft.setCursor(35,153);
    tft.setTextColor(Cyan);
    tft.setTextSize(1);
    tft.print(score);
  }

  /*Uses 2 stacks to insert a highscore anywhere in a column of 11 highscores
    by moving n items to another temporary stack, inserting the item, then
    moving n items back to the highscore stack until it is full.

    Runtime: O(n) = n^2
   */
  //Vishal
  void GameOver() {
    int select = !digitalRead(JOY_SEL);
    int vert = analogRead(JOY_VERT_ANALOG);
    int selection = 0;

    //Serial.println("Break1");
    if (score > highscore.top()) {
      letterSelection();
      while (true) {
        if (score > highscore.top()) {
          //Serial.print("Push: ");
          //Serial.println(highscore.numItems());
          temp.push(highscore.top(), highscore.initial1(), highscore.initial2(), highscore.initial3());
          highscore.pop();
        } else {
          //Serial.print("Stop: ");
          //Serial.println(highscore.numItems());
          highscore.push(score, initial1, initial2, initial3);
          break;
        }
        if (highscore.isEmpty()) {
          highscore.push(score, initial1, initial2, initial3);
          break;
        }
      }

      //Serial.println("Break2");
      while (!highscore.isFull()) {
        Serial.print("Push back");
        highscore.push(temp.top(), temp.initial1(), temp.initial2(), temp.initial3());
        temp.pop();
      }

    }

    for (int i = 41; i >= 1; i -= 4) {
      //before
      EEPROM[i] = highscore.top();
      EEPROM[i+1] = highscore.initial1();
      EEPROM[i+2] = highscore.initial2();
      EEPROM[i+3] = highscore.initial3();
      highscore.pop();
    }

    tft.fillScreen(Black);
    WriteText("GAME OVER", 9, 25, 2, Red);
    WriteText("YOUR SCORE:", 24, 45, 1, Yellow);
    tft.print(score);

    //WriteText("Press the joystick", 10, 130, 1, White);
    //WriteText("to play again!", 21, 140, 1, White);
    tft.setCursor(35, 85);
    tft.setTextSize(1);
    tft.setTextColor(Black, White);
    tft.print("Play Again");

    tft.setCursor(35, 105);
    tft.setTextSize(1);
    tft.setTextColor(White, Black);
    tft.print("Highscores");

    //BUG: Reselects the option selected if moved in the same direction.
    while (true) {
      vert = analogRead(JOY_VERT_ANALOG);
      select = !digitalRead(JOY_SEL);

      if (abs(vert - JOY_CENTRE) > JOY_DEADZONE) {
        // number of pixels we wmat to move based on  the joystick position
        int delta = (vert - JOY_CENTRE) / STEPS_PER_PIXEL;
        if (delta<0) {
          tft.setCursor(35, 85);
          tft.setTextSize(1);
          tft.setTextColor(Black, White);
          tft.print("Play Again");
          tft.setCursor(35, 105);
          tft.setTextSize(1);
          tft.setTextColor(White, Black);
          tft.print("Highscores");
          selection = 0;
        } else {
          tft.setCursor(35, 85);
          tft.setTextSize(1);
          tft.setTextColor(White, Black);
          tft.print("Play Again");
          tft.setCursor(35, 105);
          tft.setTextSize(1);
          tft.setTextColor(Black, White);
          tft.print("Highscores");
          selection = 1;
        }
      }


      if (select == 1) {
        if (selection == 0) {
          score = 0;
          initializeGameBoard();

          for (int i = 1; i <= 41; i += 4) {
            highscore.push(EEPROM[i], EEPROM[i+1], EEPROM[i+2], EEPROM[i+3]);
          }

          defeatstate = false;
          break;
        } else if (selection == 1) {
          highscoreTable();
          tft.fillScreen(Black);
          WriteText("GAME OVER", 9, 25, 2, Red);
          WriteText("YOUR SCORE:", 24, 45, 1, Yellow);
          tft.print(score);

          //WriteText("Press the joystick", 10, 130, 1, White);
          //WriteText("to play again!", 21, 140, 1, White);
          tft.setCursor(35, 85);
          tft.setTextSize(1);
          tft.setTextColor(Black, White);
          tft.print("Play Again");

          tft.setCursor(35, 105);
          tft.setTextSize(1);
          tft.setTextColor(White, Black);
          tft.print("Highscores");

          selection = 0;

        }
      }
    }
  }

  //Prints highscoreTable
  //Vishal
  void highscoreTable() {

    int select = !digitalRead(JOY_SEL);

    tft.fillScreen(0x0000);
    tft.setCursor(32, 10);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(1);
    tft.print("HIGHSCORES");

    WriteText("Press joystick to", 15, 140, 1, 0xFFFF, 0x0000);
    WriteText("go back!", 40, 150, 1, 0xFFFF, 0x0000);

    int yLevel = 25;
    //Serial.println("Break3");
    for (int i = 1; i <= 41; i += 4) {
      //WriteText("1: ", 0, (10*i), 1, 0xFFFF);

      tft.setCursor(40, yLevel);
      tft.setTextColor(0xFFFF);
      tft.setTextSize(1);
      tft.print((char)EEPROM[i+1]);
      tft.print((char)EEPROM[i+2]);
      tft.print((char)EEPROM[i+3]);
      tft.print(": ");
      tft.print(EEPROM[i]);

      yLevel += 10;

    }

  while (true) {
    select = !digitalRead(JOY_SEL);
    if (select == 1) {
      break;
    }
  }
}

  //Updates letter selected for the entering initials menu
  //Vishal
  void letterSelection() {

    int horiz = analogRead(JOY_HORIZ_ANALOG);
    int vert = analogRead(JOY_VERT_ANALOG);
    int select = digitalRead(JOY_SEL);

    int x = 20;
    int y = 70;
    int xSelected = 0;
    int ySelected = 0;
    int currentInitial = 0;
    int letterSelected[10][3];
    char letter = 'A';

    tft.fillScreen(0x0000);

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 10; j++) {
        letterSelected[j][i] = letter;
        //Serial.println(letter);
        letter += 1;
        if (letter == '[') {
          break;
        }
      }
    }

    WriteText("NEW HIGHSCORE", 25, 10, 1, 0xFFFF, 0x0000);
    WriteText("Enter Initials: ", 20, 20, 1, 0xFFFF, 0x0000);
    WriteText("_", 50, 40, 1, 0x0000, 0xFFFF);
    WriteText("_", 60, 40, 1, 0xFFFF, 0x0000);
    WriteText("_", 70, 40, 1, 0xFFFF, 0x0000);

    letter = 'A';

    // tft.setCursor(40, 30);
    // tft.setTextColor(0xFFFF);
    // tft.setTextSize(1);
    // tft.print("_");

    for (int i = 0; i < 26; i++) {
      if (i < 10) {
        tft.setCursor((x + (i*10)),y);
        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        tft.print(letter);
      } else if (i < 20) {
        tft.setCursor((x + (i*10) - (100)),y+10);
        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        tft.print(letter);
      } else {
        tft.setCursor((x + (i*10) - 200),y+20);
        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        tft.print(letter);
      }

      letter += 1;

    }


    updateSelection(20, 70, 'A', 0x0000, 0xFFFF);

    while (true) {

      horiz = analogRead(JOY_HORIZ_ANALOG);
      vert = analogRead(JOY_VERT_ANALOG);
      int select = !digitalRead(JOY_SEL);

      if (select == 1) {
        delay(100);
        if (currentInitial == 0) {
          tft.setCursor(50, 40);
          tft.setTextColor(0xFFFF, 0x0000);
          tft.setTextSize(1);
          tft.print((char)letterSelected[xSelected][ySelected]);
          WriteText("_", 60, 40, 1, 0x0000, 0xFFFF);
          initial1 = letterSelected[xSelected][ySelected];
          currentInitial += 1;
        } else if (currentInitial == 1) {
          tft.setCursor(60, 40);
          tft.setTextColor(0xFFFF, 0x0000);
          tft.setTextSize(1);
          tft.print((char)letterSelected[xSelected][ySelected]);
          WriteText("_", 70, 40, 1, 0x0000, 0xFFFF);
          initial2 = letterSelected[xSelected][ySelected];
          currentInitial += 1;
        } else {
          tft.setCursor(70, 40);
          tft.setTextColor(0xFFFF, 0x0000);
          tft.setTextSize(1);
          tft.print((char)letterSelected[xSelected][ySelected]);
          initial3 = letterSelected[xSelected][ySelected];
          break;
        }
      }

      // Update g_joyX
      if (abs(horiz - JOY_CENTRE) > JOY_DEADZONE) {
        int delta = (horiz - JOY_CENTRE);
        // move the cursor the given number of pixels
        if ((delta > 0) && !((xSelected > 4) && (ySelected == 2))) {
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0xFFFF, 0x0000);
          xSelected = constrain((xSelected + 1), 0, 9);
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0x0000, 0xFFFF);

        } else if (delta < 0) {
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0xFFFF, 0x0000);
          xSelected = constrain((xSelected - 1), 0, 9);
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0x0000, 0xFFFF);

        }
      }

      // Update g_joyY
      if (abs(vert - JOY_CENTRE) > JOY_DEADZONE) {
        int delta = (vert - JOY_CENTRE);
        if ((delta > 0) && !((xSelected > 5) && (ySelected == 1))) {
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0xFFFF, 0x0000);
          ySelected = constrain((ySelected + 1), 0, 2);
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0x0000, 0xFFFF);

        } else if (delta < 0) {
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0xFFFF, 0x0000);
          ySelected = constrain((ySelected - 1), 0, 2);
          updateSelection(20 + (xSelected*10), 70 + (ySelected*10), letterSelected[xSelected][ySelected], 0x0000, 0xFFFF);

        }
      }

      delay(100);

    }

  }

  //Added: Updates selection for the entering initials screen.
  //Vishal
  void updateSelection(int xCoordinate, int yCoordinate, char letterSelected, int color, int color2) {
    tft.setCursor(xCoordinate, yCoordinate);
    tft.setTextColor(color, color2);
    tft.setTextSize(1);
    tft.print(letterSelected);
  }


  //General print text function
  //Max
  void WriteText(String Text, int x, int y, int size,int color){
    tft.setCursor(x,y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.print(Text);
  }

  //Added: Adds to the score, this needed to be kept seperate from the displayScore
  //which is used to refresh the score
  //Vishal
  void addAndUpdateScore() {
    tft.setCursor(35,153);
    tft.setTextColor(Orange);
    tft.setTextSize(1);
    tft.print(score);

    score += 1;
    //tft.fillRect(35,153,5,7,Orange);
    tft.setCursor(35,153);
    tft.setTextColor(Cyan);
    tft.setTextSize(1);
    tft.print(score);
  }

  //Vishal
  void instructions() {

    int select = !digitalRead(JOY_SEL);

    String instructions = "Instructions: The\ntop layer of the\nscreen is lined with bricks and the"
    " bottom layer a controllable paddle, a small ball bounces around, if\nit hits"
    " a brick the\nbrick disappears, and if it hits the wall or the paddle it\n"
    "bounces. The goal of the game is to clear the bricks without\nletting"
    " the ball fall off.";

    tft.setTextWrap(true);
    tft.setTextSize(0.5);
    tft.fillScreen(Black);
    tft.setCursor(0, 0);
    tft.setTextColor(White);
    tft.print(instructions);
    delay(100);

    while (true) {
      select = !digitalRead(JOY_SEL);

      if (select == 1) {
        tft.setTextWrap(false);
        lcd_image_draw(&title_image, &tft, 0, 0, 0, 0, 128, 160);

        tft.setCursor(40, 85);
        tft.setTextSize(1);
        tft.setTextColor(Black, White);
        tft.print("Play Game");

        tft.setCursor(30, 105);
        tft.setTextSize(1);
        tft.setTextColor(White, Black);
        tft.print("Instructions");
        break;
      }
    }

  }

  //Added: Instructions menu
  //Vishal
  void startMenu() {

    int select = !digitalRead(JOY_SEL);
    int vert = analogRead(JOY_VERT_ANALOG);
    int selection = 0;

    tft.setTextWrap(false);
    lcd_image_draw(&title_image, &tft, 0, 0, 0, 0, 128, 160);

    tft.setCursor(40, 85);
    tft.setTextSize(1);
    tft.setTextColor(Black, White);
    tft.print("Play Game");

    tft.setCursor(30, 105);
    tft.setTextSize(1);
    tft.setTextColor(White, Black);
    tft.print("Instructions");

    while(true) {

      vert = analogRead(JOY_VERT_ANALOG);
      select = !digitalRead(JOY_SEL);

      if (abs(vert - JOY_CENTRE) > JOY_DEADZONE) {
        // number of pixels we wmat to move based on  the joystick position
        int delta = (vert - JOY_CENTRE) / STEPS_PER_PIXEL;
        if (delta<0) {
          tft.setCursor(40, 85);
          tft.setTextSize(1);
          tft.setTextColor(Black, White);
          tft.print("Play Game");
          tft.setCursor(30, 105);
          tft.setTextSize(1);
          tft.setTextColor(White, Black);
          tft.print("Instructions");
          selection = 0;
        } else {
          tft.setCursor(40, 85);
          tft.setTextSize(1);
          tft.setTextColor(White, Black);
          tft.print("Play Game");
          tft.setCursor(30, 105);
          tft.setTextSize(1);
          tft.setTextColor(Black, White);
          tft.print("Instructions");
          selection = 1;
        }
      }

      if (select == 1) {
        if (selection == 0) {
          break;
        } else if (selection == 1) {
          instructions();
          selection = 0;
        }
      }

    }

  }

  int main() {

    setup();
    startMenu();
    //initializeScoreBoard();
    initializeGameBoard();
    Score();
    ballTrajectory(1);

    //Initializes the highscore stack.
    for (int i = 1; i <= 41; i += 4) {
      highscore.push(EEPROM[i], EEPROM[i+1], EEPROM[i+2], EEPROM[i+3]);
    }

    //Scans the joystick and updates screen at 20 fps
    int prevTime = millis();
    while (true) {

      scanJoystick();
      updateBall();
      unitCollisionDetection();

      //TODO: Maybe add some next level text.
      if (numBricks == 0) {
        //Serial.println("Adding more bricks");

        initializeGameBoard();
        delay(2000); //A little bit of wait for the user.
      }

      if (defeatstate) {
        GameOver();
       }

      int t = millis();
      if (t - prevTime < MILLIS_PER_FRAME) {
        delay(MILLIS_PER_FRAME - (t - prevTime));
      }
      prevTime = millis();
    }

    Serial.end();

    return 0;
  }
