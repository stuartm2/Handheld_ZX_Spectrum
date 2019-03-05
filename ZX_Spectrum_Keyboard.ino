
#include <Keyboard.h>

#define NUM_KEYS 50

const int COL_PINS[] = {10, 16, 14, 15, A0, A1};
const int ROW_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int CAP_PIN = A2;
const int SYM_PIN = A3;

const int ENT = KEY_RETURN,
          CAP = KEY_LEFT_SHIFT,
          SYM = KEY_LEFT_CTRL;

char keyMap[NUM_KEYS] = {
  ' ', 'f', 'h', 'w', 'd', 's', 'a', 0x0, 0x0, 0x0, // 0-9
  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', // 10-19
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', // 20-29
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ENT, // 30-39
  CAP, 'z', 'x', 'c', 'v', 'b', 'n', 'm', SYM, ' '  // 40-49
};

int keysDown[NUM_KEYS];
bool symWasPressed = false;
bool capWasPressed = false;

void setup() {
  pinMode(SYM_PIN, INPUT_PULLUP);
  pinMode(CAP_PIN, INPUT_PULLUP);

  for (int i = 0; i < NUM_KEYS; i++) {
    keysDown[i] = 0;
  }

  for (int i = 0; i < 6; i++) {
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(COL_PINS[i], HIGH);
  }

  for (int i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], INPUT_PULLUP);
  }

  Keyboard.begin();
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() >= 7) {
    setJoyKeys();
  }

  symWasPressed = detectModKeys(SYM_PIN, SYM, symWasPressed);
  capWasPressed = detectModKeys(CAP_PIN, CAP, capWasPressed);

  for (int col = 0; col < 6; col++) {
    digitalWrite(COL_PINS[col], LOW);

    for (int row = 0; row < 8; row++) {
      int i = matrixToKeyMap(col, row);
      char c = keyMap[i];

      if (digitalRead(ROW_PINS[row]) == LOW) {
        if (keysDown[i] == 0 && c != 0x0) {
          Keyboard.press(c);
          keysDown[i] = 1;
        }
      } else if (keysDown[i] == 1) {
        Keyboard.release(c);
          keysDown[i] = 0;
      }
    }

    digitalWrite(COL_PINS[col], HIGH);
  }
}

void setJoyKeys() {
  // We're only interested in the first 7 bytes
  for (int i = 0; i < 7; i++) {
    int b = Serial.read();
    Serial.write("");
    Serial.write(keyMap[i]);
    Serial.write(">");
    Serial.write(b);
    Serial.write("\n");
    keyMap[i] = b;
  }

  // Clear the remaining buffer
  while (Serial.available()) {
    Serial.read();
  }
}

bool detectModKeys(int pin, char key, bool prevState) {
  if (digitalRead(pin) == LOW) {
    Keyboard.press(key);
    return true;
  } else if (prevState) {
    Keyboard.release(key);
    return false;
  }
}

int matrixToKeyMap(int col, int row) {
  // I may be able to do it directly from the matrix map
  int r, c;

  if (col == 0) {
    c = row - 1;
    r = col;
  } else if (row > 3) {
    c = 5 - col;
    r = row - 3;
  } else {
    c = col + 4;
    r = 4 - row;
  }

  return (r * 10) + c;
}

