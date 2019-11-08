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
   void zmain(){
        struct accData_ data;
        LSM303D_Start();
        motor_start();
        uint8_t speed = 100;
        uint8_t l_speed, r_speed;
        while(SW1_Read()!=0){
            vTaskDelay(50);
        }
        while(1){
            l_speed = (rand() % (150 - 70 + 1)) + 70;
            r_speed = (rand() % (150 - 70 + 1)) + 70;
            for(int i = 0; i < 1000; i++){
                motor_turn(l_speed, r_speed,1);
                LSM303D_Read_Acc(&data);
                if(data.accX < -4000){
                    printf("x:%10d y:%10d\n",data.accX,data.accY);
                    motor_backward(speed,500);  
                    float r = rand() / ((float) RAND_MAX);
                    if(r < 0.5){
                        motor_turn(108,0,1000);
                    } else if (r >= 0.5){
                        motor_turn(0,108,1000);
                    }
                    motor_forward(0,0);
                }
            }      
        }
    } 
#endif
/* [] END OF FILE */
