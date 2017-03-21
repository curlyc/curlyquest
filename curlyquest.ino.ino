/*
Dumb dungeon rendering code for the arduboy, made by @bakagamedev on twitter. If you find any bugs, you can contact me there.
Has no dependencies beyond the arduboy library itself and a compatible IDE.
Licenced under wtfpl, http://www.wtfpl.net/about/

Enjoy!

Changelog:
1.0 (9th june 2016)
  Initial release
1.1 (11th june 2016)
  Added backstepping using down button
  Changed map view to left side of screen and made the B button swap rendering mode
  Tidied here and there
*/
#include "Arduboy.h"
Arduboy ab;
int mapstate = 0; 
char comp = 'E';
int xnew = 0;
int ynew = 0;
int hp = 1;
int str = 1;
int def = 1;
int luck = 1;
int hpup = (hp + 1);
int strup = (str + 1);
int defup = (def + 1);
int luckup = (luck + 1);
int expe = 0;
int lvl = 10000;
int curseloc = 10;

int viewMap = 1001;   //don't ask, it doesn't compile correctly with a bool and I cannot for the life of me understand why, so this is a stupid stopgap until I figure that out
//list of visible walls
bool wallShow[11] = { };
//this maze should probably be in PROGMEM if you want it to be static, but the intention here is to integrate it into a generator
bool maze[8][8] =
{
  1,1,1,1,1,1,1,1, //top to bottom left to right
  1,0,0,0,0,0,0,1,
  1,1,0,1,0,0,1,1,
  1,0,0,0,0,1,0,1,
  1,1,0,1,1,1,0,1,
  1,0,0,0,0,0,0,1,
  1,0,1,1,1,1,0,1,
  1,0,1,1,1,1,1,1
};
bool maze2[8][8] =
{
  1,1,1,1,1,1,1,1,
  0,0,0,0,0,0,0,1,
  1,1,0,1,0,0,1,1,
  1,0,0,0,0,1,0,1,
  1,1,0,1,1,1,0,1,
  1,0,0,0,0,0,0,1,
  1,0,1,1,1,1,0,1,
  1,1,1,1,1,1,1,1
};

uint8_t cameraX = 1;
uint8_t cameraY = 1;
int8_t cameraDir = 0;

void calculateView()
{
  static const int8_t arrA[12] = { 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0 };
  static const int8_t arrB[12] = { -1, 0, +1, -1, 0, +1, -1, 0, +1, -1, 0, +1 };
 
  int8_t xs, ys;
  for(uint8_t i = 0; i < 12; ++i)
  {
    switch(cameraDir)
    {
      case 0: { xs = arrA[i]; ys = arrB[i]; break; }
      case 1: { xs = -arrB[i]; ys = arrA[i]; break; }
      case 2: { xs = -arrA[i]; ys = -arrB[i]; break; }
      case 3: { xs = arrB[i]; ys = -arrA[i]; break; }
    }
    wallShow[i] = wallCheck(cameraX + xs, cameraY + ys);
  }
 
  wallShow[10] = false; //position player is standing on
 
  if (wallShow[7])  //speed up by disabling hidden walls
  {
    wallShow[4] = false;
    wallShow[0] = false;
    wallShow[2] = false;
  }
  if (wallShow[4])
  {
    wallShow[1] = false;
  }
}

bool wallCheck(int x,int y)
{
  if((x<0) or (y<0) or (x>7) or (y>7))
    return (true);  //Out of bounds is full of walls, means you don't have to waste space by surrounding a map
  else
    return (maze[x][y]);
}

