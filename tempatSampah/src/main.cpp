#include <Arduino.h>

// Define the pin connected to the touch sensor
#define TOUCH_PIN 4

// Struct to hold menu options
struct MenuOption
{
    void (*onTap)();
    void (*onHold)();
};

// Forward declarations
void onTap();
void onHold();

// Menu options for opening lid
MenuOption openMenuOptions[] = {
    {onTap, onHold}};

// Forward declarations
void displayMenu1();
void actionMenu1();
void displayMenu2();
void actionMenu2();
void displayMenu3();
void actionMenu3();

// Array of menu options
MenuOption menuOptions[] = {
    {displayMenu1, actionMenu1},
    {displayMenu2, actionMenu2},
    {displayMenu3, actionMenu3}};

// TouchSensor class definition
class TouchSensor
{
private:
    MenuOption *options;
    int numOptions;
    int currentMenu;
    unsigned long touchStartTime;
    const unsigned long holdTime = 1000;           // Time in milliseconds to consider a touch as a hold
    unsigned long lastInteractionTime;             // Time of the last interaction
    const unsigned long inactivityTimeout = 10000; // Time in milliseconds to reset to the initial menu (5 minutes)

public:
    void setTouchSensor(MenuOption *menuOptions, int num)
    {
        options = menuOptions;
        numOptions = num;
        currentMenu = 0; // Set current menu to the first option
        touchStartTime = 0;
        lastInteractionTime = millis(); // Initialize last interaction time
    }

    void handleTouch(boolean isTouched)
    {
        unsigned long currentTime = millis();

        if (isTouched)
        {
            if (touchStartTime == 0)
            {
                touchStartTime = currentTime;
            }
            else if (currentTime - touchStartTime > holdTime)
            {
                options[currentMenu].onHold();
                touchStartTime = 0;                // Reset touch start time
                lastInteractionTime = currentTime; // Update last interaction time
            }
        }
        else
        {
            if (touchStartTime > 0 && currentTime - touchStartTime < holdTime)
            {
                currentMenu = (currentMenu + 1) % numOptions;
                options[currentMenu].onTap();
                lastInteractionTime = currentTime; // Update last interaction time
            }
            touchStartTime = 0; // Reset touch start time
        }

        // Check for inactivity timeout
        if (currentTime - lastInteractionTime > inactivityTimeout)
        {
            setTouchSensor(openMenuOptions, sizeof(openMenuOptions) / sizeof(openMenuOptions[0])); // Display the first option of the initial menu
        }
    }
};

// Instantiate TouchSensor object
TouchSensor touchSensor;

// Function definitions
void onTap()
{
    Serial.println("Action for Open Lid");
}

void onHold()
{
    menuOptions[0].onTap();
    touchSensor.setTouchSensor(menuOptions, sizeof(menuOptions) / sizeof(menuOptions[0]));
}

// Functions to display and act on menus
void displayMenu1()
{
    Serial.println("Displaying Menu 1");
}

void actionMenu1()
{
    Serial.println("Action for Menu 1");
}

void displayMenu2()
{
    Serial.println("Displaying Menu 2");
}

void actionMenu2()
{
    Serial.println("Action for Menu 2");
}

void displayMenu3()
{
    Serial.println("Displaying Menu 3");
}

void actionMenu3()
{
    Serial.println("Action for Menu 3");
}

// Setup function
void setup()
{
    Serial.begin(9600);
    pinMode(TOUCH_PIN, INPUT);
    touchSensor.setTouchSensor(openMenuOptions, sizeof(openMenuOptions) / sizeof(openMenuOptions[0]));
}

// Loop function
void loop()
{
    boolean isTouched = digitalRead(TOUCH_PIN);
    touchSensor.handleTouch(isTouched);
}
