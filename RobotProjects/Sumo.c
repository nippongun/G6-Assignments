/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

#include <project.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "LSM303D.h"
#include "IR.h"
#include "Beep.h"
#include "mqtt_sender.h"
#include <time.h>
#include <sys/time.h>
#include "serial1.h"
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/

//Sumo  
#if 1
void motor_tankR(uint8 speed,uint32 delay);     //Tank turn to the right
float quadrant1(float hit_x, float hit_y);      //Calculates the angle of hit between 0-90 degrees
float quadrant2(float hit_x, float hit_y);      //Calculates the angle of hit between 90-180 degrees
float quadrant3(float hit_x, float hit_y);      //Calculates the angle of hit between 180-270 degrees
float quadrant4(float hit_x, float hit_y);      //Calculates the angle of hit between 270-360 degrees
void zmain(void)
{
    IR_Start();
    reflectance_start();
    LSM303D_Start();
    motor_start();
    TickType_t start;
    TickType_t end;
    struct sensors_ dig;
    struct accData_ data;
    float hit_x = 0;
    float hit_y = 0;
    int time;
    int time2;
    int condition;
    bool check = 0;
    reflectance_set_threshold(9500, 9000, 11000, 11000, 9000, 9500);
    
    while(SW1_Read() != 0)      //While button is not pressed robot is in TaskDelay
    {
        vTaskDelay(100);
    }
    
    start = xTaskGetTickCount();
    reflectance_digital(&dig);
   
    while((dig.l3 == 0 || dig.r3 == 0) && (dig.l1 == 1 || dig.r1 == 1))     //While it's reading the black line it will go forward
    {
        motor_forward(100,0);
        reflectance_digital(&dig);
    }
    
    reflectance_digital(&dig); 
    
    while((dig.l3 == 1) && (dig.l2 == 1) && (dig.r2 == 1) && (dig.r3 == 1)) //While it's reading move forward till it reads white and stops
    {        
        motor_forward(100,0);
        reflectance_digital(&dig); 
    }
    
    //When the robot sees a white line it stops and wait for IR signal    
    motor_forward(0,0);
    print_mqtt("Zumo061/ready","line");    
    IR_wait(); 
    end = xTaskGetTickCount();
    time2 = end - start;
    print_mqtt("Zumo061/start","%d",time2);
    condition = 0;
    
    while(true)
    {    
        reflectance_digital(&dig);
        
        while((dig.l3 == 0) && (dig.r3 == 0))       //While it's reading white goes forward, meant for inside the ring
        {
            motor_forward(175,0);
            LSM303D_Read_Acc(&data);
            reflectance_digital(&dig); 
            hit_x = data.accX;
            hit_y = data.accY;
            
            if(!SW1_Read() )        //If the button pressed it will print the end time, total time
            {
            end = xTaskGetTickCount();
            time = end - start; 
            print_mqtt("Zumo061/end","%d",time);
            print_mqtt("Zumo061/time","%d",time - time2);  
            check = 1;
            while(check)        //While check is true the robot won't move, it's trigged by the button press
                {
                   motor_forward(0,0);    
                }
            }
            
            else if((hit_x > 9800 && hit_y > 9800) && condition == 0)       //Calculates the angle of hit between 0-90 degrees
            {               
                end = xTaskGetTickCount();
                time = end - start;
                quadrant1(hit_x, hit_y);
                print_mqtt("Zumo061/hit","%d %.0f",time, quadrant1(hit_x, hit_y));
                condition = 1;
            }
            
            else if((hit_x >  9800 && hit_y < -9800) && condition == 0)     //Calculates the angle of hit between 90-180 degrees
            {               
                end = xTaskGetTickCount();
                time = end - start;
                quadrant2(hit_x, hit_y);
                print_mqtt("Zumo061/hit","%d %.0f",time, quadrant2(hit_x, hit_y));
                condition = 1;
            }
            
            else if((hit_x < -9800 && hit_y < -9800) && condition == 0)     //Calculates the angle of hit between 180-270 degrees
            {                
                end = xTaskGetTickCount();
                time = end - start;
                quadrant3(hit_x, hit_y);
                print_mqtt("Zumo061/hit","%d %.0f",time, quadrant3(hit_x, hit_y));                
                condition = 1;
            }
            
            else if((hit_x <  -9800 && hit_y > 9800) && condition == 0)     //Calculates the angle of hit between 270-360 degrees
            {
                end = xTaskGetTickCount();
                time = end - start;
                quadrant4(hit_x, hit_y);
                print_mqtt("Zumo061/hit","%d %.0f",time, quadrant4(hit_x, hit_y));                  
                condition = 1;
            }
        }
        
        if(dig.l3 == 1 || dig.r3 == 1 || dig.r2 == 1 || dig.l2 == 1)        //When the robot reads black line it go sligthy backward and then turn
        {           
            condition = 0;
            motor_backward(200,150);
            motor_tankR(220,300);     
            reflectance_digital(&dig);
        }  
    }
}
void motor_tankR(uint8 speed,uint32 delay)
{
    MotorDirLeft_Write(0);      // set LeftMotor forward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(speed); 
    vTaskDelay(delay);
}
float quadrant1(float hit_x, float hit_y)
{
    float value_hit;
    float angle;
    value_hit = hit_x/hit_y;        //Makes it possible to calculates the arctan
    angle = ((atan(value_hit))*180)/M_PI;       //Transform radians into deg
    return angle;
}
float quadrant2(float hit_x, float hit_y)
{
    float value_hit;
    float angle;
    value_hit = hit_x/hit_y;
    angle = 180 + (((atan(value_hit))*180)/M_PI);       //To make it possible to calculate on quadrant 2  
    return angle;
}
float quadrant3(float hit_x, float hit_y)
{
    float value_hit;
    float angle;
    value_hit = hit_x/hit_y;
    angle = 180 + (((atan(value_hit))*180)/M_PI);       //To make it possible to calculate on quadrant 3   
    return angle;
}
float quadrant4(float hit_x, float hit_y)
{
    float value_hit;
    float angle;
    value_hit = hit_x/hit_y;
    angle = 360 + (((atan(value_hit))*180)/M_PI);       //To make it possible to calculate on quadrant 4
    return angle;
}
#endif

/* [] END OF FILE */
