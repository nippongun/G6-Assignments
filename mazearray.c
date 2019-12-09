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
#define xOffset 4
#define speed 100
    
 typedef struct Coordinates{
        int x;
        int y;
    }coordinates;
typedef enum Direction{WEST=0,NORTH=1,EAST=2, SOUTH=3}direction;

void tank_turn( int dir, int delay);
void r90(struct sensors_ *dig, coordinates *cor,enum Direction *dir,int *check);
void l90 (struct sensors_ *dig, coordinates *cor,enum Direction *dir,int *check);
void checkObject(int ultra_array[], int *object);
void turn(enum Direction *dir, coordinates *cor, int (* maze)[9], struct sensors_ *dig,int *check);
void add_object(int *object,direction *dir,coordinates *cor, int (* maze)[9]);

void zmain(){
    int maze[16][9]={
        {1,1,1,1,0,1,1,1,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1},
        {1,1,0,0,0,0,0,1,1},
        {1,1,1,0,0,0,1,1,1},
        {1,1,1,1,0,1,1,1,1},
    };



    int check = 0;
    int object = 0;
    //uint8_t speed = 100;

    TickType_t start = 0, end = 0, begin = 0, final = 0;


    struct sensors_ dig;
    int ultra_array[5];

    
    direction dir = NORTH;
   
    
    coordinates cor;
    cor.x = 0;
    cor.y = -2;
    
    IR_Start();
    motor_start();
    motor_forward(0,0);
    reflectance_start();
    reflectance_set_threshold(20000, 20000, 20000, 20000, 20000, 20000);
    Ultra_Start();
    while(SW1_Read() != 0){
        vTaskDelay(1);
    }
    while(dig.r3 != 1 && dig.l3 !=1){
        motor_forward(speed,0);
        reflectance_digital(&dig);
    }
    motor_forward(0,0);
    IR_wait();
    begin = xTaskGetTickCount();
    print_mqtt("Zumo061/time","%d",begin);
    while(1){
                
        checkObject(ultra_array,&object);
        reflectance_digital(&dig);
        // Line Following
        // This makes sure the robot will follow the line among the intersections
        if(check == 0){
           
            while(dig.l1 != 1){
                motor_turn(speed+8,0,1);
                reflectance_digital(&dig); 
            }
            while(dig.r1 != 1){
                motor_turn(0,speed,1);
                reflectance_digital(&dig); 
            }
        } 
        // Main Part
        // THis checks for the intersections
        if(((cor.x == 3) && (dig.l3 == 1)  && (dig.r3 == 0) && (dir == NORTH) && (check == 0)) || ((cor.x == -3) && (dig.r3 == 1) &&(dig.l3 == 0) && (dir == NORTH) && (check == 0)) || ((dig.l3 == 1) && (dig.r3 == 1)&& (check == 0)) ){
            check = 1;
            start = xTaskGetTickCount();   
            if(dir == NORTH){
                cor.y++;
            } else if (dir == EAST){
                cor.x++;
            } else if(dir == WEST){
                cor.x--;
            } else if(dir == SOUTH){
                cor.y--;
            }
            print_mqtt("Zumo061/","X:%d  Y:%d  DIR:%d",cor.x,cor.y,cor); 
        // THis part makes sure the bot is on a line in between intersections    
        } else if((dig.l3 == 0) && (dig.r3 == 0) && (check != 0)){
            end = xTaskGetTickCount();
            motor_forward(speed,end - start);
            
            add_object(&object,&dir,&cor,maze);
            turn(&dir,&cor,maze,&dig,&check);
            checkObject(ultra_array,&object);
            add_object(&object,&dir,&cor,maze);
            turn(&dir,&cor,maze,&dig,&check);
            turn(&dir,&cor,maze,&dig,&check);
                     
            
            if(cor.y>=11){
                if(dir == WEST && cor.x == 0){
                    r90(&dig, &cor, &dir, &check);
                }else if(dir == EAST && cor.x == 0){
                    l90(&dig, &cor, &dir, &check);
                } else if( dir == NORTH){
                    if(cor.x > 0){
                        l90(&dig, &cor, &dir, &check);
                    } 
                    if( cor.x < 0){
                        r90(&dig, &cor, &dir, &check);
                    }
                }  
                if(cor.x == 0 && cor.y == 13){
                    final = xTaskGetTickCount();
                    motor_stop();
                    print_mqtt("Zumo061/time","%d",final-begin);
                }
            }
            
            
            
            check = 0;
        }
        motor_forward(speed,0);
    }
}


// r90 and l90 are behaving the same.
// First, the bot moves until the specific inner sensors read white. From there on, the bot continues until it reads black again
void r90( struct sensors_ *dig, coordinates *cor,enum Direction *dir, int *check){
    while(dig->r1 != 0 ||dig->l1 != 0 ){
        tank_turn(RIGHT,0);
        reflectance_digital(dig);
    }
                
    motor_forward(0,500);
    
    while(dig->r1 != 1){
        tank_turn(RIGHT,0);
        reflectance_digital(dig);
    }
    motor_forward(0,500);
    
    if(cor->x > -3 && cor->x <= 0){
        while(dig->r1 != 0){
            tank_turn(RIGHT,0);
            reflectance_digital(dig);
        }
    }
    motor_forward(0,500);
    *dir = *dir+1;
    print_mqtt("Zumo061/","%d",*dir);
    *check = 0;
    //checkObject();
}

