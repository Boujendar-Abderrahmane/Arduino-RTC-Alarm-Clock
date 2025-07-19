#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// --- Configuration Constants ---
#define LCD_ADDRESS 0x27 // Adjust if needed (common addresses: 0x27, 0x3F)
#define LCD_COLUMNS 16
#define LCD_ROWS 2

#define BUZZER_PIN 8

// controle buttons
#define SELECT_BUTTON_PIN 2 
#define ADJUST_BUTTON_PIN 4
#define ADJUST_BACK_BUTTON_PIN 6 
#define MODE_BUTTON_PIN 7


//

#define NUM_ALARMS 5
#define NUM_RAMADAN_ALARMS 3 



enum ClockMode {
  NORMAL_MODE,
  RAMADAN_MODE
};
ClockMode currentMode = NORMAL_MODE; // Default to normal mode
ClockMode activeAlarmMode;

enum AdjustState {
  VIEW_TIME, // Normal operation
  ADJUST_HOUR,
  ADJUST_MINUTE,
  ADJUST_SECOND,
  SAVE_CHANGES
};

AdjustState adjustState = VIEW_TIME;
DateTime tempTime; // Variable to hold time being adjusted



// --- Global Objects ---
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
RTC_DS3231 rtc;

// --- Alarm Structure ---
struct Alarm {
  int hour;
  int minute;
  int second;
  int duration; // in seconds
  bool triggered;
};

// --- Alarm Definitions ---
Alarm alarms[NUM_ALARMS] = {
  {8, 30, 0, 3, false},
  {12, 30, 0, 5, false},
  {13, 0, 0, 3, false},
  {14, 30, 45, 2, false},
  {20, 30, 00, 5, false}
};


Alarm ramadanAlarms[NUM_RAMADAN_ALARMS] = {
  {4, 0, 0, 5, false},  
  {18, 30, 0, 5, false}, 
  {21, 0, 0, 3, false}  
};

// --- Global Variables for Buzzer and State ---
bool buzzerOn = false;
unsigned long buzzerStartMillis = 0; 
int activeAlarmIndex = -1;
int lastResetDay = -1; // To track daily alarm resets
bool inAdjustmentMode = false;

// Debouncing variables
unsigned  long lastSelectButtonPress = 0;
unsigned  long lastAdjustButtonPress = 0;
unsigned  long lastAdjustBackButtonPress = 0;
const     long debounceDelay = 100; // ms
unsigned  long lastModeButtonPress = 0;
// --- Function Prototypes ---

void displayTime(DateTime now);
void handleBuzzer(DateTime now);
void resetAlarmsDaily(DateTime now);
void checkAlarms(DateTime now);
void displayNextAlarm(DateTime now);
bool isButtonPressed(int pin, unsigned long *lastPressTime);
void adjustTime();


// ====================================================================
// SETUP FUNCTION
// ====================================================================
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off initially
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(ADJUST_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ADJUST_BACK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.clear(); // Clear LCD on startup

  Serial.begin(9600);
  

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC module!");
    lcd.setCursor(0, 0);
    lcd.print("RTC ERROR!");
    lcd.setCursor(0, 1);
    lcd.print("Check wiring.");
    while (true); // Halt execution if RTC is not found
  }

  // Uncomment the line below ONLY ONCE to set the RTC time
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // If RTC lost power, set time to compilation time
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}


// ====================================================================
// LOOP FUNCTION
// ====================================================================
void loop() {
  if (adjustState == VIEW_TIME) {// Only allow mode change in normal view mode
      if (isButtonPressed(MODE_BUTTON_PIN, &lastModeButtonPress)) {
        if (currentMode == NORMAL_MODE) {
            currentMode = RAMADAN_MODE;
            Serial.println("Switched to Ramadan Mode");
        }else {
        currentMode = NORMAL_MODE;
        Serial.println("Switched to Normal Mode");
        }
      lcd.clear(); // Clear LCD to reflect new mode immediately
      };

    // Normal operation 
    DateTime now = rtc.now();
    resetAlarmsDaily(now);
    checkAlarms(now);
    handleBuzzer(now);
    displayTime(now);
    displayNextAlarm(now);
    // Check for entering adjustment mode
    if (isButtonPressed(SELECT_BUTTON_PIN, &lastSelectButtonPress)) {
        adjustState = ADJUST_HOUR;
        tempTime = rtc.now(); // Get current time to modify
        lcd.clear();
    }
    delay(1000); // Only delay if not in adjustment mode
  } else {
    // In adjustment mode, call the adjustment function
    adjustTime();
    delay(100); // Shorter delay for responsiveness during adjustment
  }
}


