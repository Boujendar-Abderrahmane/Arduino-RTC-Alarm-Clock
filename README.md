# Arduino-RTC-Alarm-Clock
A programmable school bell system with support for normal and Ramadan schedules using Arduino, RTC module, LCD display, and buzzer
# 📚 School Bell Scheduler with Ramadan Mode (Arduino)

This project provides a **customizable and versatile school bell system** built on the Arduino platform. It intelligently manages daily alarms to signal class changes and breaks, offering seamless switching between **standard school hours** and **adjusted timings for Ramadan**. This is particularly useful in regions where educational institutions adapt their operating hours during the holy month.

## ✨ Features

* **Accurate Timekeeping:** Integrates a DS3231 Real-Time Clock (RTC) module to maintain precise time and date, even when the main power is off (thanks to its backup battery).
* **Dual Operating Modes:**
    * **Normal Mode:** Configured for typical school operating hours (e.g., 8:30 AM – 6:30 PM).
    * **Ramadan Mode:** Adapted for shorter school hours during Ramadan (e.g., 8:30 AM – 3:30 PM), ensuring breaks and class changes align with the adjusted schedule.
* **Configurable Alarms:** Easily set specific alarm times (hour, minute, second) and their ringing durations.
* **Audible Notifications:** A buzzer provides clear audible signals for each bell event.
* **Interactive Button Interface:** Intuitive controls for:
    * Adjusting the current RTC time (hour, minute, second).
    * Toggling between Normal and Ramadan operating modes.
    * Navigating through adjustment menus.
* **Real-time LCD Display:** A 16x2 I2C LCD displays the current time and the time of the next upcoming bell.
* **Daily Alarm Reset:** Alarms automatically reset each day to ensure they are triggered as expected.
* **Power-Off Time Retention:** The RTC module's battery backup ensures the clock retains its time settings even during power outages.

## 🚀 Modes Explained

The system features two distinct operational modes, easily switchable via a dedicated button:

### 🔹 Normal Mode
This mode utilizes the default alarm schedule for a typical school day.
**Pre-configured alarms:**
* **08:30:00** (e.g., First Bell)
* **10:30:00** (e.g., Break 1)
* **13:30:00** (e.g., Lunch Break)
* **14:30:00** (e.g., Afternoon Class Change)
* **18:30:00** (e.g., End of School Day)

### 🔸 Ramadan Mode
This mode is designed to accommodate adjusted school timings during Ramadan, typically reflecting a shorter school day.
**Pre-configured alarms:**
* **08:30:00** (e.g., First Bell, Ramadan schedule)
* **10:30:00** (e.g., Break 1, Ramadan schedule)
* **13:00:00** (e.g., Lunch/Break, Ramadan schedule)
* **15:30:00** (e.g., End of Adjusted School Day)

> **Quick Toggle:** Use the **Mode button** to seamlessly switch between Normal and Ramadan operation.

## 🛠️ Hardware Requirements

To build this project, you will need the following components:

| Component            | Quantity |
| :------------------- | :------- |
| Arduino UNO/Nano     | 1        |
| DS3231 RTC Module    | 1        |
| I2C 16x2 LCD Display | 1        |
| Buzzer               | 1        |
| Push Buttons         | 4        |
| Breadboard + Jumper Wires | As needed |

## 📌 Pin Configuration

Wire your components to the Arduino as per the following table. All buttons are configured with internal pull-ups (`INPUT_PULLUP`) and should connect to `GND` when pressed.

| Component         | Arduino Pin |
| :---------------- | :---------- |
| Buzzer            | D8          |
| Select Button     | D2          |
| Adjust + Button   | D4          |
| Adjust - Button   | D6          |
| Mode Button       | D7          |
| I2C LCD (SDA/SCL) | A4/A5       |
| DS3231 RTC (SDA/SCL) | A4/A5       |

## 💻 Code Overview

