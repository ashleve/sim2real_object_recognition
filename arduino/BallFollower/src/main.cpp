#include <Arduino.h>
#include <Ping.h>
#include <Encoder.h>
#include <string>
#include "NewPing.h"
#include "ISAMobile.h"
#include "variables.h"
#include "functions.hpp"

enum Option
{
    SERVO,
    STOP,
    START,
    MOTOR,
    NONE,
};

Encoder rightSide(ENCODER_REAR_RIGHT_1, ENCODER_REAR_RIGHT_2);
Encoder leftSide(ENCODER_REAR_LEFT_1, ENCODER_REAR_LEFT_2);

NewPing sonarMiddle(ultrasound_trigger_pin[(int)UltraSoundSensor::Front],
                    ultrasound_echo_pin[(int)UltraSoundSensor::Front],
                    MAX_DISTANCE);

unsigned int pingDelay = 100; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;      // Holds the next ping time.

const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars]; // temporary array for use when parsing

dataPacket packet;

boolean hasReceivedNewData = false;

float yawRequested = 0;
float pitchRequested = 0;

void recvWithStartEndMarkers();

dataPacket parseData();

void showParsedData(dataPacket);

void moveMotors(double, double);

void calculateMotors();

void setup()
{

    initSerial(115200);   // Starting Serial Terminal
    pingTimer = millis(); // Start now
    Serial.println("This demo expects 3 pieces of data - text, an integer and a floating point value");
    Serial.println("Enter data in this style <HelloWorld, 12, 24.7>  ");
    Serial.println();

    initMotors();
    initPID();

    // Each platform has to do this independently, checked manually
    calibrateServo(ServoSelector::Yaw, (int)yawCenter);
    calibrateServo(ServoSelector::Pitch, (int)pitchCenter);

    initServos();
    centerServos();

    initESP826();

    setpointLeft = 20;
    setpointRight = 20;

    requestedInputLeft = 20;
    requestedInputRight = 20;

    delay(2000);

    Serial.println("Initalization ended");
}

long positionLeft = 0;
long positionRight = 0;
bool once = false;

void echoCheck();

Option resolveOption(String);

// * Checking if agent is too close to an obstacle. If it is too close then it will change speed
void dodgeObstacles();

// * Update sonar ping results in given time
void updateSonarsResults();

// * parse input data
void calibrateServos();

// * [dataPacket.first] is motorLeft value
// * [dataPacket.second] is motorRight value
// * This means that when you sending request responsible for changing motor value you have to set
// * this like:  <motor, 12, 24.7> left side will be with value 12 and right side will have 24.7

bool printParseData = false;
bool showMotorPositions = false;
bool showDistanceToObstacle = false;

// * Main loop
void loop()
{

    //moveMotors(50,50);
    calculateMotors();
    //Serial.println(calculatedOutPutLeft);

    updateSonarsResults();
    recvWithStartEndMarkers();

    if (hasReceivedNewData == false)
        return;

    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    packet = parseData();
    if (printParseData)
    {
        showParsedData(packet);
    }

    switch (resolveOption(packet.message))
    {
    case SERVO:
        calibrateServos();
        break;
    case STOP:
        //moveMotors(0.0, 0.0);
        isStopped = true;
        break;
    case START:
        isStopped = false;
        break;
    case MOTOR:
        //moveMotors(packet.first, packet.second);
        break;
    default:
        break;
    }
    hasReceivedNewData = false;
    dodgeObstacles();
}

void updateSonarsResults() // * Updating middle sonars results [sonarMiddle]. Should be call first in the loop()
{
    if (millis() >= pingTimer)
    {                                      // pingSpeed milliseconds since last ping, do another ping.
        pingTimer += pingDelay;            // Set the next ping time.
        sonarMiddle.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping statu
        if (showMotorPositions)
        {
            positionLeft = leftSide.read();
            positionRight = rightSide.read();
            Serial.println("###  Motor-positions  ##########################################");
            Serial.println("Left: " + String(positionLeft));
            Serial.println("Right: " + String(positionRight));
            Serial.println("################################################################");
        }
    }
}

void echoCheck()
{ // Timer interrupt calls this function every 24uS where you can check the ping status.
    // Don't do anything here!
    if (sonarMiddle.check_timer())
    { // This is how you check to see if the ping was received.
        // Here's where you can add code.
        // Serial.print("Ping: ");
        // Serial.println("cm");
    }
    // Don't do anything here!
}

void recvWithStartEndMarkers() // * Marking if has new Data in bool [hasReceivedNewData]. And saving data in [receivedChars] array.
{

    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && hasReceivedNewData == false)
    {
        rc = Serial.read();

        if (recvInProgress == true)
        {
            if (rc != endMarker)
            {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars)
                {
                    ndx = numChars - 1;
                }
            }
            else
            {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                hasReceivedNewData = true;
            }
        }

        else if (rc == startMarker)
        {
            recvInProgress = true;
        }
    }
}

