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
#if 1
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

#define RIGHT 0
#define LEFT 1

enum Direction{WEST,NORTH,EAST}direction;

struct Coordinates{
    int x;
    int y;
}coordinates;

void tank_turn(uint8_t speed, int dir, int delay);
void r90();
void l90();
const int offset = 3;
const int maze[14][7];

enum Direction direction = NORTH;

int check = 0;
int object = 0;
const int speed = 100;

TickType_t start, end;


struct sensors_ dig;

void zmain(void){
    coordinates.x = 0;
    coordinates.y = 0;
    
    IR_Start();
    motor_start();
    motor_forward(0,0);
    reflectance_start();
    reflectance_set_threshold(20000, 20000, 20000, 20000, 20000, 20000);
    Ultra_Start();
    while(SW1_Read() != 0){
        vTaskDelay(1);
    }
    while(1){
        
        if(Ultra_GetDistance()<15){
            object = 1;
            Beep(10,10);
        }
        reflectance_digital(&dig);
        if(check == 0){
            
            while(dig.l1 != 1){
                //tank_turn(general_speed, RIGHT,1);
                motor_turn(speed+8,0,1);
                reflectance_digital(&dig); 
            }
            while(dig.r1 != 1){
                //tank_turn(general_speed, LEFT,1);
                motor_turn(0,speed,1);
                reflectance_digital(&dig); 
            }
        }
        if((dig.l3 == 1) && (dig.r3 == 1)&& (check == 0)){
            check = 1;
            start = xTaskGetTickCount();    
            
            if(direction == NORTH){
                coordinates.y++;
            } else if (direction == EAST){
                coordinates.x++;
            } else if(direction == WEST){
                coordinates.x--;
            }
                        
        } else if((dig.l3 == 0) && (dig.r3 == 0) && (check != 0)){
            end = xTaskGetTickCount();
            motor_forward(speed,end - start);
            
            if(object){
                //motor_stop();
                if(coordinates.x == 3 && direction == NORTH){
                    l90();
                } else if(coordinates.x == -3 && direction == NORTH){
                    r90();
                } else if (direction == EAST){
                    l90();
                } else if(direction == WEST){
                    r90();
                }else{
                    r90();
                }
                object = 0;
            }
            
            if(coordinates.x == 3 && direction == EAST){
                l90();
            } else if(coordinates.x == -3 && direction == EAST){
                r90();
            } else if(coordinates.x == 3 && direction == WEST){
                r90();
            } else if(coordinates.x == -3 && direction == WEST){
                l90();
            }
            check = 0;
        }     
        motor_forward(speed,1);
    }
   
}

void r90(){
    while(dig.l1 != 0){
        tank_turn(speed,RIGHT,1);
        reflectance_digital(&dig);
    }
                
    motor_forward(0,500);
    
    while(dig.r1 != 1){
        tank_turn(speed,RIGHT,1);
        reflectance_digital(&dig);
    }
    motor_forward(0,500);
    direction++;
    check = 0;
}

void l90(){
    
    motor_forward(0,500);
    while(dig.r1 != 0){
        tank_turn(speed,LEFT,1);
        reflectance_digital(&dig);
    }
                
    motor_forward(0,500);
    
    while(dig.l1 != 1){
        tank_turn(speed,LEFT,1);
        reflectance_digital(&dig);
    }
    motor_forward(0,500);
    direction--;
    check = 0;
}

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
#endif
/* [] END OF FILE */
