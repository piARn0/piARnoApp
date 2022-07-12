//
// Created by JW on 24/06/2022.
//

#include <sstream>

#include "Piarno.h"
#include "Engine.h"
#include "XrPassthroughGl.h"

using namespace global;

void Piarno::init() {
    buildPiano();

    loadMidi(0);
    createTiles();

    /// Build UI
    //alignment guidelines
    pianoOutline.geometry = engine->getGeometry(Mesh::wireframe);
    pianoOutline.pos = vec3{-widthWhite/2, 0, 0};
    pianoOutline.scl = pianoKeys.back().pos - pianoKeys.front().pos + vec3{widthWhite, 0.5, heightWhite};
    pianoOutline.col = color{0, 0, 255, 255};
    pianoScene.attach(pianoOutline);


    //center control panel: play/pause, timeline
    vec3 origin = pianoKeys[2].pos;
    vec3 off{0, 0, 0.2};

    pauseButton.geometry = engine->getGeometry(Mesh::cube);
    pauseButton.pos = origin + off;
    pauseButton.scl = vec3{0.03, 0.02, 0.03};
    pauseButton.pressCol = color{0, 0, 0, 0}; //disable pressColor
    pauseButton.label = "PLAY";
    pianoScene.attach(pauseButton);

    off.x += 0.1;
    timeline.geometry = engine->getGeometry(Mesh::cube);
    timeline.pos = origin + off;
    timeline.scl = vec3{0.03, 0.02, 0.03};
    timeline.col = color{100, 100, 100, 255};
    timeline.label = "TIME"; //TODO: show current time in minutes:seconds instead
    timeline.min = 0.0;
    timeline.max = 0.5;
    timeline.minVal = 0;
    timeline.maxVal = midi.getFileDurationInSeconds();
    timeline.set(currentTime);
    pianoScene.attach(timeline);


    //left control panel: song selector, select button
    origin = pianoKeys[0].pos;
    off = vec3{-0.4, 0, 0.3};

    songListScroll.geometry = engine->getGeometry(Mesh::cube);
    songListScroll.pos = origin + off;
    songListScroll.scl = vec3{0.03, 0.02, 0.03};
    songListScroll.col = color{100, 100, 100, 255};
    songListScroll.label = "SCROLL";
    songListScroll.min = 0.0;
    songListScroll.max = 0.3;
    songListScroll.minVal = 0;
    songListScroll.maxVal = songs.size() - 1;
    songListScroll.set(0);
    pianoScene.attach(songListScroll);

    off.x += 0.4;
    off.z += 0.2;
    selectSong.geometry = engine->getGeometry(Mesh::cube);
    selectSong.pos = origin + off;
    selectSong.scl = vec3{0.03, 0.02, 0.03};
    selectSong.col = color{100, 100, 100, 255};
    selectSong.label = "SELECT";
    pianoScene.attach(selectSong);



    //right control panel: playback speed, scroll speed, toggle alignment guide, piano offset
    origin = pianoKeys.back().pos;
    off = vec3{0.1, 0, 0.3};

    playbackSpeed.geometry = engine->getGeometry(Mesh::cube);
    playbackSpeed.pos = origin + off;
    playbackSpeed.scl = vec3{0.03, 0.02, 0.03};
    playbackSpeed.col = color{100, 100, 100, 255};
    playbackSpeed.label = "SPEED";
    pianoScene.attach(playbackSpeed);

    off.z += 0.1;
    scrollSpeed.geometry = engine->getGeometry(Mesh::cube);
    scrollSpeed.pos = origin + off;
    scrollSpeed.scl = vec3{0.03, 0.02, 0.03};
    scrollSpeed.col = color{100, 100, 100, 255};
    scrollSpeed.label = "SCROLL";
    pianoScene.attach(scrollSpeed);

    off.z += 0.1;
    off.x += 0.015;
    toggleOutline.geometry = engine->getGeometry(Mesh::cube);
    toggleOutline.pos = origin + off;
    toggleOutline.scl = vec3{0.03, 0.02, 0.03};
    toggleOutline.col = color{0, 0, 150, 255};
    toggleOutline.pressCol = color{0, 0, 100, 255};
    toggleOutline.label = "ALIGN";
    pianoScene.attach(toggleOutline);
}