The project's logic is structured around:
* An `Alarm` `struct` to neatly manage individual alarm properties: `hour`, `minute`, `second`, `duration` (in seconds), and a `triggered` boolean flag.
* Timekeeping is robustly handled by the [RTClib](https://github.com/adafruit/RTClib) library for the DS3231 module.
* Alarms are automatically reset each day to ensure recurring daily triggers.
* The second row of the LCD dynamically displays the next upcoming alarm time, along with the current operating mode ("Norm" or "Ram").
* A user-friendly menu system allows precise adjustment of the RTC time directly via the push buttons.

## 🚀 Setup Instructions

Follow these steps to get your School Bell Scheduler ready:

1.  **Install Arduino IDE:** If you haven't already, download and install the latest version from [arduino.cc](https://www.arduino.cc/en/software).

2.  **Install Required Libraries:**
    * Open the Arduino IDE.
    * Navigate to `Sketch > Include Library > Manage Libraries...`.
    * Search for and install:
        * `RTClib` (by Adafruit)
        * `LiquidCrystal_I2C` (by DFRobot or Frank de Brabander)
    * `Wire.h` is a built-in library and does not require manual installation.

3.  **Upload the Sketch:**
    * Open the `School_Bell_Scheduler.ino` file (or whatever your main sketch file is named) in the Arduino IDE.
    * Select your Arduino board (`Tools > Board > Arduino AVR Boards > Arduino Uno` or your specific board).
    * Select the correct serial port (`Tools > Port`).
    * Click the "Upload" button (right arrow icon) to compile and upload the sketch to your Arduino.

4.  **Set RTC Time (First Time Only - **CRUCIAL**):**
    * **Locate this line** in the `setup()` function of your sketch:
        ```cpp
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        ```
    * **UNCOMMENT this line.**
    * **Upload the sketch again.** This will set the RTC to the time when the code was compiled.
    * **IMMEDIATELY COMMENT THIS LINE OUT AGAIN.**
    * **Re-upload the sketch.** This prevents the RTC from resetting every time the Arduino powers on or the sketch is uploaded in the future.

5.  **Wire the Circuit:** Connect all your components as detailed in the [Pin Configuration](#pin-configuration) table above.

## 💡 Usage

Once powered on and configured, the system operates as follows:

1.  **Observing Time & Next Alarm:**
    * The top line of the LCD continuously displays the current time.
    * The bottom line shows the time of the next scheduled alarm for the active mode ("Norm" or "Ram").

2.  **Switching Modes:**
    * Press the **Mode Button (D7)** to toggle between "Normal Mode" and "Ramadan Mode". The "Next" alarm display will update to reflect the schedule of the newly selected mode.

3.  **Adjusting Current Time:**
    * Press the **Select Button (D2)** to enter time adjustment mode.
    * The LCD will prompt you to adjust the Hour, then Minute, then Second.
    * Use the **Adjust + Button (D4)** to increment the value.
    * Use the **Adjust - Button (D6)** to decrement the value.
    * Press the **Select Button (D2)** again to confirm the current value and move to the next adjustment field.
    * After adjusting seconds, you'll see a "SAVE? YES/NO" prompt:
        * Press **Select Button (D2)** to save the new time to the RTC module.
        * Press **Adjust + Button (D4)** to discard changes and exit without saving.

## ⚙️ Customization

You can easily tailor the alarm times and their durations by modifying the `alarms` and `ramadanAlarms` arrays in the `School_Bell_Scheduler.ino` sketch:

```cpp
// --- Normal Mode Alarms (Example: Standard School Day Schedule) ---
// Format: {Hour, Minute, Second, Duration (seconds), Triggered? (internal state)}
#define NUM_ALARMS 5 // Crucially, this must match the number of alarms below!
Alarm alarms[NUM_ALARMS] = {
    {8, 30, 0, 3, false},  // First bell
    {10, 30, 0, 5, false}, // Break 1
    {13, 0, 0, 3, false},  // Lunch Break
    {14, 30, 0, 2, false}, // Afternoon class change
    {18, 30, 0, 5, false}  // End of school day
};

// --- Ramadan Mode Alarms (Example: Adjusted School Schedule) ---
// Format: {Hour, Minute, Second, Duration (seconds), Triggered? (internal state)}
#define NUM_RAMADAN_ALARMS 4 // Crucially, this must match the number of alarms below!
Alarm ramadanAlarms[NUM_RAMADAN_ALARMS] = {
    {8, 30, 0, 5, false},   // First bell, Ramadan schedule
    {10, 30, 0, 5, false},  // Break 1, Ramadan schedule
    {13, 0, 0, 5, false},   // Lunch/Break, Ramadan schedule
    {15, 30, 0, 3, false}   // End of adjusted school day
};
````

  * **Important:** Always ensure that `NUM_ALARMS` and `NUM_RAMADAN_ALARMS` precisely match the number of `Alarm` entries in their respective arrays.
  * You can also adjust other configuration constants at the top of the sketch, such as `BUZZER_PIN` or `LCD_ADDRESS`, to match your specific hardware setup.

## troubleshooting

If you encounter any issues, here are some common problems and their solutions:

  * **"Couldn't find RTC module\!" or "RTC ERROR\!" on LCD:**
      * **Check Wiring:** Double-verify all connections to your DS3231 module (SDA, SCL, VCC, GND). Ensure they are firmly seated.
      * **Library Installation:** Confirm that the `Wire.h` (built-in) and `RTClib` (Adafruit) libraries are correctly installed.
      * **Module Fault:** The RTC module itself might be faulty.
  * **LCD shows blank boxes or nothing:**
      * **Contrast Potentiometer:** Gently adjust the small potentiometer (usually blue) on the back of the I2C LCD module until text becomes visible.
      * **I2C Address:** Verify that the `LCD_ADDRESS` constant in your code matches the actual I2C address of your LCD module. Use an I2C scanner sketch if unsure.
      * **Wiring:** Re-check SDA, SCL, VCC, and GND connections to the LCD.
  * **Buzzer not sounding:**
      * **Wiring:** Ensure the buzzer is connected correctly (positive to `BUZZER_PIN`, negative to GND). Some buzzers are polarized.
      * **Pin Definition:** Confirm that the `BUZZER_PIN` define in your code matches the Arduino pin you've wired the buzzer to.
  * **Buttons not responding or behaving erratically:**
      * **Wiring:** Ensure buttons are wired correctly (one leg to the Arduino pin, the other to GND).
      * **`INPUT_PULLUP`:** Confirm that `pinMode(BUTTON_PIN, INPUT_PULLUP);` is correctly set for all button pins in `setup()`.
      * **Debounce Delay:** If you experience multiple presses from a single button push, try slightly increasing the `debounceDelay` constant in the code.
  * **Time not retaining after power off/on:**
      * **RTC Battery:** Ensure your DS3231 RTC module has a working CR2032 backup battery installed. Without it, the time will reset on power loss.
      * **First-Time RTC Setup:** Verify you correctly performed the "Set RTC Time (First Time Only - CRUCIAL)" procedure.

## 🚀 Future Improvements (Suggestions)

  * **EEPROM Support:** Implement storing custom alarm times in the Arduino's EEPROM, allowing user-defined alarms to persist across power cycles without re-uploading code.
  * **Alarm Editing UI:** Develop a more advanced user interface via buttons to allow users to add, edit, or delete individual alarm times directly on the device.
  * **Web/Mobile Interface:** Integrate a Wi-Fi module (e.g., ESP8266) to enable configuration and monitoring via a web browser or a mobile application.
  * **Daylight Saving Handling:** Add logic to automatically adjust for Daylight Saving Time changes if applicable to the region of use.

## 🤝 Contributing

Contributions, issues, and feature requests are welcome\! Feel free to check [issues page](https://www.google.com/search?q=https://github.com/your-username/your-repo-name/issues) or create a [pull request](https://www.google.com/search?q=https://github.com/your-username/your-repo-name/pulls).

## 📄 License

This project is open-source and licensed under the [MIT License](LICENSE.md).

## 🧑‍💻 Author

Made with care to accommodate real-life Ramadan scheduling needs in schools.

**BOUJENDAR ABDERRAHMANE**

  * GitHub: [@Boujendar-Abderrahmane](https://github.com/Boujendar-Abderrahmane)
  * Email: `boujendar.abderrahmane [at] gmail [dot] com` 



## 🙏 Acknowledgements

  * **Adafruit:** For their excellent `RTClib` library.
  * **Arduino Community:** For the vast amount of resources and support available.
  * **DFRobot / Frank de Brabander:** For the `LiquidCrystal_I2C` library.
