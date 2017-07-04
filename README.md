# :red_circle: OpenVR Input Recorder
Tired of pulling your headset on everytime you change code? 

* Record VR actions once and replay each time you change code
* Test multiplayer VR content without needing multiple headsets
* Replay at 2x to speed up test cycles
* Records motion data and button/trigger presses for all connected devices
* Since tracking data is recorded/injected at the OpenVR driver level, it works with any game engine

## Usage
To record:
```
 ./openvr-input-recorder.exe record my_recording
```

To replay:
```
 ./openvr-input-recorder.exe replay my_recording
```

### For testing/debugging
* I recommend making recordings that leave your app in the same state at the start and end. Otherwise you'll have to put on the headset to reset the app.
* Input recording/playback doesn't work very well for testing non-deterministic stuff, for obvious reasons

### For testing multiplayer
* You'll need 2 or more machines, but only one headset. Record to a file accessible by all machines (I use Google Drive), and playback on each machine as you see fit.
