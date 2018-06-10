# GaradgetTizen

# Get started:
1) Clone this repo
2) Open Tizen Studio using a workspace unrelated to this repo
3) Go to File -> Import in Tizen Studio
4) Choose Tizen -> Tizen Project and click Next
5) Browse to the directory you cloned the repo into
6) Make sure the project type is set to Wearable v3.0 and click OK.

# Runnig:
1) Expand the GaradgetApp and build the project.
2) Hit Run or Debug to start the emulator.

# Running on a real device
1) Set your watch to connect to wifi. 
2) Under the connections menu, open Wifi Networks and select the network you are connected to.
3) Scroll down to IP address and write this down
4) On your computer, open the directory you installed tizen studio into (c:\tizen-studio is the default).
5) Open the tools folder
6) Run sdb connect <ipaddress> (watch on you phone as you'll need to give permission the first time)
7) This may give you an error but it should connect. Try the command again and you should see an error saying you are already connected.
8) Go back to Tizen Studio. You should be able to choose your device as the target in the dropdown box.

# Bugs
- Only the first garage door is supported in this version.
- Each time you redeploy the app, the credentials will be lost and you will need to configure again.
- UI fit / finish
- Probably many more.

# To do list
1) Improve config screen. I have a lot of code there doing very little... It feels like I'm doing it wrong. At a minimum we should fix the alignment of text fields when entering data.
2) Handle multiple garage doors.
3) Better update the state on the home screen. It can incorrectly show the door state.
4) Implement annimated door.
5) Better crednetial use / caching. Right now we request a token that's valid for 12hrs. Not sure if this is the right thing to do.
6) Create a widget for each door. This may require a service to do the work for us or maybe just inter-app communication.
7) General code cleanup.
8) Better handling of back button.
9) Release to app store.