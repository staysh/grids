
## grids

an idiomatic grid ui library for embedded systems


grids is a c++ library geared towards idiomatic programming, rapid UI prototyping, and simple integer physics. It is designed for use in small, embedded systems like the teensy hardware platform. I began this project to have something I could use to prototype monome grid based  interfaces for one-off hardware experiments as well as to possibly make it easier for non-programmers to experiment as well. Some heuristics/intentions I had in development were:
- no dynamic memory allocation
  - this is cautioned against in embedded design but perhaps that is a dated concern
  - pointers and references can be frustrating to new programmers
- be convenient without closing
  - have repetitive tasks and logic be abstracted without accidentally presuposing restrictions
- semantically parallel the serialosc spec
  - learning either will reinforce framiliarity
- minimize the need for memorization
  - very few functions
  - consistent and well-mapping language

Some future goals for the project are:
- decouple from the teensy platform to include others like daisy, ardiuno, even generic c++ or rpi
- adapt with users to build out helpful types and flows and streamline syntactical oddities
- make a sibling library for the arc

Would love some feedback if anyone happens to use this.
### Requirements

Currently...
- monome grid
- teensy 3.6 (uses USBHost_t36)


### Documentation

The smallest objects in grids are a Point and a Vector
```
Point p{0,0};       //0 to 16
Vector v{-2,5};    //-128 to 127
```
A point must be somewhere on a monome 256 grid or smaller. A point is a bitfield...
```
typedef struct Point
{
  uint8_t x:4;
  uint8_t y:4;
}
```
What this means is that the values automatically wrap around 16.
```
Point p{32, 32}; //becomes {0,0}
Point q{20, 21}; //becomes {4, 5}
``` 
A Vector is clamped to [-128, 127]
```
Vector a{100, -100};
Vector b{100, -100};
Vector c = a + b;       //c is {127, -128}
```
These two types can interact as you might expect:
```
Vector distance = p - q;   //point - point = vector
Point displaced = p + distance //point + vector = point
p += q //undefined, cant add two points
```
These elements are the start of a small set of of integer physics tools I imagine might be useful with the notion of a grid. No modulo or value checking is necessay so code can be more succinct.
## grid and grids
There are two more main types that make up the library:
- grid: continuous region of keys and leds
- grids: a function lookup table and serial connection

The syntax for these is a little clunky at the moment, but there is not a lot to know...
```
Grid<64> rightHalf = Grid<64>({0,0}, {8,8});
```
rightHalf describes a grid with an offset of x=0, y=0 and a size of width=8, height=8  

The offset is a *Point*, and the size is a *Vector*.  

```
Grids<128, 1> app = Grids<128, 1>();
```
app describes a 128 x 1 function lookup table that watches the serial bus for messages from a monome grid and sends out messages to a monome grid.  

The second argument in the <> is a "depth" of sorts. Having multiple layers provides support for "popup" windows or "sub-menus" depending on how you conceive of it.

Notice that <64, 5> would create an array of [64 * 5], so every layer, whether used or not, is created during compilation of your code. The memory footprint of this is rather insignificant. A grids instance for a 256 grid with 100 layers is something like 100 kB...

### Connecting Grid and Grids:
Continuing from the above code...
```
app.addGridCallback(&rightHalf, 0, someFunc);
```
...again, the syntax hopefully can get cleaned up a bit to ditch the ampersand somehow, but hopefully the mnemonic of "add" and "and" can get you through. 

This function takes a "reference" to a Grid object, a layer number, and the name of a function that will run when a key press maps to a coordinate within the virtual grid. 

The function must already be prototyped, but you can implement what happens later, as is common practice in .ino files.

### Callback Functions
A function that is used as a callback should always take 3 arguments...
```
void someFunc(bool v, int x, int y);
```
Here v is the state of the key (true=pressed), x and y are the *global coordinates of the key, not the coordinates local to some grid.*

Luckily our grid object has data that can help...
```
void someFunc(bool v, int x, int y)
{
  int localX = x - rightHalf.offset.x;
  int localY = y - rightHalf.offset.y;
}
```
## Sending messages to a monome grid
There is only one function used to send messages to a monome grid
```
app.write({0x18, 0, 0, 15});
app.write({0x12});
app.write({SET_COL, 0, 0, 0b10101010});
app.write({SET_MAP_LEVEL, 8, 0}, arrayOfLevels);
```
One function, many ways to call. The user is expected to implement the serialosc protocol on their own, with a few conveniences built in like macros for the hexcodes and a few extra data types with helper functions...

```
d_8 d8Buffer = { 0 };   
d_32 d32Buffer = { 0 };

int mapLevels[64] = { 15 };

mapToD_32(mapLevels, d32Buffer); returns a d_32

app.write({SET_MAP_LEVEL, 0, 0}, mapToD_32(rightHalf.levels, d32Buffer);
```
One thing to remember is that the brace enclosed list of bytes goes first, and can go on forever, with one of either two array types optionally at the end depending on which type of message you are sending.

Only two helper functions exist right now to pack values according to spec
```
mapToD_8(bool arr[64], d_8 buff); //generates a bitwise on/off map
mapTOD_32(int arr[64], d_32 buff); //generates a 4 bit packed level array of 32 bytes
```
## Grid type data members
You can access the stored keys or levels of a grid object like so:
```
rightHalf.levels //returns a reference to an integer array as big as the grid off all the saved led level data
rightHalf.keys //reuturns a reference to a bool array as big as the grid of all the saved key data
rightHalf.key[x + y * 16] = true; //sets the state of key (x,y) on a 128 grid
rightHalf.level[0] = 15; //stores some level in the virtual grid for point (0,0)
```
Where rightHalf is the name of some Grid variable, like we used above.

## A complete example
So, wrapping up, here is a complete program that creates momentary switches over an entire monome 128 grid
```
#include "grids.h"

Grids<128, 1> momentaryGrid = Grids<128, 1>();
Grid<128> myButtons = Grid<128>({0,0}, {16,8});

void buttonCallback(bool v, int x, int y);

void setup() {
  momentaryGrid.init_();  //starts serial comms
  momentaryGrid.addGridCallback(&myButtons, 0, buttonCallback);
}
void loop() {
  momentaryGrid.run();
}
void buttonCallback(bool v, int x, int y)
{
  momentaryGrid.write(v ? SET_ONE_ON : SET_ONE_OFF, x, y});
}
```
