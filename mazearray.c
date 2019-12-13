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
#if 0
    
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
void handle_turn(enum Direction *dir, coordinates *cor, int (* maze)[9], struct sensors_ *dig,int *check);
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
    reflectance_set_threshold(15000, 15000, 15000, 15000, 15000, 15000);
    Ultra_Start();
    while(SW1_Read() != 0){
        vTaskDelay(1);
    }
    while(dig.r3 != 1 && dig.l3 !=1){
        motor_forward(speed,0);
        reflectance_digital(&dig);
    }
    motor_forward(0,0);
    print_mqtt("Zumo061/start","ready");
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
        // This checks for the intersections
        if(((cor.x == 3) && (dig.l3 == 1)  && (dig.r3 == 0) && (dir == NORTH) && (check == 0)) || ((cor.x == -3) && (dig.r3 == 1) &&(dig.l3 == 0) && (dir == NORTH) && (check == 0)) || ((dig.l3 == 1) && (dig.r3 == 1)&& (check == 0)) ){
            check = 1;
            start = xTaskGetTickCount(); 
            // Controller of the coordiantes
            if(dir == NORTH){
                cor.y++;
            } else if (dir == EAST){
                cor.x++;
            } else if(dir == WEST){
                cor.x--;
            } else if(dir == SOUTH){
                cor.y--;
            }
            print_mqtt("Zumo061/position","X:%d  Y:%d  DIR:%d",cor.x,cor.y,dir); 
        // This part makes sure the bot is on a line in between intersections    
        } else if((dig.l3 == 0) && (dig.r3 == 0) && (check != 0)){     
            end = xTaskGetTickCount();
            motor_forward(speed,end - start);
            
            /*
            * The crucial main part. First it adds and object to the array, which was already scanned earlier 
            * Afterwards, a turn is conducted. If no object is found, the bot just continues and cycles through the next functions
            * If an object was found again, it gets added to the maze array. 
            * To last status updates must be conducted to get bot out of cloaked areas 
            */
            add_object(&object,&dir,&cor,maze);
            handle_turn(&dir,&cor,maze,&dig,&check);
            checkObject(ultra_array,&object);
            add_object(&object,&dir,&cor,maze);
            handle_turn(&dir,&cor,maze,&dig,&check);
            checkObject(ultra_array,&object);
            add_object(&object,&dir,&cor,maze);
            handle_turn(&dir,&cor,maze,&dig,&check);
                     
            // this handles the upper "pyramid" and tries to center the bot in the coordinates of x = 0
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
                // if it reaches the last line, read until both inner sensors read white
                if(cor.y >= 13 ){
                    while(dig.l1 == 1 || dig.r1 == 1){
                        reflectance_digital(&dig);
                    }
                    reflectance_digital(&dig);
                    if(dig.l1 != 1 && dig.r1 != 1)motor_stop();
                    final = xTaskGetTickCount();
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
    
    while(dig->r1 != 1){
        tank_turn(RIGHT,0);
        reflectance_digital(dig);
    }
    if(cor->x > -3 && cor->x <= 0){
        while(dig->r1 != 0){
            tank_turn(RIGHT,0);
            reflectance_digital(dig);
        }
    }
    *dir = *dir+1;
    // print_mqtt("Zumo061/debug","%d",*dir);
    *check = 0;
}

void l90( struct sensors_ *dig, coordinates *cor,enum Direction *dir, int *check){
    
    while(dig->r1 != 0 ||dig->l1 != 0 ){
        tank_turn(LEFT,0);
        reflectance_digital(dig);
    }
    
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
    *dir = *dir-1;
    // print_mqtt("Zumo061/debug","%d",*dir);
    *check = 0;
}

// By setting the different motor in opposite direction, this allows a cleaner turn in a grid system.
// While 0 represents forward movement, 1 represents backward movement as seen in the makros
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
void checkObject(int ultra_array[], int *object){
    
    for(int i = 0; i < 5; i++){
            ultra_array[i]=Ultra_GetDistance();
        }
        
    if(ultra_array[3]<15){
        *object = 1;  
    }     
}

// This function determines all possibilities depending on the object layout relative to the bots coordiantes and direction
void handle_turn(enum Direction *dir, coordinates *cor, int (* maze)[9], struct sensors_ *dig,int *check){
    // All descriptions are realtive to the maze; not the bot
     if(*dir == WEST) {
      
            //if the way up is free and an object is to the left
        if(maze[cor->y+1][cor->x+xOffset]== 0 || maze[cor->y][cor->x-1+xOffset] == 1){
            // turn right
            r90(dig, cor, dir, check);
            // if an object is above and to the left ...
        } else if(maze[cor->y][cor->x+xOffset-1]== 1 && maze[cor->y+1][cor->x+xOffset] == 1){
            // ... turn right twice *rare case*
            r90(dig, cor, dir, check);
            r90(dig, cor, dir, check);
        }
       
    }
    if(*dir == EAST) {
      
            //if the way up is free and an object is to the right
        if(maze[cor->y+1][cor->x+xOffset]== 0 || maze[cor->y][cor->x+1+xOffset] == 1){
            // ... turn left
            l90(dig, cor, dir, check);
            
            /* ▓
            *  >▓  
            */    
            // if an object is above and to the left ...
        } else if(maze[cor->y][cor->x+xOffset+1]== 1 && maze[cor->y+1][cor->x+xOffset] == 1){
            // ... turn left twice *rare case*
            l90(dig, cor, dir, check);
            l90(dig, cor, dir, check);
        }
       
    }
    
    if(*dir == NORTH){
        
        // if an object ahead and the block next to it blocking the way, and the intersect on the right upper corner is open, go right
        if(maze[cor->y+1][cor->x+xOffset-1]&&maze[cor->y+1][cor->x+xOffset]&& maze[cor->y+1][cor->x+xOffset+1] == 0){
            // turn right right
            r90(dig, cor, dir, check);
        // if in front and right 
        } else if (maze[cor->y+1][cor->x+xOffset] &&maze[cor->y][cor->x+1+xOffset] ){
            // go left
            l90(dig, cor, dir, check);  
        // if in front and left    
        } else if(maze[cor->y+1][cor->x+xOffset]&&maze[cor->y][cor->x+xOffset-1]){
            // go right
            r90(dig, cor, dir, check);
            // if in front is occupied and left is free, 
        } else if(maze[cor->y+1][cor->x+xOffset]==1 && maze[cor->y][cor->x+xOffset-1] == 0){
            // turn left
            l90(dig, cor, dir, check);
        }
    }
}

// Adds an object by setting the according variable to 1
void add_object(int *object,direction *dir,coordinates *cor, int (* maze)[9]){
    if(*object == 1){
        //print_mqtt("Zumo061/debug","Object");
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