void drawView() //byte x,byte width)
{
  /*
  draws the viewpoint as calculated in calculateView(), drawing every wall in turn, from far->near.
  */
  int wallSize[] = { 6,10,18,32,64};  //size in pixels of each step
  char wall = 0;  //current wall

  int drawSize,halfSize,backSize,halfBackSize,left,leftBack,top,topBack;
  for(char i=0; i<4; i++) //distance
  {
    drawSize = wallSize[i+1]; halfSize = drawSize/2;      //size of walls on screen
    backSize = wallSize[i];   halfBackSize = backSize/2;  //size of the backside of the walls, for depth
    leftBack = 32-(halfBackSize*3);      //x position of the walls on screen
    left     = 32-(halfSize*3);
    topBack  = 32-halfBackSize;         //y position of the walls on screen
    top      = 32-halfSize;

    for(char n=0; n<3; n++) //left->right
    {
      if (wallShow[wall]) //if wall exists, draw it
      {
        if((n==0)&&(!wallShow[wall+1]))  //left wall, only draw if the middle wall is missing
        {
          ab.fillRect(left+drawSize,top,(leftBack+backSize)-(left+drawSize),drawSize,0);  //blank out area behind wall
          ab.drawLine(leftBack+backSize,topBack+backSize,left+drawSize,top+drawSize,1);   //lower line
          ab.drawLine(left+drawSize,top,leftBack+backSize,topBack,1);                     //upper line
          ab.drawLine(leftBack+backSize,topBack,leftBack+backSize,topBack+backSize,1);    //far line
        }
        if((n==2)&&(!wallShow[wall-1])) //right wall, ditto
        {
          ab.fillRect(leftBack,top,left-leftBack,drawSize,0);
          ab.drawLine(leftBack,topBack,left,top,1);     //upper
          ab.drawLine(leftBack,topBack+backSize,left,top+drawSize,1); //lower
          ab.drawLine(leftBack,topBack,leftBack,topBack+backSize,1);  //side
        }
        if((i<3)&&(!wallShow[wall+3]))  //draw flat wall if not immediately next to the camera, and if there is no wall infront
        {
          int wid = drawSize; //width of wall
          if ((n==2) && (left+wid > 64))  //if the wall goes off the render area, chop the width down
          {
            wid = 15; //(64-halfSize)-1;  //magic numbering this because the only time it ever happens is on a close right side wall
          }
          ab.fillRect(left,top,wid,drawSize,0);     //blank out wall area and draw then draw the outline
          ab.drawRect(left,top,wid+1,drawSize+1,1);
        }
      }
      wall++;
      left += drawSize;     //advance left positions
      leftBack += backSize;
    }
  }
  ab.drawRect(0,0,64,64,1);
}

void drawMap()
{
  //draw map grid
  const uint8_t dx = 63;  //x offset, puts on right side of the screen
  for(int iy=0; iy<8; iy++) //loops x&y, draws a rectangle for every wall
  {
    for(int ix=0; ix<8; ix++)
    {
      if (wallCheck(ix,iy))
      {
        ab.drawRect(dx+(ix*8),(iy*8),9,9,1);
      }
    }
  }
  //draws the player as n s e or w indicating direction player is facing
 char cx = cameraX+1;
 char cy = cameraY+1;
  xnew = ((((cx)*8)-7)+64);
  ynew = (((cy)*8)-7); 
  ab.setCursor((xnew), (ynew)); 
      ab.print(comp);
     //testing here

  //outlines the map
  ab.drawLine(dx+64,0,dx+64,63,1);
  ab.drawLine(dx,63,dx+64,63,1);
}

void initView()
{
  viewMap = 1001; 
}

void setup() {
  ab.begin();
  ab.setFrameRate(1); //Kludge around not bothering to implement a keydown function
  calculateView();
}

