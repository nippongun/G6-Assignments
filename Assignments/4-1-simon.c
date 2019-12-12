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
void zmain(void)
 {
    struct sensors_ ref;
    struct sensors_ dig;
    int x = 0;
    int check = 0;
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
        motor_forward(100,0);
         
        if((ref.l3 >= 13000) && (ref.r3 >= 13000) && check == 0){
            check = 1;
            x++;
            
            printf("%d",x);
            if(x==1){
                motor_forward(0,500);
            } else if(x==4){
                motor_forward(0,500);
                x = 0;
            }
        } else if((ref.l3 < 13000) && (ref.r3 < 13000)){
            check = 0;
        }
    }
 }
#endif
/* [] END OF FILE */
