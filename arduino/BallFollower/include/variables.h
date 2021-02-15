#pragma once
#include <elapsedMillis.h>
#include <Servo.h>
#include <Wire.h>
#include <PID_v1.h>

#define SCB_AIRCR (*(volatile uint32_t *)0xE000ED0C) // Application Interrupt and Reset Control location


// communication over serial
struct dataPacket 
{
    char message[64] = {0};
    float first = 0.0f;
    float second = 0.0f;
    float third = 0.0f;
};


//Timing
// elapsedMillis elapsedTime;
const int MAX_DISTANCE = 200; // Distance in Centymeters
const uint16_t MAX_POWER = 255;
const uint16_t MIN_POWER = 0;  

// constants for motor driver
const uint8_t IN1=25;
const uint8_t IN2=26;
const uint8_t ENA=23;

const uint8_t IN3=27;
const uint8_t IN4=28;
const uint8_t ENB=22;

//Tuning motor parameters (Experimental setup)
double KpM = 3.7; // * (P)roportional motor tuning
double KiM = 0.15; // * (I)integral motor tuning
double KdM = 0.05;  // * (D)erivative  motor tuning
 
//Tuning servo parameters 
float Kp = 20.0f;
float Ki = 8.0f;
float Kd = 0.5f;

// Servos
float yawErrorAccumulated = 0;
float pitchErrorAccumulated = 0;     // setpoint is rotational speed in Hz  

// Motors PID
const unsigned long CALCULATION_PERIOD = 50; 
const unsigned long INT_COUNT = 20;    

double setpointRight = 0;       
double requestedInputRight = 0;        
double calculatedOutPutRight = 0; 
double setpointLeft = 0; 
double requestedInputLeft = 0;  
double calculatedOutPutLeft = 0;
//============
PID motorRight(&requestedInputRight, &calculatedOutPutRight, &setpointRight, KpM, KiM, KdM, DIRECT);  
PID motorLeft(&requestedInputLeft, &calculatedOutPutLeft, &setpointLeft, KpM, KiM, KdM, DIRECT);  

// direction currentDirection;
bool isBreaking = false;
bool isStopped = false;


//constants for ESP8266
const uint8_t ESP8266_RST = 29, ESP8266_CH_PD = 30;

const uint8_t Lidar_XSHUT = 36;

enum class EngineSelector
{
	Left,
	Right
};

enum class ServoSelector{
    Yaw,
    Pitch
};

//constants for servos
//left-right
Servo servoYaw;
int yawMin = 45, yawMax = 135, yawCenter = 80, yawCurrent = 80;
//up-down
Servo servoPitch;
int pitchMin = 45, pitchMax = 135, pitchCenter = 58, pitchCurrent = 58;

// Theoretical speed of the servo
// 0.23s / 60 degrees (at 4.8V, no load)
float servoMillisecondsPerDegree = 20;
float damping = 0.01;
int deadZone = 5;

elapsedMillis yawMoveTimer;
elapsedMillis pitchMoveTimer;

bool isYawServoMoving = false;
bool isPitchServoMoving = false;


unsigned int servoWaitTimeYaw = 0;
unsigned int servoWaitTimePitch = 0;




