[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# piARno App


## Members
- Joong-Won Seo [@JoongWonSeo](https://github.com/JoongWonSeo)
- Jack Sterling Klusmann [@Jack-Klusmann](https://github.com/Jack-Klusmann)
- Mohammad Naanaa [@hamnaanaa](https://github.com/hamnaanaa)


## Project Description
piARno is a piano teaching and practicing application for head-mounted displays (in our case, the Oculus Quest).
Upon launching the application, the user will be prompted to specify the location of the piano as well as select the song the user would like to play. 
Subsequently, the user will be guided through the selected song by 3D augmented tiles moving towards the piano keys.
Once the user should press a key, a tile will touch the respective piano key and light up. The alignment of the tiles tells the users which keys to press next. The length of the tiles tell the user how long the keys need to be pressed.


## Installation

This project supports two installation options - pre-compiled APK and building from sources with Android Studio.


### Pre-Compiled APK

You can use ADB or SideQuest to install the pre-compiled APK onto your Quest 2. Download the latest APK from [Releases](https://github.com/piARn0/piARnoApp/releases).


### Building from sources

1. Clone this repository  
2. Install Android Studio  
3. If you are a Mac user, install XCode.  
4. Within the Android Studio SDK Manager, install:  
  4.1 Android SDK Platform, API level 26  
  4.2 Android SDK Build Tools, v 28.0.3 or later  
  4.3 Android NDK  
5. Within Android Studio, ensure the following veriables are set:   
  5.1 Set the environment variable `JAVA_HOME` to the JDK location `C:\Program Files\Android\Android Studio\jre`.  
  5.2 Set the environment variable `ANDROID_HOME` to the Android SDK location `C:\Users\[username]\AppData\Local\Android\Sdk`.  
  5.3 Set the environment variable `ANDROID_NDK_HOME` to the Android NDK location `C:\Users\[username]\AppData\Local\Android\Sdk\ndk\[versionnumber]`.  
  5.4 Add the JDK tools directory to your PATH `C:\Program Files\Android\Android Studio\jre\bin`.  
  5.5 Add the Android SDK platform-tools directory to your PATH `C:\Users\[username]\AppData\Local\Android\Sdk\platform-tools`.  
  5.6 Add the Android SDK tools directory to your PATH `C:\Users\<username>\AppData\Local\Android\Sdk\tools`.  
6. If you are a Windows user, set up your system to detect your Android device over USB. You may need to install a USB driver for ADB.  
7. Import the project in Android Studio by selecting the `<repo root>/XrSamples/XrPassthrough/Projects/Android` folder.
8. Connect your Oculus Quest 2 headset via USB and press 'Run'.


## Implementation Overview

All of the relevant source files are located in `<repo root>/XrSamples/XrPassthrough/Src`.

`Engine.h/cpp` and `Object.h/cpp` contain the classes that create an abstraction layer on top of the underlying frameworks OpenGL ES and OpenXR, in order to simplify the actual project code and make it more portable.

`Piarno.h/cpp` contains the code of the project relevant logic (piano overlay rendering, falling tiles based on the MIDI file, UI controls, etc.).

The `midi` subfolder contains the `Midifile` library (see below) to parse raw MIDI files into the events containing e.g. note press/release information.

The subfolders `models` and `songs` each contain pre-converted 3D models and MIDI files, respectively (in order to avoid NDK's complicated file loading procedure).



## Used Frameworks & Credits

This project is based on the XRPassthrough sample code from the [Oculus OpenXR Mobile SDK](https://developer.oculus.com/downloads/package/oculus-openxr-mobile-sdk).

### Low-Level

- [Oculus Mobile SDK](https://developer.oculus.com/downloads/package/oculus-mobile-sdk/) using C++ for Android [Native Development Kit](https://developer.android.com/ndk).

- [Passthrough API](https://developer.oculus.com/documentation/unity/unity-passthrough/): Real-time 3D visualization of the physical world in the Oculus headsets for AR development.

- [OpenXR](https://www.khronos.org/openxr/): OpenXR is a royalty-free, open standard that provides high-performance access to Augmented Reality (AR) and Virtual Reality (VR)—collectively known as XR—platforms and devices.
  
- [OpenGL](https://www.opengl.org): 3D rendering.

### Song and File Parsing

- [Midifile](https://github.com/craigsapp/midifile): C++ MIDIfile parsing library developed by Craig Stuart Sapp ([@craigsapp](https://github.com/craigsapp/midifile)).

- [obj2c](https://github.com/glampert/obj2c): Simple command line tool to convert Wavefront OBJ models to C/C++ data arrays developed by Guilherme R. Lampert ([@glampert](https://github.com/glampert)).

- [Minecraft Ten Font](https://www.fontspace.com/minecraft-ten-font-f40317): Font used for rendering in-app text.

- [Free Piano Tutorials](https://www.freepianotutorials.net/): Most of included sample MIDI songs from [freepianotutorials.net](https://www.freepianotutorials.net/).