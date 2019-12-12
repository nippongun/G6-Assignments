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

enum Direction{WEST=0,NORTH=1,EAST=2, SOUTH=3}direction;

struct Coordinates{
    int x;
    int y;
}coordinates;

void tank_turn(uint8_t speed, int dir, int delay);
void r90();
void l90();
void backup();
void checkObject();
void forward();
void turn();
void add_object();
const int xOffset = 4;
const int yOffset = 0;
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

enum Direction direction = NORTH;

int check = 0;
int object = 0;
int counter = 0;
int aux_count = 0;
int object_count = 0;
const int speed = 100;

TickType_t start, end;


struct sensors_ dig;
int ultra_array[5];
void zmain(){
    coordinates.x = 0;
    coordinates.y = -2;
    
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
                
        checkObject();
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
        if(((dig.l3 == 1) && (dig.r3 == 1)&& (check == 0)) || (coordinates.x == 3 && (dig.l3 == 1) && direction == NORTH && (dig.r3 == 0))||(coordinates.x == -3 && (dig.r3 == 1) && direction == NORTH &&(dig.l3 == 0) )){
            check = 1;
            start = xTaskGetTickCount();   
            if(direction == NORTH){
                coordinates.y++;
            } else if (direction == EAST){
                coordinates.x++;
            } else if(direction == WEST){
                coordinates.x--;
            } else if(direction == SOUTH){
                coordinates.y--;
            }
            print_mqtt("Zumo061/","X:%d  Y:%d  DIR:%d",coordinates.x,coordinates.y,direction); 
        // THis part makes sure the bot is on a line in between intersections    
        } else if((dig.l3 == 0) && (dig.r3 == 0) && (check != 0)){
            end = xTaskGetTickCount();
            motor_forward(speed,end - start);
            counter++;
            
            
            // If a
            add_object();
            turn();
            checkObject();
            add_object();
            turn();
            turn();
            
                        
            check = 0;
        }
        motor_forward(speed,0);
    }
}


// r90 and l90 are behaving the same.
// First, the bot moves until the specific inner sensors read white. From there on, the bot continues until it reads black again
void r90(){
    while(dig.r1 != 0 ||dig.l1 != 0 ){
        tank_turn(speed,RIGHT,1);
        reflectance_digital(&dig);
    }
                
    motor_forward(0,500);
    
    while(dig.r1 != 1){
        tank_turn(speed,RIGHT,1);
        reflectance_digital(&dig);
    }
    motor_forward(0,500);
    
    if(coordinates.x > -3 && coordinates.x <= 0){
        while(dig.r1 != 0){
            tank_turn(speed,RIGHT,1);
            reflectance_digital(&dig);
        }
    }
    motor_forward(0,500);
    direction++;
    check = 0;
    checkObject();
}

void l90(){
    
    motor_forward(0,500);
    while(dig.r1 != 0 ||dig.l1 != 0 ){
        tank_turn(speed,LEFT,1);
        reflectance_digital(&dig);
    }
                
    motor_forward(0,500);
    
    while(dig.l1 != 1){
        tank_turn(speed,LEFT,1);
        reflectance_digital(&dig);
    }
    if(coordinates.x < 3 && coordinates.x >= 0){
        while(dig.l1 != 0){
            tank_turn(speed,LEFT,1);
            reflectance_digital(&dig);
        }
    }
    motor_forward(0,500);
    direction--;
    check = 0;
    checkObject();
}

// By setting the different motor in opposite direction, this allows a cleaner turn in a grid system.
// While 0 represents forward movement, 1 represents backward movement
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
    //Since "motor_turn" does not set the any motor in any direction, this function can be utilized.
    motor_turn(speed,speed,delay);
}

void backup(){
    while(dig.r3 != 1 || dig.l3 != 1){
        reflectance_digital(&dig);
        
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
        motor_backward(speed,0);
    }
}

