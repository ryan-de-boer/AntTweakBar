# Direct3D 8.1 and 12 port of AntTweakBar

# Direct3D 12
Needs Visual studio 2015 and the Direct3D sdk that comes with it.

There are currently no binaries, you need to compile it yourself.

Init like this.
```C
// 'pDevice' should be a 'ID3D12Device *'
TwInit( TW_DIRECT3D12, pDevice );
````

Draw like this.
```C
// 'pGraphicsCommandList' should be a open 'ID3D12GraphicsCommandList *', close it and execute after the call.
TwDrawContext( pGraphicsCommandList );
```

There is also a couple of minor changes not related to D3D12. 
```C
// Set the height of the mesh drawn for a Quaternion / Direction / Axis.
// Its is expressed in rows and is multiplied by the height of a character tow / height of the font.
// Max rows is 16
TwAddVarRW(twBar, "Camera", TW_TYPE_QUAT4F, &twRotation, "opened=true rows=8");

// You can define sub groups with the '/' divider.
// This will create 3 groups and place "Clear color" in the last group, "Framebuffer", all groups are created open.
TwAddVarRW(twBar, "Clear color", TW_TYPE_COLOR3F, &clearColor, "opened=false group='Misc/Colors/Framebuffer'");

// When defining groups you can set them as open/closed using '+'/'-', if they exist their opend state is changed.
// This will create or modify 2 groups, "Meshes" will be open, "Modifier" will be closed.
TwAddVarRW(twBar, "Scale", TW_TYPE_FLOAT, &scale, "group='-Meshes/+Modifiers'");
```

If you find bugs open issues, or better yet, send pull requests with fixes.

# Direct3D 8.1 port of AntTweakBar

Needs VS2022 and DX8.1 SDK

Store DX8.1 SDK include and lib folders here: Dependencies\dx81sdk

https://archive.org/details/dx81sdk_full

Compile with 32bit configuration.

1. Include
First, include the AntTweakBar header with your other includes.
```C
#include <AntTweakBar.h>
```

2. Initialize AntTweakBar
```C
TwInit(TW_DIRECT3D8, pD3DDevice); // for Direct3D 8
TwWindowSize(myWindowWidth, myWindowHeight);
TwDefine("SHOW_CURSOR"); // If AntTweakBar cursors are needed.
TwDefine("HIDE_CURSOR"); // If game has cursors and want to use them instead (otherwise they may flash).
```

3. Create a tweak bar and add variables
Declare one or more pointers of type TwBar*, and create one or more tweak bars by calling TwNewBar

```C
TwBar *myBar;
myBar = TwNewBar("NameOfMyTweakBar");
Then, you can add variables to your tweak bar(s) by calling TwAddVarRW, TwAddVarRO, TwAddVarCB, or TwAddButton

TwAddVarRW(myBar, "NameOfMyVariable", TW_TYPE_INT32, &myVar, "");
TwAddVarRW(myBar, "NameOfMyBoolVariable", TW_TYPE_BOOLCPP, &myBoolVar, "");
```

4. Draw your tweak bar
Call TwDraw at the end of your main loop to draw your tweak bar(s). It must be called just before the frame buffer is presented (swapped).
```C
// main loop
while( ... )
{
    // clear the frame buffer
    // update view and camera
    // update your scene
    // draw your scene
 
    TwDraw();  // draw the tweak bar(s)
 
    // present/swap the frame buffer
 
} // end of main loop
```
5. Handle mouse and keyboard events, and window size changes
With Windows (and DirectX)
Use TwEventWin when processing incoming events
```C
// In the Windows MessageProc callback
LRESULT CALLBACK MessageProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if( TwEventWin(wnd, msg, wParam, lParam) ) // send event message to AntTweakBar
        return 0; // event has been handled by AntTweakBar
    // else process the event message
    // ...
}
```
6. Terminate AntTweakBar
Just before closing your graphic window and uninitializing the graphic API, call TwTerminate to uninitialize AntTweakBar
```C
// at the end of your program
TwTerminate();
// close your graphic window
// exit your application
```
See this for more information:
https://anttweakbar.sourceforge.io/doc/tools_anttweakbar_howto.html


### Original Readme.txt from AntTweakBar development library follows

AntTweakBar is a small and easy-to-use C/C++ library that allows programmers
to quickly add a light and intuitive GUI into OpenGL and DirectX based 
graphic programs to interactively tweak parameters.

This package includes the development version of the AntTweakBar library 
for Windows, GNU/Linux and OSX, and some program examples (sources + binaries).

For installation and documentation please refer to:
http://anttweakbar.sourceforge.net/doc
