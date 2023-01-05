#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "fancontroller.h"
#include <math.h>



void setup_i2c1(void)
{

    i2c_init(i2c1, 400000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}

void setup_gpio(void)
{

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    cyw43_arch_gpio_put(LED_PIN, 1);
}

ssd1306_t setup_oled(void)
{

    ssd1306_t disp;
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp);

    return disp;
}

void read_serial(char *strg)
{
    char readChar;
    do{
        readChar = getchar_timeout_us(0);
    }while(readChar != STARTCHARACTER);
    
    int n = 0;

    while((readChar = getchar_timeout_us(0)) != ENDCHARACTER || n < STRINGLENGTH){
        strg[n++] = readChar;
    }
}

void interpretSerialData(char *strg, bool isTestRunning,temperatureValue *currentTempData, temperatureValue *lastTempData, dutyCycle currentDutyCycle){
    if(sizeof(strg) == 3 && strg[0] == 91 && strg[2] == 93){
        switch (strg[1])
        {                            // different Letters can be used for macros such as
            case 'S': startupProcedure(); break;        // Start and
            case 'E': coolDownProcedure(currentDutyCycle, lastTempData); break;        // End  --> Cooldown Procedure
            default: if(isTestRunning)increasePWMSpeed(currentDutyCycle, lastTempData); break;         // invalid, a message can be sent, if a test is running the PWM Speed increases
        }
    }
    int isAllowedChar = 0;
    for(int i = 0; i < sizeof(strg); i++){
        for(int y = 0; y < sizeof(CONTROLCHARACTERS); y++)
        {
            if(strg[i] == CONTROLCHARACTERS[y]){
                isAllowedChar++; break;
            } 
        }
        if(strg[i] > 47 && strg[i] < 58){
            isAllowedChar++;
        }
    }
    if(isAllowedChar == sizeof(strg)){
        extractTemperatureData(strg, currentTempData);
    }
    else if(isTestRunning){
        // This means there is invalid data in the line, or there are letters on the line which
        // support may be added later on but not at the moment so for now, if a Test is running the
        // PWM Speed gets increases
        increasePWMSpeed(currentDutyCycle, lastTempData);
    }
}

void extractTemperatureData(char *strg, temperatureValue *tempDataArray)
{
    int offset = 0;
    
    uint8_t currentIndex = 0;

    int length = 0;
    char tempString[10];
    bool isControlCharacter = 0;

    for (int i = 1; i < POWERSUPPLY_COUNT; i++)
    {

        for (int y = 0; y < sizeof(CONTROLCHARACTERS); y++)
        {
            if (*strg + i + offset == CONTROLCHARACTERS[y])
            {
                isControlCharacter = 1;
            }
        }
        if (!isControlCharacter)
        {
            tempString[length++] = *strg + i + offset;
        }
        else
        {
            int value = 0;
            for (int n = 0; n < sizeof(tempString); n++)
            {
                value += pow(10.0, (double)n) * (int)tempString[sizeof(tempString) - n];
            }
            tempDataArray[currentIndex++].Value = value;
            offset += 1 + sizeof(tempString);
            isControlCharacter = 0;
        }
    }

    //TemperatureDataBlock currentTempData = {getAverage(tempDataArray), getMaxTemp(tempDataArray), getRunningPSUs(tempDataArray)};
    /*
        This Data contains all the neccessary information required to fuel the next steps, so just transmitting this pointer and not
        the big one with the entire TempDataValues is more memory efficient
    */
}

uint16_t checkDataValidity(temperatureValue *tempData)
{

    uint16_t dataValidBytes = 0;

    return dataValidBytes;
}

dutyCycle determinePWMSpeed(uint16_t dVB, temperatureValue *tempData, temperatureValue *lastTempData, dutyCycle currentDutyCycle)
{
    dutyCycle dutyCycle = {0};

    

    return dutyCycle;
}

dutyCycle increasePWMSpeed(dutyCycle currentDutyCycle, temperatureValue *lastTempData)
{
    dutyCycle newDutyCycle = {currentDutyCycle.Value + 10};

    float avgTemperature = getAverage(lastTempData);
    float maxTemperature = getMaxTemp(lastTempData);

    if(maxTemperature > SINGLETEMPCUTOFF){
        newDutyCycle.Value += PWMSPEEDINCREASE;
    }
    else if(avgTemperature > AVGTEMPCUTOFF){
        newDutyCycle.Value += PWMSPEEDINCREASE;
    }
    
    clamp(&newDutyCycle, BOTTOMCLAMPBORDER, TOPCLAMPBORDER);

    return newDutyCycle;
}

dutyCycle setPWMOutput(dutyCycle dC, uint8_t Pin)
{
    dutyCycle newDutyCycle = {0};

    return newDutyCycle;
}

void coolDownProcedure(dutyCycle currentDutyCycle, temperatureValue *lastTempData)
{
    /**
     *      Lets the Fans run for a little more depending on the Previous Temperatures
     */
}

void startupProcedure(void){
    /**
     *      Is there even a lot needed for this?
    */
}

float getAverage(temperatureValue *Values){
    float temp = 0.0;
    for(int i = 0; i < sizeof(Values); i++)
    {
        temp += Values[i].Value;
    }
    return temp / sizeof(Values);
}

float getMaxTemp(temperatureValue *Values)
{
    float temp = Values[0].Value;

    for(int i = 1; i < sizeof(Values); i++)
    {
        if(Values[i].Value > temp){
            temp = Values[i].Value;
        }
    }
    return temp;
}

int getRunningPSUs(temperatureValue *Values){
    int counter = 0; 
    for(int i = 0; i< sizeof(Values); i++)
    {
        if(Values[i].Value < PSUOFFTEMPERATURE){
            counter++;
        }
    }
    return counter;
}

void clamp(dutyCycle *Value, int bottomBorder, int topBorder){
    if(Value->Value > topBorder) Value->Value = topBorder;
    if(Value->Value < bottomBorder) Value->Value = bottomBorder;
}

int main(){

};