dataPacket parseData() // * split the data into its parts. The "," is splitter formula.
{

    dataPacket tmpPacket;

    char *strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars, ",");   // get the first part - the string
    strcpy(tmpPacket.message, strtokIndx); // copy it to messageFromPC

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
   if (NULL == strtokIndx)
        return tmpPacket;
    tmpPacket.first = atof(strtokIndx);

    strtokIndx = strtok(NULL, ",");
    if (NULL == strtokIndx)
        return tmpPacket;

    tmpPacket.second = atof(strtokIndx);
    strtokIndx = strtok(NULL, ",");

    if (NULL == strtokIndx)
        return tmpPacket;
    tmpPacket.third = atof(strtokIndx);

    return tmpPacket;
}

Option resolveOption(String input) // * Require [Option] enum.
{
    if (input == "servo")
        return SERVO;
    else if (input == "stop")
        return STOP;
    else if (input == "start")
        return START;
    else if (input == "motor")
        return MOTOR;
    return NONE;
}

void calibrateServos()
{
    if (isStopped == false)
        return;

    yawRequested = packet.first;
    pitchRequested = packet.second;
    double distance = packet.third;
    {
        // float yawError = -(yawRequested / (HorizontalFOV/2) );
        float yawError = -yawRequested;
        float Kp = 25.0f;
        float Ki = 4.0f;

        float output = Kp * yawError + Ki * yawErrorAccumulated;
        yawErrorAccumulated += yawError;

        moveServo(ServoSelector::Yaw, (int)(yawCenter + output));
    }
    {

        // float pitchError = -(pitchRequested / (VerticalFOV/2));
        float pitchError = -pitchRequested;
        float Kp = 15.0f;
        float Ki = 3.0f;

        float output = Kp * pitchError + Ki * pitchErrorAccumulated;
        pitchErrorAccumulated += pitchError;

        // move servo
        moveServo(ServoSelector::Pitch, (int)(pitchCenter + output));
    }
    double distanceInCM = sonarMiddle.ping_result / US_ROUNDTRIP_CM;
    if (distanceInCM < 30 || distanceInCM == 0)
    {
        requestedInputRight = setpointRight + 5;
        requestedInputLeft = setpointLeft + 5;
    }else
    {
         const double yawInputMotorsError = (yawCenter - yawCurrent) * 0.5;
                        const double inputMotors = distance * 1000;
                
                        // inputLeftBack = -inputMotors;
                        // inputRightBack = inputMotors;

                        requestedInputLeft = inputMotors - yawInputMotorsError;
                        requestedInputRight = inputMotors + yawInputMotorsError;


                        motorRight.Compute();  
                        motorLeft.Compute(); 

                        
                        // SetPowerLevel(EngineSelector::Left, outputLeftBack);
                        // SetPowerLevel(EngineSelector::Right, outputRightBack);
                        SetPowerLevel(EngineSelector::Left, calculatedOutPutLeft);
                        SetPowerLevel(EngineSelector::Right, calculatedOutPutRight);
                    

                        // Serial.printf("yaw: %d, inputMotors: %f\n", yawCurrent, inputMotors);
                        // Serial.printf("Computed A output %d\n", leftSide.read());
                        // Serial.printf("Computed B output %d\n", rightSide.read());
    }
}

void dodgeObstacles()
{
    int distanceInCM = sonarMiddle.ping_result / US_ROUNDTRIP_CM;
    if (showDistanceToObstacle)
    {
        Serial.println("###  Distance-to-obstacle  #####################################");
        Serial.println("Ping: " + String(distanceInCM) + " cm");
        Serial.println("################################################################");
    }
    if (distanceInCM < 30 || distanceInCM == 0)
    {
        requestedInputRight = setpointRight + 5;
        requestedInputLeft = setpointLeft + 5;
    }
}

// * Printing parsed data
void showParsedData(dataPacket packet)
{
    Serial.println("################################################################");
    Serial.print("Message ");
    Serial.println(packet.message);
    Serial.print("first ");
    Serial.println(packet.first);
    Serial.print("second ");
    Serial.println(packet.second);
    Serial.println("################################################################");
}

bool printMotorsPower = true;

void moveMotors(double leftMotor, double rightMotor)
{
    if (isStopped)
        return;

    setpointLeft = leftMotor;
    setpointRight = rightMotor;
    
    calculateMotors();
}

void calculateMotors(){
    if (isStopped)
        return;

    motorRight.Compute();
    motorLeft.Compute();

    if(printMotorsPower){
        Serial.println(calculatedOutPutLeft);
        Serial.println(calculatedOutPutRight);
    }

    MotorL_Move(calculatedOutPutLeft);
    MotorR_Move(calculatedOutPutRight);
}