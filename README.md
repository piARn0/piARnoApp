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

TODO: Add release .apk

### Building from sources

TODO: list all Android studio dependencies and explain the process. not done yet

1. Clone this repository  
2. Install Android Studio  
3. If you are a Mac user, install XCode.  
4. Within the Android Studio SDK Manager, install:  
  4.1 Android SDK Platform, API level 26  
  4.2 Android SDK Build Tools, v 28.0.3 or later  
  4.3 Android NDK  
5. Within Android Studio, ensure the following veriables are set:   
  5.1 Set the environment variable JAVA_HOME to the JDK location C:\Program Files\Android\Android Studio\jre.  
  5.2 Set the environment variable ANDROID_HOME to the Android SDK location C:\Users\[username]\AppData\Local\Android\Sdk.  
  5.3 Set the environment variable ANDROID_NDK_HOME to the Android NDK location C:\Users\[username]\AppData\Local\Android\Sdk\ndk\[versionnumber].  
  5.4 Add the JDK tools directory to your PATH C:\Program Files\Android\Android Studio\jre\bin.  
  5.5 Add the Android SDK platform-tools directory to your PATH C:\Users\[username]\AppData\Local\Android\Sdk\platform-tools.  
  5.6 Add the Android SDK tools directory to your PATH C:\Users\<username>\AppData\Local\Android\Sdk\tools.  
6. If you are a Windows user, set up your system to detect your Android device over USB. You may need to install a USB driver for ADB.  

## Used Frameworks

This project is based on the XRPassthrough sample code from the [Oculus OpenXR Mobile SDK](https://developer.oculus.com/downloads/package/oculus-openxr-mobile-sdk).

### Low-Level

- [Oculus Mobile SDK](https://developer.oculus.com/downloads/package/oculus-mobile-sdk/) using C++ for Android [Native Development Kit](https://developer.android.com/ndk).

- [Passthrough API](https://developer.oculus.com/documentation/unity/unity-passthrough/): Real-time 3D visualization of the physical world in the Oculus headsets for AR development.

- [OpenXR](https://www.khronos.org/openxr/): OpenXR is a royalty-free, open standard that provides high-performance access to Augmented Reality (AR) and Virtual Reality (VR)—collectively known as XR—platforms and devices.
  
- [OpenGL](https://www.opengl.org): 3D rendering.

### Song and File Parsing

- [Midifile](https://github.com/craigsapp/midifile): C++ MIDIfile parsing library developed by Craig Stuart Sapp ([@craigsapp](https://github.com/craigsapp/midifile)).

- [obj2c](https://github.com/glampert/obj2c): Simple command line tool to convert Wavefront OBJ models to C/C++ data arrays developed by Guilherme R. Lampert ([glampert](https://github.com/glampert)).

- [Minecraft Ten Font](https://www.fontspace.com/minecraft-ten-font-f40317): Font used for rendering in-app text.
