# OpenVR Input Recorder
Tired of pulling your headset on everytime you change code? 

* Record VR actions once and replay each time you change code
* Replay at 2x to shorten test cycles
* Test multiplayer VR on your own (and, in theory, without owning multiple headsets, but this isn't working yet)
* Since tracking data is recorded/injected at the OpenVR driver level, it works with any game engine

Demo: [https://www.youtube.com/watch?v=GaCRjhzMMMg](https://www.youtube.com/watch?v=GaCRjhzMMMg)

## Installation
1. Download the latest (hotfix2) [OpenVR Input Emulator](https://github.com/matzman666/OpenVR-InputEmulator/releases) and run the installer (close SteamVR first)
2. Download the latest [OpenVR Input Recorder](https://github.com/lebek/openvr-input-recorder/releases)

## Usage
To record:
```
$ ./openvr-input-recorder.exe record my_recording
```

To replay:
```
$ ./openvr-input-recorder.exe replay my_recording
```

To replay at 2x speed:
```
$ ./openvr-input-recorder.exe replay my_recording 2
```

To loop at 2x speed:
```
$ ./openvr-input-recorder.exe loop my_recording 2
```

## Tips

### For testing/debugging
* I recommend making recordings that leave your app in the same state at the start and end. Otherwise you'll have to put on the headset to reset the app.
* Input recording/playback doesn't work very well for testing non-deterministic stuff, for obvious reasons

### For testing multiplayer
* You'll need 2 or more machines, each with a connected headset. Record to a file accessible by all machines (I use Google Drive). Launch your app on all machines and playback the recording(s) as you see fit.
