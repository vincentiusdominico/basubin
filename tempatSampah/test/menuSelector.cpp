#include <Arduino.h>

class TouchMenuController
{
public:
    TouchMenuController(int touchPin, unsigned long debounceTime = 50, unsigned long holdThreshold = 1000)
        : touchPin(touchPin), debounceTime(debounceTime), holdThreshold(holdThreshold),
          lastTouchState(LOW), currentTouchState(LOW), lastDebounceTime(0), touchStartTime(0), holdFlag(false)
    {
        pinMode(touchPin, INPUT);
        // Initialize menu structure
        initializeMenu();
    }

    // This function returns 0 if no action, 1 if menu changed, and 2 if menu selected
    int update()
    {
        int reading = digitalRead(touchPin);
        int result = 0; // Default no action

        // Debounce the touch input
        if (reading != lastTouchState)
        {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceTime)
        {
            // If the state has changed
            if (reading != currentTouchState)
            {
                currentTouchState = reading;

                // If the touch sensor is touched
                if (currentTouchState == HIGH)
                {
                    touchStartTime = millis();
                    holdFlag = false; // Reset the hold flag
                }
                else
                {
                    // If the touch sensor is released
                    if (!holdFlag)
                    {
                        // Only process tap if hold has not been triggered
                        unsigned long touchDuration = millis() - touchStartTime;
                        if (touchDuration < holdThreshold)
                        {
                            // Short press detected (Tap)
                            currentOption = (currentOption + 1) % numOptions;
                            Serial.print("Menu option changed to: ");
                            Serial.println(currentOption);
                            result = 1; // Menu changed
                            // Reset the timeout countdown
                            lastActionTime = millis();
                        }
                    }
                }
            }

            // Check for hold condition
            if (currentTouchState == HIGH && !holdFlag && (millis() - touchStartTime >= holdThreshold))
            {
                // Long press detected (Hold)
                holdFlag = true;
                Serial.print("Option selected: ");
                Serial.println(currentOption);
                result = 2; // Menu selected
                // Reset the timeout countdown
                lastActionTime = millis();
            }
        }

        lastTouchState = reading;

        // Check for timeout to reset menu to initial state
        if (millis() - lastActionTime > timeoutDuration)
        {
            resetMenu();
            Serial.println("Menu reset due to inactivity.");
        }

        return result; // Return the result of the update
    }

    int getCurrentOption()
    {
        return currentOption;
    }

    void resetMenu()
    {
        currentOption = 0;
        lastActionTime = millis(); // Reset last action time
    }

    void setNumOptions(int numOptions)
    {
        this->numOptions = numOptions;
    }

    void initializeMenu()
    {
        // Initialize menu with default values
        numOptions = 3;            // Default number of options
        timeoutDuration = 300000;  // 5 minutes timeout (300000 milliseconds)
        lastActionTime = millis(); // Initialize last action time
    }

private:
    int touchPin;
    unsigned long debounceTime;
    unsigned long holdThreshold;
    int numOptions;
    int currentOption;

    bool lastTouchState;
    bool currentTouchState;
    unsigned long lastDebounceTime;
    unsigned long touchStartTime;
    bool holdFlag; // Flag to indicate hold has been processed

    unsigned long timeoutDuration; // Timeout duration in milliseconds
    unsigned long lastActionTime;  // Last action time in milliseconds
};

// Define the pin connected to TTP223
const int touchPin = 14;

// Create an instance of the TouchMenuController class
TouchMenuController menuController(touchPin);

void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
}

void loop()
{
    // Update the menu controller and get the result
    int action = menuController.update();

    // Use the result in other logic
    if (action == 1)
    {
        // Menu changed
        // Add your code here for menu change action
    }
    else if (action == 2)
    {
        // Menu selected
        int selectedOption = menuController.getCurrentOption();
        // Perform action based on selected menu
        Serial.print("Selected option: ");
        Serial.println(selectedOption);
        // Reset menu for next selection
        menuController.resetMenu();
    }
}
