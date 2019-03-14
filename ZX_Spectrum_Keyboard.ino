
#include <Joystick.h>
#include <Keyboard.h>
#include <Mouse.h>

#define NUM_KEYS 50
#define HORIZONTAL 0
#define VERTICAL 1
#define MODE_SWITCH_DELAY 500

const int COL_PINS[] = {10, 16, 14, 15, A0, A1};
const int ROW_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int CAP_PIN = A2;
const int SYM_PIN = A3;
const int JOY_LED = 0;
const int MOUSE_LED = 1;

const int ENT = KEY_RETURN,
          F_1 = KEY_F1,
          D_U = KEY_UP_ARROW,
          D_R = KEY_RIGHT_ARROW,
          D_D = KEY_DOWN_ARROW,
          D_L = KEY_LEFT_ARROW,
          CAP = KEY_LEFT_SHIFT,
          SYM = KEY_LEFT_CTRL;

char keyMap[NUM_KEYS] = {
  0x0, 0x0, ENT, F_1, D_U, D_R, D_D, D_L, 0x0, 0x0, // 0-9
  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', // 10-19
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', // 20-29
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ENT, // 30-39
  CAP, 'z', 'x', 'c', 'v', 'b', 'n', 'm', SYM, ' '  // 40-49
};

int keysDown[NUM_KEYS];
bool symWasPressed = false;
bool capWasPressed = false;
bool isMouse = false;
bool isJoystick = false;
bool mousePressed[2] = {0, 0};
bool joyPressed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
  2, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

void setup() {
  pinMode(SYM_PIN, INPUT_PULLUP);
  pinMode(CAP_PIN, INPUT_PULLUP);
  pinMode(MOUSE_LED, OUTPUT);
  pinMode(JOY_LED, OUTPUT);

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
}

void loop() {
  symWasPressed = detectModKeys(SYM_PIN, SYM, symWasPressed);
  capWasPressed = detectModKeys(CAP_PIN, CAP, capWasPressed);

  if (isMouse) {
    doMouse();
  } else if (isJoystick) {
    doJoystick();
  } else {
    doTopKeys();
  }

  doKeys();
}

void doTopKeys() {
  digitalWrite(COL_PINS[0], LOW);

  for (int i = 0; i < 8; i++) {
    char c = keyMap[i];

    if (digitalRead(ROW_PINS[i]) == LOW) {
      if (i == 2 && symWasPressed) {
        digitalWrite(COL_PINS[0], HIGH);
        releaseAllKeys();
        enterMouseMode();
        delay(MODE_SWITCH_DELAY);
        return;
      } else if (i == 2 && capWasPressed) {
        digitalWrite(COL_PINS[0], HIGH);
        releaseAllKeys();
        enterJoystickMode();
        delay(MODE_SWITCH_DELAY);
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

  digitalWrite(COL_PINS[0], HIGH);
}

void doKeys() {
  for (int col = 1; col < 6; col++) {
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

void doJoystick() {
  digitalWrite(COL_PINS[0], LOW);

  for (int btn = 0; btn < 8; btn++) {
    if (digitalRead(ROW_PINS[btn]) == LOW) {
      if (btn == 2 && symWasPressed) {
        digitalWrite(COL_PINS[0], HIGH);
        releaseAllKeys();
        exitJoystickMode();
        enterMouseMode();
        delay(MODE_SWITCH_DELAY);
        return;
      } else if (btn == 2 && capWasPressed) {
        digitalWrite(COL_PINS[0], HIGH);
        releaseAllKeys();
        exitJoystickMode();
        delay(MODE_SWITCH_DELAY);
        return;
      }

      if (btn == 2 && !joyPressed[btn]) { // Large fire
        joyPressed[btn] = 1;
        Joystick.setButton(0, true);
      } else if (btn == 3 && !joyPressed[btn]) { // Small fire
        joyPressed[btn] = 1;
        Joystick.setButton(1, true);
      } else if (btn == 4 && !joyPressed[btn]) { // Up
        joyPressed[btn] = 1;
        Joystick.setYAxis(-1);
      } else if (btn == 5 && !joyPressed[btn]) { // Right
        joyPressed[btn] = 1;
        Joystick.setXAxis(1);
      } else if (btn == 6 && !joyPressed[btn]) { // Down
        joyPressed[btn] = 1;
        Joystick.setYAxis(1);
      } else if (btn == 7 && !joyPressed[btn]) { // Left
        joyPressed[btn] = 1;
        Joystick.setXAxis(-1);
      }
    } else {
      if (btn == 2 && joyPressed[btn]) {
        joyPressed[btn] = 0;
        Joystick.setButton(0, false);
      } else if (btn == 3 && joyPressed[btn]) {
        joyPressed[btn] = 0;
        Joystick.setButton(1, false);
      } else if (btn == 4 && joyPressed[btn]) {
        joyPressed[btn] = 0;
        Joystick.setYAxis(0);
      } else if (btn == 5 && joyPressed[btn]) {
        joyPressed[btn] = 0;
        Joystick.setXAxis(0);
      } else if (btn == 6 && joyPressed[btn]) {
        joyPressed[btn] = 0;
        Joystick.setYAxis(0);
      } else if (btn == 7 && joyPressed[btn]) {
        joyPressed[btn] = 0;
        Joystick.setXAxis(0);
      }
    }
  }

  digitalWrite(COL_PINS[0], HIGH);
  delay(10);
}

void doMouse() {
  digitalWrite(COL_PINS[0], LOW);

  for (int btn; btn < 8; btn++) {
    if (digitalRead(ROW_PINS[btn]) == LOW) {
      if (btn == 2 && symWasPressed) {
        digitalWrite(COL_PINS[0], HIGH);
        releaseAllKeys();
        exitMouseMode();
        delay(MODE_SWITCH_DELAY);
        return;
      } else if (btn == 2 && capWasPressed) {
        digitalWrite(COL_PINS[0], HIGH);
        releaseAllKeys();
        exitMouseMode();
        enterJoystickMode();
        delay(MODE_SWITCH_DELAY);
        return;
      }

      if (btn == 4) { // Up
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

bool detectModKeys(int pin, char key, bool prevState) {
  if (digitalRead(pin) == LOW) {
    Keyboard.press(key);
    return true;
  } else if (prevState) {
    Keyboard.release(key);
    return false;
  }
}

void enterJoystickMode() {
  Joystick.begin(true);
  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
  isJoystick = true;
  digitalWrite(JOY_LED, HIGH);
}

void exitJoystickMode() {
  for (int i = 0; i < 8; i++) {
    joyPressed[i] = 0;
  }

  Joystick.end();
  isJoystick = false;
  digitalWrite(JOY_LED, LOW);
}

void enterMouseMode() {
  Mouse.begin();
  isMouse = true;
  digitalWrite(MOUSE_LED, HIGH);
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
  digitalWrite(MOUSE_LED, LOW);
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