void Piarno::update() {
    //buttons and UI
    const auto &controllers = engine->getControllers();
    pauseButton.update(controllers);
    timeline.update(controllers);
    scrollSpeed.update(controllers);
    playbackSpeed.update(controllers);
    toggleOutline.update(controllers);
    songListScroll.update(controllers);
    selectSong.update(controllers);

    if (pauseButton.isPressed())
        isPaused = !isPaused;

    // make the pauseButton either red or green displaying the current `isPaused` state
    if (isPaused) {
        pauseButton.col = color{255, 0, 0, pauseButton.col.a()};
        pauseButton.label = "PLAY";
    } else {
        pauseButton.col = color{0, 255, 0, pauseButton.col.a()};
        pauseButton.label = "PAUSE";
    }

    if (timeline.isBeingPressed()) {
        isPaused = true;
        currentTime = timeline.get();
    }
    else {
        timeline.set(currentTime);
    }

    if(playbackSpeed.isReleased()) { //round to 0.25, 0.5, ..., 2.0
        playbackSpeed.set(round(playbackSpeed.get() * 4) / 4);
    }

    if(toggleOutline.isPressed())
        pianoOutline.show = !pianoOutline.show;

    if(selectSong.isPressed()) {
        loadMidi((size_t) songListScroll.get());
        createTiles();
        currentTime = 0;
        timeline.set(currentTime);
    }

    if(songListScroll.isReleased())
        songListScroll.set(round(songListScroll.get()));

    //set piano position with controller
    auto ctrlL = controllers[0].pos;
    auto ctrlR = controllers[1].pos;
    if (engine->isButtonPressed(IO::rightTrigger) && engine->isButtonPressed(IO::leftTrigger)) {
        pianoScene.pos = (ctrlL + ctrlR) / 2;
        pianoScene.pos.y -= 0.05;
        pianoScene.rot.y = atan2(ctrlR.x - ctrlL.x, ctrlR.z - ctrlL.z) - M_PI/2;
    }

    //update time and tiles
    if(!isPaused)
        currentTime = std::min(currentTime + 1.0 / 72.0 * playbackSpeed.get(), midi.getFileDurationInSeconds());

    updateTiles();
}


void Piarno::render() {
    engine->renderText("WELCOME TO PIARNO",
                       vec3{0, 1 + sin(engine->getFrame() / 72.0f) * 0.05f, -2},
                       vec3{0.5, 0.5, 0.3},
                       vec3{0, 0, 0},
                       color{255, 255, 255, 255});

    pianoScene.render();

    //render song list
    {
        float height = 0.05;
        vec3 size{0.03, 0.03, 0.05};
        vec3 rot = songListScroll.globalRot(songListScroll.rot + vec3{-M_PI/2, M_PI/2, 0});
        for (size_t i = 0; i < songs.size(); i++) {
            auto &s = songs[i];
            vec3 listPos = songListScroll.globalPos(songListScroll.pos + vec3{songListScroll.max/2 - (songListScroll.get() - i) * height, 0, 0.2});

            color_t a = (1 - std::min(std::abs(songListScroll.get() - i) / (songListScroll.max / height / 1.5f), 1.0f)) * 255;
            if (0 < a) {
                color c = i == round(songListScroll.get()) ?
                        color{0, 126, 252, a} : color{255, 255,255, a};
                engine->renderText(s, listPos, size, rot, c);
            }
        }
    }
}



bool Piarno::isBlack(int index) {
    static const bool blackIndex[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};

    return blackIndex[(index + offset) % 12];
}

void Piarno::buildPiano() {
    pianoKeys.resize(numKeys);

    float x = 0;

    for (int i = 0; i < numKeys; i++) {
        auto &k = pianoKeys[i];
        k.geometry = engine->getGeometry(Mesh::rectGradient);

        k.rot = vec3{M_PI / 2, 0, 0};

        if (!isBlack(i)) //white key
        {
            k.pos = vec3{x, 0, 0};
            k.scl = vec3{widthWhite - gap, heightWhite, 1};
            k.col = color{255, 255, 255, 50, k.geometry};
            k.col.a(0) = k.col.a(1) = 230; //make top parts more solid

            x += widthWhite;

            pianoScene.attach(k); //attach white keys first (for translucent render ordering!)
        } else //black key
        {
            k.pos = vec3{x - widthWhite / 2, blackHover, - heightWhite/2 + heightBlack/2};
            k.scl = vec3{widthBlack - gap, heightBlack, 1};
            k.col = color{0, 0, 0, 50, k.geometry};
            k.col.a(0) = k.col.a(1) = 230; //make top parts more solid
        }
    }

    //attach black keys after white keys
    for (int i = 0; i < numKeys; i++) {
        auto &k = pianoKeys[i];
        if (isBlack(i)) //black key
            pianoScene.attach(k);
    }

    //center
    float width = x;
    for(auto &k : pianoKeys) {
        k.pos.x -= width/2;
    }
}

