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
    
    TickType_t start, end;
    int general_speed = 100;
    
    motor_start();
    reflectance_start();
    IR_Start();
    reflectance_set_threshold(13000, 13000, 13000, 13000, 13000, 13000);
    while(SW1_Read() != 0)
        {
            vTaskDelay(100);
        }
    while(1)
    {
        reflectance_digital(&ref); 
        
        if(check == 0){
            
            while(ref.l1 != 1){
                //tank_turn(general_speed, RIGHT,1);
                motor_turn(general_speed+8,0,1);
                reflectance_digital(&ref); 
            }
            while(ref.r1 != 1){
                //tank_turn(general_speed, LEFT,1);
                motor_turn(0,general_speed,1);
                reflectance_digital(&ref); 
            }
        }
        if((ref.l3 == 1) && (ref.r3 == 1)&& (check == 0)){
            check = 1;
            x++;
            start = xTaskGetTickCount();    
        } else if((ref.l3 == 0) && (ref.r3 == 0) && (check != 0)){
            end = xTaskGetTickCount();
            motor_forward(general_speed,end - start);
            
            if( x == 2 && check == 1){
                while(ref.l1 == 0 || ref.r1 == 0){
                    tank_turn(general_speed,LEFT,1);
                    reflectance_digital(&ref);
                }
                motor_forward(0,500);
                check = 0;
            } 
            if(x == 1){
                motor_forward(0,0);
                IR_wait();
            }
            if((x == 3 || x == 4) && check == 1){ 
                
                while(ref.l1 != 0){
                    tank_turn(general_speed,RIGHT,1);
                    reflectance_digital(&ref);
                }
                
                motor_forward(0,500);
                
                while(ref.r1 != 1){
                    tank_turn(general_speed,RIGHT,1);
                    reflectance_digital(&ref);
                }
                motor_forward(0,500);
                check = 0;
            }      
            if(x == 5){
                motor_stop();
            }
            check = 0;
        }    
        motor_turn(general_speed+8,general_speed,1);
    }
 }
#endif
/* [] END OF FILE */