// ====================================================================
// HELPER FUNCTIONS
// ====================================================================

/**
 * @brief Displays the current time on the LCD.
 * @param now The current DateTime object from the RTC.
 */
void displayTime(DateTime now) {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());
}
// --- Helper function to get current alarms based on mode ---
Alarm* getCurrentAlarms() {
  if (currentMode == RAMADAN_MODE) {
    return ramadanAlarms;
  } else {
    return alarms;
  }
}

int getCurrentNumAlarms() {
  if (currentMode == RAMADAN_MODE) {
    return NUM_RAMADAN_ALARMS;
  } else {
    return NUM_ALARMS;
  }
}



/**
 * @brief Resets all alarms to not triggered at the start of a new day.
 * @param now The current DateTime object from the RTC.
 */
void resetAlarmsDaily(DateTime now) {
  if (now.day() != lastResetDay) {
    Serial.println("New day! Resetting all alarms.");
    // Reset both sets of alarms
    for (int i = 0; i < NUM_ALARMS; i++) {
      alarms[i].triggered = false;
    }
    for (int i = 0; i < NUM_RAMADAN_ALARMS; i++) {
      ramadanAlarms[i].triggered = false;
    }
    lastResetDay = now.day();
  }
}

/**
 * @brief Checks if any alarm's time matches the current time and triggers it.
 * @param now The current DateTime object from the RTC.
 */
void checkAlarms(DateTime now) {
  // Only check alarms if no alarm is currently active
  if (!buzzerOn) {
    Alarm* activeAlarms = getCurrentAlarms();
    int numActiveAlarms = getCurrentNumAlarms();

    for (int i = 0; i < numActiveAlarms; i++) {
      if (!activeAlarms[i].triggered &&
         now.hour() == activeAlarms[i].hour &&
         now.minute() == activeAlarms[i].minute &&
         now.second() == activeAlarms[i].second) {

        Serial.print("Alarm triggered (Mode ");
        Serial.print(currentMode == RAMADAN_MODE ? "Ramadan" : "Normal");
        Serial.print("): ");
        Serial.println(i);
        buzzerOn = true;
        buzzerStartMillis = millis();
        digitalWrite(BUZZER_PIN, HIGH);
        activeAlarmIndex = i; // This index refers to the activeAlarms array
        activeAlarmMode = currentMode;
        activeAlarms[i].triggered = true; // Mark as triggered
        break;
      }
    }
  }
}


/**
 * @brief Manages the buzzer's on/off state based on alarm duration.
 * @param now The current DateTime object from the RTC (not directly used but included for consistency).
 */
void handleBuzzer(DateTime now) { 
       if (buzzerOn && activeAlarmIndex != -1) {
          //Determine which alarm array to use based on activeAlarmMode
          Alarm* currentlyActiveAlarmArray;
          if (activeAlarmMode == RAMADAN_MODE) {
               currentlyActiveAlarmArray = ramadanAlarms;
          } else {
               currentlyActiveAlarmArray = alarms;
        }
    // Check if the alarm duration has passed
    if (millis() - buzzerStartMillis >= (unsigned long)currentlyActiveAlarmArray[activeAlarmIndex].duration * 1000) {
      Serial.println("Buzzer duration ended.");
      digitalWrite(BUZZER_PIN, LOW); // Turn buzzer off
      buzzerOn = false;             // Reset buzzer state
      activeAlarmIndex = -1;        // No active alarm
    }
  }
}

/**
 * @brief Finds and displays the next upcoming, untriggered alarm.
 * @param now The current DateTime object from the RTC.
 */