void Piarno::createTiles() {
    // count the total number of key presses events (=notes being played)
    int keyPressNum = 0;
    for (int i = 0; i < midi.getNumEvents(0); i++) {
        int command = midi[0][i][0];
        if (command == 0x90) {
            keyPressNum++;
        }
    }
    // TODO: cleanup after debugging
    log("[DEBUG/Piarno] Number of notes in this song: " + std::to_string(keyPressNum));
    allTiles.resize(keyPressNum);
    
    std::vector<Tile *> currentTile(numKeys,nullptr); //"currently" (within the below loop) active tile

    int k = 0; //current key press
    for (int i = 0; i < midi.getNumEvents(0); i++) {
        int command = midi[0][i][0];
        int key = midi[0][i][1] - offset;

        // this key is out of range for the active piano
        if (key < 0 || key >= numKeys) {
            continue;
        }

        if (command == 0x90) {
            // key press
            log("[DEBUG/Piarno] Detected keypress for tile k=" + std::to_string(k) +
                " at piano key: " + std::to_string(key));
            allTiles[k].startTime = midi[0][i].seconds;
            allTiles[k].key = key;
            currentTile[key] = &allTiles[k]; //register the currently active tile for this lane
            k++;
        } else if (command == 0x80) {
            // key release
            if (currentTile[key] == nullptr) {
                log("[DEBUG/Piarno] Detected key release for a key that was already released with number: " +
                    std::to_string(key) + " and time " + std::to_string(midi[0][i].seconds));
                continue;
            } else
                log("[DEBUG/Piarno] Detected key release for at piano key: " + std::to_string(key) +
                    " with time " + std::to_string(midi[0][i].seconds));

            currentTile[key]->endTime = midi[0][i].seconds;

            // draw the tile
            auto &tile = currentTile[key]->tile;
            tile.geometry = engine->getGeometry(Mesh::rect);
            tile.pos = pianoKeys[key].pos; //z will be set every frame based on current time
            tile.rot = vec3{M_PI / 2, 0, 0};
            tile.scl = vec3{(isBlack(key) ? widthBlack : widthWhite) - gap, 1, 1}; //height will be updated

            if(isBlack(key)) {
                tile.pos.y = blackHover - keyPressDepth; //float above keys
                tile.col = color{0, 50, 150, 255};
            }
            else {
                tile.pos.y = -keyPressDepth;
                tile.col = color{0, 85, 255, 255};
            }

            pianoScene.attach(tile);

            // release the placeholder indicating that there is no more key played at [key] index
            currentTile[key] = nullptr;

        } else {
            // ignore any other event
            continue;
        }
    }
}

void Piarno::updateTiles() {
    keyHighlight.assign(numKeys, 0.0f);

    for(auto &[tile, key, start, end] : allTiles) {
        float startDist = distFromTime(start - currentTime); //distance in meters to start pos
        float endDist = distFromTime(end - currentTime); //distance in meters to end pos

        tile.scl.y = std::max(0.0f, endDist) - std::max(0.0f, startDist); //tile length
        tile.pos.z = -heightWhite / 2 - (std::max(0.0f, startDist) + tile.scl.y / 2); //center of tile

        tile.show = endDist > 0; //don't render if tile is already in the past

        //highlight keys depending on its key press time
        float highlightStart = distFromTime(1); //start shadow 1 second before press
        if(0 < startDist && startDist < highlightStart) //fade-in highlight before start
            keyHighlight[key] = std::max(keyHighlight[key], 1 - (startDist / highlightStart));
        else if(startDist <= 0 && endDist > 0) //fade-out highlight after press (until key end or max highlightStart)
            keyHighlight[key] = std::max(keyHighlight[key], std::min(1 + (startDist / highlightStart * 2), endDist / (endDist - startDist)));
    }

    //apply highlight (turn red & press down)
    for(int k=0; k<numKeys; k++) {
        float h = keyHighlight[k];
        auto &c = pianoKeys[k].col;

        if(!isBlack(k)) { //white
            c.setAll(G, color_t(255 * (1 - h)));
            c.setAll(B, color_t(255 * (1 - h)));
            pianoKeys[k].pos.y = -h * keyPressDepth;
        }
        else { //black
            c.setAll(R, color_t(255 * h));
            pianoKeys[k].pos.y = blackHover - h * keyPressDepth;
        }
    }
}

float Piarno::distFromTime(double time) {
    return scrollSpeed.get() * time;
}


void Piarno::loadMidi(int i) {
    std::stringstream file;

    switch(i) {
        case 0:
        {
#include "songs/ac_2am.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 1:
        {
#include "songs/canon.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 2:
        {
#include "songs/sweden.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 3:
        {
#include "songs/twinkle.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 4:
        {
#include "songs/supermario.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 5:
        {
#include "songs/hittheroad.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 6:
        {
#include "songs/gymnopedie.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 7:
        {
#include "songs/elise.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 8:
        {
#include "songs/jacque.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 9:
        {
#include "songs/ode.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 10:
        {
#include "songs/heartnsoul.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
        case 11:
        {
#include "songs/dre.h"
            file.str(std::string(bytes, bytes + sizeof(bytes)));
        } break;
    }

    midi = smf::MidiFile(file);
    //midi.absoluteTicks();
    midi.joinTracks();
    midi.doTimeAnalysis();
}
