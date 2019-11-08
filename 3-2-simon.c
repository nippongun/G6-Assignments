/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
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

#if 0
void motor_backward_turn(u_int8_t l_speed,u_int8_t r_speed, u_int32_t delay){
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
        //motor_backward(speed,1);
    motor_turn(l_speed,r_speed,delay);
}
//Assignment 2, week 4 
void zmain(void){
    u_int8_t speed = 100;
    Ultra_Start();
    motor_start();
    motor_forward(0,0);
    vTaskDelay(200);
    
    int distance = 0;
    while(1){
        
        distance = Ultra_GetDistance();
        motor_forward(speed,500);
        if(distance <= 10){
            motor_backward_turn(speed/4,speed,1000);
        }
    }
}
#endif
/* [] END OF FILE */
