#include "grids.h"

void toggleZone(bool s, int x, int y);
void momentaryZone(bool s, int x, int y);
void incrementZone(bool s, int x, int y);

Grids<128, 1> grids = Grids<128, 1>();

Grid<64> toggleMap = Grid<64>({8,0}, {8, 8}); 
Grid<8> momentaryRow = Grid<8>({0,0}, {8, 1});
Grid<8> incrementRow = Grid<8>({0,1}, {8, 1});

d_8 d8Buffer = { 0 };
d_32 d32Buffer = { 0 };

Point bball{0,2};
Vector spd{1,1};
unsigned long inter = 0;

void setup() {
  
  grids.init_();
  grids.addGridCallback(&toggleMap, 0, toggleZone);
  grids.addGridCallback(&momentaryRow, 0, momentaryZone);
  grids.addGridCallback(&incrementRow, 0, incrementZone);
  //grids.write({ SET_ALL_ON });
  grids.write({ SET_LEVEL_MAX, 0 });
  grids.write({ SET_ALL_ON });
    
  for(int i = 0; i < 16; i++)
  {
    grids.write( {SET_LEVEL_MAX, i} );
    delay(100);
  }
  
  grids.write({ SET_ALL_OFF });
  delay(500);

  for(int i = 0; i < 16; i++)
  {
    grids.write({ SET_COL, i, 0, 0b11111111 });
    delay(100);
  }
  for(int i = 0; i < 16; i++)
  {
    grids.write({ SET_COL, i, 0, 0 });
    delay(100);
  }
  
  for(int i = 0; i < 8; i++)
  {
    grids.write({ SET_ROW, 0, i, 255 });
    grids.write({ SET_ROW, 8, 7 - i, 255});
    delay(100);
  }
  
  for(int i = 0; i < 32; i++)
  {
    d32Buffer[i] = i;
  }
  for(int i = 0; i < 64; i++)
  {
    grids.write({ SET_MAP_LEVEL, 0, 0}, d32Buffer);
    grids.write({ SET_MAP_LEVEL, 8, 0}, d32Buffer);
    for(int i = 0; i < 32; i++)
    {
      d32Buffer[i] += 1;
    }
    delay(100);
  }
  
  grids.write({ SET_ALL_OFF });
  grids.write({SET_ONE_LEVEL, bball.x, bball.y, 15});
               
}


void loop() {
  
  grids.run();

  if(millis() - inter > 100)
  {
    grids.write({SET_ONE_LEVEL, bball.x, bball.y, 0});
    bball = bball + spd;
    if(bball.x == 7 || bball.x == 0)
    {
      spd.i *= -1;
    }
    if(bball.y == 7 || bball.y == 2)
    {
      spd.j *= -1;
    }
    grids.write({SET_ONE_LEVEL, bball.x, bball.y, 15});
    inter = millis(); 
  }

}
void toggleZone(bool s, int x, int y)
{
  uint8_t index = x - 8 + y * 8;  //buffer[64]
  if(s)
  {
    toggleMap.key[index] = toggleMap.key[index] ? false : true;
    bool vOut = toggleMap.key[index];
    grids.write( { (vOut ? 0x11 : 0x10), x, y } );
  }
}

void momentaryZone(bool s, int x, int y)
{
  if(s) 
    grids.write({0x11, x, y});
  else
    grids.write({0x10, x, y});                 
}

void incrementZone(bool s, int x, int y)
{
  if(s)
  {
    incrementRow.level[x] += 1;
    grids.write({SET_ONE_LEVEL, x, y, incrementRow.level[x]});
  }
}