void forward(){
    while((dig.l3 == 0) && (dig.r3 == 0)){
        motor_forward(speed,0);
        reflectance_digital(&dig);
        checkObject();
    }
}
//To avoid false negatives, i.e the sonic sensors don't read objects properly, the reading must be done redundantly.
//
void checkObject(){
    
    for(int i = 0; i < 5; i++){
            ultra_array[i]=Ultra_GetDistance();
        }
        
        if(ultra_array[3]<15){
            object = 1;
            
        }
        
}


void turn(){
    
     if(direction == WEST) {
      
        // if object above and no object left
        if(maze[coordinates.y+1][coordinates.x+xOffset]==1 && maze[coordinates.y][coordinates.x-1+xOffset] == 0){
            // go forward  
            //forward();
        } else if(maze[coordinates.y+1][coordinates.x+xOffset]== 0 || maze[coordinates.y][coordinates.x-1+xOffset] == 1){
            r90();
        } else if(maze[coordinates.y][coordinates.x+xOffset-1]== 1 && maze[coordinates.y+1][coordinates.x+xOffset] == 1){
            r90();
            r90();
            motor_stop();
        }
       
    }
    if(direction == EAST) {
      
        // if object above and no object right
        
        if(maze[coordinates.y+1][coordinates.x+xOffset]==1 && maze[coordinates.y][coordinates.x+1+xOffset] == 0){
        
        //
        } else if(maze[coordinates.y+1][coordinates.x+xOffset]== 0 || maze[coordinates.y][coordinates.x+1+xOffset] == 1){
            l90();
            
        /* ▓
        *  >▓  
        */    
        // if an object above and right of the bot
        } else if(maze[coordinates.y][coordinates.x+xOffset+1]== 1 && maze[coordinates.y+1][coordinates.x+xOffset] == 1){
            // go left twice
            l90();
            l90();
        }
       
    }
    
    if(direction == NORTH){
        // if left and right of the bot
        if(maze[coordinates.y+yOffset][coordinates.x+xOffset+1] && maze[coordinates.y+yOffset][coordinates.x+xOffset-1] && maze[coordinates.y+yOffset+1][coordinates.x+xOffset]== 0  && direction==NORTH){
            //go forward
            //forward();
        // if an object ahead and the block next to it blocking the way, and the intersect on the right upper corner is open, go right
        } else if(maze[coordinates.y+1][coordinates.x+xOffset-1]&&maze[coordinates.y+1][coordinates.x+xOffset]&& maze[coordinates.y+1][coordinates.x+xOffset+1] == 0){
            // ho right
            r90();
        // if in front and right 
        } else if (maze[coordinates.y+yOffset+1][coordinates.x+xOffset] &&maze[coordinates.y+yOffset][coordinates.x+1+xOffset] && direction == NORTH){
            // go left
            l90();
            //motor_stop();
        // if in front and left    
        } else if(maze[coordinates.y+yOffset+1][coordinates.x+xOffset]&&maze[coordinates.y+yOffset][coordinates.x+xOffset-1]){
            // go right
            r90();
            //motor_stop();
            // if in front is occupied and left is free, 
        } else if(maze[coordinates.y+yOffset+1][coordinates.x+xOffset]==1 && maze[coordinates.y+yOffset][coordinates.x+xOffset-1] == 0){
            // turn left
            l90();
        } else if(maze[coordinates.y+1][coordinates.x+xOffset]==0){
            //forward();
        }
    }
    
   

}

void add_object(){
    if(object){
        //Beep(250,1);
        print_mqtt("Zumo061/","Object");
        object_count++;               
        counter = 0;
        aux_count = 1;
        if(direction == NORTH){
            maze[coordinates.y+1+yOffset][coordinates.x+xOffset] = 1;
            //l90();
        } else if (direction == EAST){
            maze[coordinates.y+yOffset][coordinates.x+1+xOffset] = 1;
            //l90();
        } else if(direction == WEST){
            maze[coordinates.y+yOffset][coordinates.x-1+xOffset] = 1;
            //r90();
        }else{
            //r90();
        }
        object = 0;
    } 
}

#endif
/* [] END OF FILE */
