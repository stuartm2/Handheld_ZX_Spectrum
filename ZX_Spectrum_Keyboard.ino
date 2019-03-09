
#include <Keyboard.h>
#include <Mouse.h>

#define NUM_KEYS 50
#define HORIZONTAL 0
#define VERTICAL 1

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

char specialMap[7] = {KEY_RETURN,      // Joypad btn
                      KEY_ESC,         // Large btn
                      KEY_F1,          // Small btn
                      KEY_UP_ARROW,
                      KEY_RIGHT_ARROW,
                      KEY_DOWN_ARROW,
                      KEY_LEFT_ARROW};

int keysDown[NUM_KEYS];
bool symWasPressed = false;
bool capWasPressed = false;
bool isMouse = false;
bool isSpecial = false;
bool mousePressed[2] = {0, 0};

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
  symWasPressed = detectModKeys(SYM_PIN, SYM, symWasPressed);
  capWasPressed = detectModKeys(CAP_PIN, CAP, capWasPressed);

  if (isMouse) {
    doMouse();
  } else if (Serial.available() >= 7) {
    setJoyKeys();
  } else {
    doKeys();
  }
}

void doKeys() {
  for (int col = 0; col < 6; col++) {
    digitalWrite(COL_PINS[col], LOW);

    for (int row = 0; row < 8; row++) {
      int i = matrixToKeyMap(col, row);
      char c = isSpecial ? specialMap[i] : keyMap[i];

      if (digitalRead(ROW_PINS[row]) == LOW) {
        if (i == 1 && symWasPressed) {
          digitalWrite(COL_PINS[col], HIGH);
          releaseAllKeys();
          enterMouseMode();
          return;
        } else if (i == 1 && capWasPressed) {
          digitalWrite(COL_PINS[col], HIGH);
          toggleSpecialMode();
          return;
        } else if (keysDown[i] == 0 && c != 0x0) {
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

void doMouse() {
  digitalWrite(COL_PINS[0], LOW);

  for (int btn; btn < 8; btn++) {
    if (digitalRead(ROW_PINS[btn]) == LOW) {
      if (symWasPressed && (btn == 2 || btn == 3)) { // Exit mouse mode
        digitalWrite(COL_PINS[0], HIGH);
        exitMouseMode();
        return;
      } else if (btn == 4) { // Up
        Mouse.move(0, -1, 0);
      } else if (btn == 5) { // Right
        Mouse.move(1, 0, 0);
      } else if (btn == 6) { // Down
        Mouse.move(0, 1, 0);
      } else if (btn == 7) { // Left
        Mouse.move(-1, 0, 0);
      }

      if (btn == 2 && !mousePressed[0]) {
        mousePressed[0] = 1;
        Mouse.press();
      } else if (btn == 3 && !mousePressed[1]) {
        mousePressed[1] = 1;
        Mouse.press(MOUSE_RIGHT);
      }
    } else if (btn == 2 && mousePressed[0]) {
      Mouse.release();
      mousePressed[0] = 0;
    } else if (btn == 3 && mousePressed[1]) {
      Mouse.release(MOUSE_RIGHT);
      mousePressed[1] = 0;
    }
  }

  digitalWrite(COL_PINS[0], HIGH);
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

void enterMouseMode() {
  Mouse.begin();
  isMouse = true;
  doJoggle(HORIZONTAL);
}

void exitMouseMode() {
  if (mousePressed[0]) {
    Mouse.release();
    mousePressed[0] = 0;
  }

  if (mousePressed[1]) {
    Mouse.release(MOUSE_RIGHT);
    mousePressed[1] = 0;
  }

  Mouse.end();
  isMouse = false;
  doJoggle(VERTICAL);
}

void doJoggle(int dir) {
  int x = (dir == HORIZONTAL);
  int y = (dir == VERTICAL);

  for (int i = 0; i < 5; i++) {
    Mouse.move(x * 5, y * 5, 0);
    delay(50);
    Mouse.move(x * -5, y * -5, 0);
    delay(50);
  }
}

void toggleSpecialMode() {
  isSpecial = !isSpecial;

  if (isSpecial) {
    Keyboard.release(CAP);
  } else {
    Keyboard.press(CAP);
  }

  delay(500);
}

void releaseAllKeys() {
  for (int i = 0; i < NUM_KEYS; i++) {
    keysDown[i] = 0;
  }
}

int matrixToKeyMap(int col, int row) {
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

