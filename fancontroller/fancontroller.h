#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include "ssd1306.h"

#define POWERSUPPLY_COUNT 10
#define LED_PIN CYW43_WL_GPIO_LED_PIN
#define FANCURVE_DATASETS 10

typedef struct{
    uint8_t Value;
} temperatureValue;

typedef struct{
    uint8_t Value;
} dutyCycle;

typedef struct{
    uint8_t Value;
} SpeedModifier;

typedef struct{
    float AverageTemp;
    float MaxTemp;
    int PSUAmt;
} TemperatureDataBlock;



typedef struct{
    uint8_t Values[FANCURVE_DATASETS][2];
    uint8_t dataSetAmount;
} FanCurve;

const int STARTCHARACTER = 91;
const int ENDCHARACTER = 93;
const int STRINGLENGTH = 64;
const int CONTROLCHARACTERS[] = {59};

const int AVGTEMPCUTOFF = 60;
const int SINGLETEMPCUTOFF = 70;
const int PWMSPEEDINCREASE = 10;

const int TOPCLAMPBORDER = 100;
const int BOTTOMCLAMPBORDER = 0;

const FanCurve SINGLEPSUACTIVE = {};
const FanCurve MULTIPLEPSUACTIVE = {};

const FanCurve COOLDOWNCURVE ={};

const float PSUOFFTEMPERATURE = 20.0f;

/**
 *      @author Lars Hoehne
 *      @date January 2023
*/

/**
 *      @brief initializes the i2c1
 * 
*/
void setup_i2c1(void);
/**
 *      @brief initializes the on board LED
*/
void setup_gpio(void);
/**
 *      @brief initializes the oled display
 *      
 *      @retval instance of the display
*/
ssd1306_t setup_oled(void);
/**
 *      @brief retreives data from the serial port
 * 
 *      @retval string read from serial 
*/
void read_serial(char *strg);
/**
 *      @brief Turns the String into a temperatureValue Array, seperated by the controlCharacters
 *     
 *      @param[in] strg: inputString with the temperature Data
 *      @param[in] controlCharacter: Each char in this array leads to the String to be interrupted at that part and a new data Set to be Started
 *      @param[in] tempDataArray: This is the Pointer to which the data will be written
 * 
*/
void extractTemperatureData(char *strg, temperatureValue *tempDataArray);
/**
 *      @brief Determine what the Data should be used for
 *      
 *      @param[in] strg: inputString with either temperature Data, Data to signalize the start of a Test, Data to signalize the end of a Test, or invalid data
 *      @param[in] isTestRunning: bool if a Test is running
 *      @param[in] currentTempData: The pointer to which the new temperature data will be written to
 *      @param[in] lastTempData: pointer to an array with the previous temperatureValues
 *      @param[in] currentDutyCycle: The current DutyCycle the Fans are running at
*/
void interpretSerialData(char *strg, bool isTestRunning,temperatureValue *currentTempData, temperatureValue *lastTempData, dutyCycle currentDutyCycle);
/**
 *      @brief  checks if the temperature data is valid
 * 
 *      @param[in] tempData : pointer to an array of int values
 * 
 *      @retval dataValidBytes
*/
uint16_t checkDataValidity(temperatureValue *tempData);
/**
 *      @brief calculates the PWM Speed
 * 
 *      @param[in] dVB: dataValidBytes
 *      @param[in] tempData: pointer to an array of current temperatureValues
 *      @param[in] lastTempData: pointer to an array with the previous temperatureValues
 *      @param[in] currentDutyCycle: The current DutyCycle the Fans are running at
 * 
 *      @retval DutyCycle
*/
dutyCycle determinePWMSpeed(uint16_t dVB, temperatureValue *tempData, temperatureValue *lastTempData, dutyCycle currentDutyCycle);
/**
 *      @brief increases the PWM Speed by a const amount e.g. PWMSPEEDINCREASE
 * 
 *      @param[in] currentDutyCycle: The current DutyCycle the Fans are running at
 *      @param[in] lastTempData: pointer to an array with the previous temperatureValues
 * 
 *      @retval Returns the new DutyCycle
*/
dutyCycle increasePWMSpeed(dutyCycle currentDutyCycle, temperatureValue *lastTempData);
/**
 *      @brief controls the Fanspeed task and can start, stop and hold the Task this PWM Task will run on the second core only
 * 
 *      @param[in] dC: DutyCycle
 *      @param[in] Pin: The Pin connected to the PWM Device
*/
dutyCycle setPWMOutput(dutyCycle dC, uint8_t Pin);
/**
 *      @brief A procedure to safely cool down the system
 * 
 *      @param currentDutyCycle: The current DutyCycle the Fans are running at
 *      @param lastTempData: pointer to an array with the previous temperatureValues
*/
void coolDownProcedure(dutyCycle currentDutyCycle, temperatureValue *lastTempData);
/**
 *      @brief A procedure to safely start the system
*/
void startupProcedure(void);
/**
 *      @brief Returns the Average of a given temperatureValue based array
 * 
 *      @param[in] Values: this is a pointer to the Array of temperatureValues of which the average should be calculated
 * 
 *      @retval The Average as a float value
*/
float getAverage(temperatureValue *Values);
/**
 *      @brief Returns the highest Value in a temperatureValue Array
 * 
 *      @param[in] Values: this is a pointer to the Array of temperatureValues of which the maximum value should be found
 * 
 *      @retval The Highest Temperature Value
*/
float getMaxTemp(temperatureValue *Values);
/**
 *      @brief Checks how many PSUs are active by comparing their TempValues to a const e.g. PSUOFFTEMPERATURE
 * 
 *      @param[in] Values: this is a pointer to the Array of temperatureVales to determine how many of them are turned on
 * 
 *      @retval Returns the amount of PSUs turned on as an int
*/      
int getRunningPSUs(temperatureValue *Values);
/**
 *      @brief Clamps the value of an int inbetween the 2 borders
 * 
 *      @param[in] Value: Pointer to the int to be clamped
 *      @param[in] bottomBorder: The lowest the Value can go
 *      @param[in] topBorder: The highest the Value can go 
*/
void clamp(dutyCycle *Value, int bottomBorder, int topBorder);