void displayNextAlarm(DateTime now) {
  lcd.setCursor(0, 1); // Second line for next alarm

  // Initialize with a far future time to find the soonest
  DateTime nextAlarmTime = DateTime(2100, 1, 1, 0, 0, 0); // Far future date
  int nextAlarmHour = -1;
  int nextAlarmMinute = -1;
  bool nextFound = false;

  Alarm* activeAlarms = getCurrentAlarms(); // Get the correct alarm set
  int numActiveAlarms = getCurrentNumAlarms(); // Get the correct count

  for (int i = 0; i < numActiveAlarms; i++) { // Loop through the current alarm set
    DateTime alarmCandidateTime(now.year(), now.month(), now.day(),
                                activeAlarms[i].hour, activeAlarms[i].minute, activeAlarms[i].second);

    if (alarmCandidateTime < now && !activeAlarms[i].triggered) {
      alarmCandidateTime = DateTime(now.year(), now.month(), now.day() + 1,
                                    activeAlarms[i].hour, activeAlarms[i].minute, activeAlarms[i].second);
    } else if (alarmCandidateTime < now && activeAlarms[i].triggered) {
      continue;
    }

    if (!activeAlarms[i].triggered && alarmCandidateTime < nextAlarmTime) {
      nextAlarmTime = alarmCandidateTime;
      nextAlarmHour = activeAlarms[i].hour;
      nextAlarmMinute = activeAlarms[i].minute;
      nextFound = true;
    }
  }

  // ... rest of displayNextAlarm remains the same ...
  if (nextFound) {
    lcd.print("Next:");
    if (nextAlarmHour < 10) lcd.print("0");
    lcd.print(nextAlarmHour);
    lcd.print(":");
    if (nextAlarmMinute < 10) lcd.print("0");
    lcd.print(nextAlarmMinute);
    // Add mode indicator
    lcd.print(currentMode == RAMADAN_MODE ? " Ram " : " Norm ");
    lcd.print("   "); // Clear any leftover characters
  } else {
    lcd.print("Next: --:--      ");
  }
}



bool isButtonPressed(int pin, unsigned long *lastPressTime) {
  if (digitalRead(pin) == LOW) { // Assuming pull-up resistors, button press makes it LOW
    if (millis() - *lastPressTime > debounceDelay) {
      *lastPressTime = millis();
      return true;
    }
  }
  return false;
}

void adjustTime() {
  // --- Button Handling ---
  bool selectPressed = isButtonPressed(SELECT_BUTTON_PIN, &lastSelectButtonPress);  //Select_BUTTON_PIN 2
  bool adjustPressed = isButtonPressed(ADJUST_BUTTON_PIN, &lastAdjustButtonPress); //ADJUST_BUTTON_PIN 4
  bool adjustBackPressed = isButtonPressed(ADJUST_BACK_BUTTON_PIN, &lastAdjustBackButtonPress); //ADJUST_BUTTON_PIN 6

  // --- State Machine ---
  switch (adjustState) {
 
    case ADJUST_HOUR:
      if (adjustPressed) tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), (tempTime.hour() + 1) % 24, tempTime.minute(), tempTime.second());
      if (adjustBackPressed) tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), (tempTime.hour() - 1) % 24, tempTime.minute(), tempTime.second());
      if (selectPressed) adjustState = ADJUST_MINUTE;
      lcd.setCursor(0,0); lcd.print("Adj Hour:");
      lcd.setCursor(0,1); if(tempTime.hour()<10)lcd.print("0"); lcd.print(tempTime.hour()); lcd.print("  "); // Display current adjusted value
      break;

    case ADJUST_MINUTE:
      if (adjustPressed) tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(), (tempTime.minute() + 1) % 60, tempTime.second());
      if (adjustBackPressed) tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(),(tempTime.minute() - 1) % 60, tempTime.second());
      if (selectPressed) adjustState = ADJUST_SECOND;
      lcd.setCursor(0,0); lcd.print("Adj Min:");
      lcd.setCursor(0,1); if(tempTime.minute()<10)lcd.print("0"); lcd.print(tempTime.minute()); lcd.print("  ");
      break;

    case ADJUST_SECOND:
      if (adjustPressed) tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(), tempTime.minute() , ( tempTime.second() + 1) % 60);
      if (adjustBackPressed) tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(),tempTime.minute(), (tempTime.second()- 1) % 60);
      if (selectPressed) adjustState = SAVE_CHANGES;
      lcd.setCursor(0,0); lcd.print("Adj Sec:");
      lcd.setCursor(0,1); if(tempTime.minute()<10)lcd.print("0"); lcd.print(tempTime.second()); lcd.print("  ");
      break;

    case SAVE_CHANGES:
      lcd.setCursor(0,0); lcd.print("SAVE? YES/NO");
      lcd.setCursor(0,1); lcd.print("Selct=Yes,Adj=No");
      if (selectPressed) {
        rtc.adjust(tempTime); // Save new time to RTC
        Serial.println("Time Saved!");
        lcd.clear();
        adjustState = VIEW_TIME; // Exit adjustment mode
      }
      if (adjustPressed) { // Discard changes
        Serial.println("Time adjustment cancelled.");
        lcd.clear();
        adjustState = VIEW_TIME; // Exit adjustment mode
      }
      break;
  }
}