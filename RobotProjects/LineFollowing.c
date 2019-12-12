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

// Line following
#if 1
void zmain(void)
 { 
    struct sensors_ dig;
    TickType_t start;
    TickType_t end;
    int time;
    int time1;
    bool onLine = 1;
    reflectance_start();
    motor_start();
    IR_Start();
    int count = 0;
    reflectance_set_threshold(9500, 9000, 11000, 11000, 9000, 9500);
    
    while(SW1_Read() != 0)      //While button is not pressed robot is in TaskDelay
    {
        vTaskDelay(100);
    }
    
    start = xTaskGetTickCount();
    reflectance_digital(&dig);
    
    while((dig.l3 == 0 || dig.r3 == 0) && (dig.l1 == 1 || dig.r1 == 1))      //While it's reading the black line it will go forward
    {
        motor_forward(100,0);
        reflectance_digital(&dig);
    }
    
    if((count == 0) && (dig.l3 == 1) && (dig.l2 == 1) && (dig.r2 == 1) && (dig.r3 == 1))        //Counts first line
    {
        reflectance_digital(&dig); 
        count++;
        while(dig.l3 == 1 || dig.r3 == 1)       //Goes forwards on the first line
        {
            motor_forward(100,0);
            reflectance_digital(&dig); 
        }
    }
    
    //When the robot sees a white line it stops and wait for IR signal
    motor_forward(0,0);
    print_mqtt("Zumo061/ready", "line");
    IR_wait();    
    end = xTaskGetTickCount();
    time1 = end - start;
    print_mqtt("Zumo061/start", "%d",time1);
    
    while(1) 
    {
        reflectance_digital(&dig);
        if((dig.l1 == 0 && dig.r1 == 0) && onLine)      //Reads if goes out of the line(middle sensors)
        {
            print_mqtt("Zumo061/miss", "%d",xTaskGetTickCount());
            onLine = 0;
        }
        
        if((dig.l1 == 1 && dig.r1 == 1) && !onLine)
        {
            print_mqtt("Zumo061/line", "%d",xTaskGetTickCount());       //Reads if goes out of the line(middle sensors)
            onLine = 1;
        }        

        reflectance_digital(&dig);
        while((dig.l1 == 1 && dig.r1 == 1) && (dig.r2 == 0 || dig.l2 == 0))     //Go forward
        {       
            reflectance_digital(&dig);
            motor_forward(187,0);
        }
            
        while((dig.l1 == 0)&&(dig.r2 == 1))     //Turn Right
        {
            reflectance_digital(&dig);
            motor_turn(240,0,0);
        }

        while((dig.r1 == 0)&&(dig.l2 == 1))     //Turn Left    
        {
            reflectance_digital(&dig);        
            motor_turn(0,240,0);
        }
       
        if((count == 1)&&(dig.l3 == 1) && (dig.l2 == 1) && (dig.r2 == 1) && (dig.r3 == 1))      //Reads the first ending line 
        {
            reflectance_digital(&dig); 
            count++;
            while((dig.l3 == 1) && (dig.l2 == 1) && (dig.r2 == 1) && (dig.r3 == 1))     //Goes forwards on the first ending line
            {
            
                reflectance_digital(&dig); 
                motor_forward(200,0);
            }
        }
        reflectance_digital(&dig); 
        
        if((count == 2)&&(dig.l3 == 1) && (dig.l2 == 1) && (dig.r2 == 1) && (dig.r3 == 1))      //Reads the ending line and print end, total time
        {
            reflectance_digital(&dig); 
            end = xTaskGetTickCount();
            time = end - start;
            print_mqtt("Zumo061/end", "%d",time);
            print_mqtt("Zumo061/time", "%d",time - time1);
            reflectance_digital(&dig); 
            while((dig.l3 == 1) && (dig.l2 == 1) && (dig.r2 == 1) && (dig.r3 == 1))     //Stop and stay in the ending line
            {
            reflectance_digital(&dig);
            motor_stop();
            }
        }
    }
}
#endif 
/* [] END OF FILE */
