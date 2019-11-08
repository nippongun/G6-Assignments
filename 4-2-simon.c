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
/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/
#if 0
    
#define RIGHT 0
#define LEFT 1
    
void tank_turn(uint8_t speed, int dir, int delay){
    if(dir == 0 ){
        //tank turn right
        MotorDirLeft_Write(0);
        MotorDirRight_Write(1);
    } else if (dir == 1){
        //tank turn left
        MotorDirLeft_Write(1);
        MotorDirRight_Write(0);
    }
    motor_turn(speed,speed,delay);
}
void zmain(void)
 {
    struct sensors_ ref;
    struct sensors_ dig;
    int x = 0;
    int check = 0;
    
    int turn_delay = 580;
    int general_speed = 100;
    
    motor_start();
    reflectance_start();

    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    while(SW1_Read() != 0)
        {
            vTaskDelay(100);
        }
    while(1)
    {
        reflectance_read(&ref); 
         
        if((ref.l2 >= 13000) && (ref.r2 >= 13000)&& check == 0){
            check = 1;
            x++;
            
            if ( x == 5 ){
                motor_stop();
            }
            
            motor_turn(general_speed+8,general_speed,100);
            
            if(x==2){
               tank_turn(general_speed,LEFT,turn_delay);
               motor_forward(0,500);
            } else if(x == 3 || x == 4){
                tank_turn(general_speed+3,RIGHT,turn_delay);
                motor_forward(0,500);
            }
        } else if((ref.l2 < 13000) && (ref.r2 < 13000)){
            check = 0;
        }
        motor_turn(general_speed+8,general_speed,0);
    }
 }
#endif
/* [] END OF FILE */
