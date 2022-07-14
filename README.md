# piARno App

## Members:
- Joong-Won Seo [@JoongWonSeo](https://github.com/JoongWonSeo)
- Mohammad Naanaa [@hamnaanaa](https://github.com/hamnaanaa)
- Jack Sterling Klusmann [@Jack-Klusmann](https://github.com/Jack-Klusmann)

## Project Description:
piARno is a piano teaching and practicing application for head-mounted displays (in our case, the Oculus Quest).
Upon launching the application, the user will be prompted to specify the location of the piano as well as select the song the user would like to play. 
Subsequently, the user will be guided through the selected song by 3D augmented tiles moving towards the piano keys.
Once the user should press a key, a tile will touch the respective piano key and light up. The alignment of the tiles tells the users which keys to press next. The length of the tiles tell the user how long the keys need to be pressed.

## Used Frameworks:
- [Oculus Mobile SDK](https://developer.oculus.com/downloads/package/oculus-mobile-sdk/) using C++ for Android [Native Development Kit](https://developer.android.com/ndk)
- [Passthrough API](https://developer.oculus.com/documentation/unity/unity-passthrough/): Real-time 3D visualization of the physical world in the Oculus headsets for AR development
- [OpenXR](https://www.khronos.org/openxr/): 
- [OpenGL](https://www.opengl.org): 3D rendering
- [Midifile](https://github.com/craigsapp/midifile): C++ MIDIfile parsing library