void loop() {
  if (!(ab.nextFrame()))
    return;

  //change view angle on press
  char cd = cameraDir;
  if (ab.pressed(LEFT_BUTTON) && (mapstate == 0)){
    cameraDir -= 1;
  }
  if (ab.pressed(RIGHT_BUTTON) && (mapstate == 0)){
    cameraDir += 1;
  }

  if (cameraDir < 0){
    cameraDir = 3;
  }
  if (cameraDir > 3){
    cameraDir = 0;
  }
  if (cameraDir != cd){
    calculateView();
  }

  //if the player presses up or down and the space is free, go to it
  if (ab.pressed(UP_BUTTON) && (mapstate == 0) or (ab.pressed(DOWN_BUTTON) && (mapstate == 0)))
  {
    //calculate next space
    int8_t nx=0,ny=0;   //using int8 rather than uint8 so negative numbers are possible
    if (cameraDir==0)
      { nx = 1; comp = 'E'; 
      char cx = cameraX+1;
      char cy = cameraY+1;
      xnew = ((((cx)*8)-7)+64);
      ynew = (((cy)*8)-7); 
      ab.setCursor((xnew), (ynew)); 
      ab.print(comp);}
    if (cameraDir==1)
      { ny = 1; comp = 'S'; 
      char cx = cameraX+1;
      char cy = cameraY+1;
      xnew = ((((cx)*8)-7)+64);
      ynew = (((cy)*8)-7);
      ab.setCursor((xnew), (ynew)); 
      ab.print(comp);}
    if (cameraDir==2)
      { nx = -1; comp = 'W';
      char cx = cameraX+1;
      char cy = cameraY+1;
      xnew = ((((cx)*8)-7)+64);
      ynew = (((cy)*8)-7); 
      ab.setCursor((xnew), (ynew)); 
      ab.print(comp);}
    if (cameraDir==3)
      { ny = -1; comp = 'N';
      char cx = cameraX+1;
      char cy = cameraY+1;
      xnew = ((((cx)*8)-7)+64);
      ynew = (((cy)*8)-7); 
      ab.setCursor((xnew), (ynew)); 
      ab.print(comp);}

    if (ab.pressed(DOWN_BUTTON) && (mapstate == 0))  //if they pressed down, flip the direction
    {
      nx = 0-nx;
      ny = 0-ny;
    }

    nx += cameraX;  //calculate new coordinate
    ny += cameraY;
    if (!wallCheck(nx,ny))  //if space is empty and in bounds, move to it
    {
      cameraX = nx;
      cameraY = ny;
      calculateView();
    }
  }
  if(ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))  //swap screen mode when B button is pressed
  { mapstate = (mapstate + 1);
  }
  if (mapstate == 2) {
    mapstate = 0;
  }
 //lvl up system DONE
  if (ab.pressed(DOWN_BUTTON) && (mapstate == 1) && (curseloc < 40)) {
        curseloc = (curseloc + 10);
  }
  if (ab.pressed(UP_BUTTON) && (mapstate == 1) && (curseloc > 10)) {
        curseloc = (curseloc - 10);
  }
    
  if (ab.pressed(A_BUTTON) && (hpup < lvl) && (mapstate == 1) && (curseloc == 10)) {
        lvl = (lvl - hpup);
        hp = (hp + 1);
        hpup = (hpup +1);
        
  }
   if (ab.pressed(A_BUTTON) && (strup < lvl) && (mapstate == 1) && (curseloc == 20)) {
        lvl = (lvl - strup);
        str = (str + 1);
        strup = (strup +1);
        
  }
   if (ab.pressed(A_BUTTON) && (defup < lvl) && (mapstate == 1) && (curseloc == 30)) {
        lvl = (lvl - defup);
        def = (def + 1);
        defup = (defup +1);
        
  }
   if (ab.pressed(A_BUTTON) && (luckup < lvl) && (mapstate == 1) && (curseloc == 40)) {
        lvl = (lvl - luckup);
        luck = (luck + 1);
        luckup = (luckup +1);
        
  }
switch( mapstate ) { //changes things where the map goes
  case 0:
    //map
  ab.clear(); //clear screen
    drawMap();  //draw map
    drawView(); //draw perspective
    
    //draw camera angle indicator, for debugging.
    //in here somewhere add the n s e w bit
    
    break;
  case 1:
    //stats screen
    ab.clear(); //clear screen 
    ab.setCursor(1, curseloc);
    ab.print("X"); //up to 10 charicters
    ab.setCursor(1, 0);
    ab.print("upgrade"); //up to 10 charicters
    ab.setCursor(11, 10);
    ab.print("HP"); //up to 10 charicters
    ab.setCursor(11, 20);
    ab.print("STR"); //up to 10 charicters
    ab.setCursor(11, 30);
    ab.print("DEF"); //up to 10 charicters
    ab.setCursor(11, 40);
    ab.print("LUCK"); //up to 10 charicters
    ab.setCursor(40, 10);
    ab.print(hpup); //up to 10 charicters
    ab.setCursor(40, 20);
    ab.print(strup); //up to 10 charicters
    ab.setCursor(40, 30);
    ab.print(defup); //up to 10 charicters
    ab.setCursor(40, 40);
    ab.print(luckup); //up to 10 charicters

    ab.setCursor(65, 0);
    ab.print(" stats"); //up to 10 charicters
    ab.setCursor(65, 10);
    ab.print("HP"); //up to 10 charicters
    ab.setCursor(65, 20);
    ab.print("STR"); //up to 10 charicters
    ab.setCursor(65, 30);
    ab.print("DEF"); //up to 10 charicters
    ab.setCursor(65, 40);
    ab.print("LUCK"); //up to 10 charicters
    ab.setCursor(95, 10);
    ab.print(hp); //up to 10 charicters
    ab.setCursor(95, 20);
    ab.print(str); //up to 10 charicters
    ab.setCursor(95, 30);
    ab.print(def); //up to 10 charicters
    ab.setCursor(95, 40);
    ab.print(luck); //up to 10 charicters

    ab.setCursor(11, 50);
    ab.print("lvls"); //up to 10 charicters
    ab.setCursor(50, 50);
    ab.print(lvl); //up to 10 charicters
    break;
  case 2:
    //inventory screen
    ab.setCursor(0, 0);
    ab.print("inventory");
    
    break;
  case 3:
    //npc talk
    ab.setCursor(0, 0);
    ab.print("npc talk ");
    break;
  
  }
 
  ab.display();
}
