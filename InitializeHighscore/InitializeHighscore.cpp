#include <Arduino.h>
#include <EEPROM.h>

//Max highscore is 256
int main() {
  init();
  Serial.begin(9600);

  EEPROM[1] = 11;
  EEPROM[2] = 'D';
  EEPROM[3] = 'A';
  EEPROM[4] = 'D';
  EEPROM[5] = 10;
  EEPROM[6] = 'V';
  EEPROM[7] = 'P';
  EEPROM[8] = 'P';
  EEPROM[9] = 9;
  EEPROM[10] = 'S';
  EEPROM[11] = 'U';
  EEPROM[12] = 'P';
  EEPROM[13] = 8;
  EEPROM[14] = 'W';
  EEPROM[15] = 'O';
  EEPROM[16] = 'W';
  EEPROM[17] = 7;
  EEPROM[18] = 'G';
  EEPROM[19] = 'G';
  EEPROM[20] = 'G';
  EEPROM[21] = 6;
  EEPROM[22] = 'U';
  EEPROM[23] = 'P';
  EEPROM[24] = 'L';
  EEPROM[25] = 5;
  EEPROM[26] = 'O';
  EEPROM[27] = 'W';
  EEPROM[28] = 'M';
  EEPROM[29] = 4;
  EEPROM[30] = 'G';
  EEPROM[31] = 'G';
  EEPROM[32] = 'S';
  EEPROM[33] = 3;
  EEPROM[34] = 'P';
  EEPROM[35] = 'O';
  EEPROM[36] = 'W';
  EEPROM[37] = 2;
  EEPROM[38] = 'W';
  EEPROM[39] = 'W';
  EEPROM[40] = 'G';
  EEPROM[41] = 1;
  EEPROM[42] = 'P';
  EEPROM[43] = 'O';
  EEPROM[44] = 'P';

  Serial.end();
  return 0;
}