void l90( struct sensors_ *dig, coordinates *cor,enum Direction *dir, int *check){
    
    motor_forward(0,500);
    while(dig->r1 != 0 ||dig->l1 != 0 ){
        tank_turn(LEFT,0);
        reflectance_digital(dig);
    }
                
    motor_forward(0,500);
    
    while(dig->l1 != 1){
        tank_turn(LEFT,0);
        reflectance_digital(dig);
    }
    if(cor->x < 3 && cor->x >= 0){
        while(dig->l1 != 0){
            tank_turn(LEFT,0);
            reflectance_digital(dig);
        }
    }
    motor_forward(0,500);
    *dir = *dir-1;
    print_mqtt("Zumo061/","%d",*dir);
    *check = 0;
    //checkObject(int ultra_array[], int );
}

// By setting the different motor in opposite direction, this allows a cleaner turn in a grid system.
// While 0 represents forward movement, 1 represents backward movement
void tank_turn(int dir, int delay){
    if(dir == 0 ){
        //tank turn right
        MotorDirLeft_Write(0);
        MotorDirRight_Write(1);
    } else if (dir == 1){
        //tank turn left
        MotorDirLeft_Write(1);
        MotorDirRight_Write(0);
    }
    //Since "motor_turn" does not set the any motor in any direction, this function can be utilized.
    motor_turn(speed,speed,delay);
}
//To avoid false negatives, i.e the sonic sensors don't read objects properly, the reading must be done redundantly.
//
void checkObject(int ultra_array[], int *object){
    
    for(int i = 0; i < 5; i++){
            ultra_array[i]=Ultra_GetDistance();
        }
        
    if(ultra_array[3]<15){
        *object = 1;  
    }     
}


void turn(enum Direction *dir, coordinates *cor, int (* maze)[9], struct sensors_ *dig,int *check){
    
     if(*dir == WEST) {
      
        // if object above and no object left
        if(maze[cor->y+1][cor->x+xOffset]==1 && maze[cor->y][cor->x-1+xOffset] == 0){
            // go forward  
            //forward();
        } else if(maze[cor->y+1][cor->x+xOffset]== 0 || maze[cor->y][cor->x-1+xOffset] == 1){
            r90(dig, cor, dir, check);
        } else if(maze[cor->y][cor->x+xOffset-1]== 1 && maze[cor->y+1][cor->x+xOffset] == 1){
            r90(dig, cor, dir, check);
            r90(dig, cor, dir, check);
        }
       
    }
    if(*dir == EAST) {
      
        // if object above and no object right
        
        if(maze[cor->y+1][cor->x+xOffset]==1 && maze[cor->y][cor->x+1+xOffset] == 0){
        
        //
        } else if(maze[cor->y+1][cor->x+xOffset]== 0 || maze[cor->y][cor->x+1+xOffset] == 1){
            l90(dig, cor, dir, check);
            
        /* ▓
        *  >▓  
        */    
        // if an object above and right of the bot
        } else if(maze[cor->y][cor->x+xOffset+1]== 1 && maze[cor->y+1][cor->x+xOffset] == 1){
            // go left twice
            l90(dig, cor, dir, check);
            l90(dig, cor, dir, check);
        }
       
    }
    
    if(*dir == NORTH){
        // if left and right of the bot
        if(maze[cor->y][cor->x+xOffset+1] && maze[cor->y][cor->x+xOffset-1] && maze[cor->y+1][cor->x+xOffset]== 0){
            //go forward
            //forward();
        // if an object ahead and the block next to it blocking the way, and the intersect on the right upper corner is open, go right
        } else if(maze[cor->y+1][cor->x+xOffset-1]&&maze[cor->y+1][cor->x+xOffset]&& maze[cor->y+1][cor->x+xOffset+1] == 0){
            // ho right
            r90(dig, cor, dir, check);
        // if in front and right 
        } else if (maze[cor->y+1][cor->x+xOffset] &&maze[cor->y][cor->x+1+xOffset] ){
            // go left
            l90(dig, cor, dir, check);
            //motor_stop();
        // if in front and left    
        } else if(maze[cor->y+1][cor->x+xOffset]&&maze[cor->y][cor->x+xOffset-1]){
            // go right
            r90(dig, cor, dir, check);
            //motor_stop();
            // if in front is occupied and left is free, 
        } else if(maze[cor->y+1][cor->x+xOffset]==1 && maze[cor->y][cor->x+xOffset-1] == 0){
            // turn left
            l90(dig, cor, dir, check);
        } else if(maze[cor->y+1][cor->x+xOffset]==0){
            //forward();
        }
    }
}

void add_object(int *object,direction *dir,coordinates *cor, int (* maze)[9]){
    if(*object == 1){
        print_mqtt("Zumo061/","Object");
        if(*dir == NORTH){
            maze[cor->y+1][cor->x+xOffset] = 1;
        } else if (*dir == EAST){
            maze[cor->y][cor->x+1+xOffset] = 1;
        } else if(*dir == WEST){
            maze[cor->y][cor->x-1+xOffset] = 1;
        }
        *object = 0;
    } 
}

#endif
/* [] END OF FILE */
