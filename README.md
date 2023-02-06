Pay no attention to the man behind the curtain...

![image](https://user-images.githubusercontent.com/5544935/170974749-7bf55bcd-4df7-48f7-a09f-f5853ed91e2b.png)


## Reqs

- Flutter must be installed on your system in order to build the client app.
- Build scripts are hardcoded to `windows-x64-embedder`, will be changed asap

## Setup

Run `setup.ps1`. It should cmake the things, and create a new flutter app boilerplate with the `main.dart` in this repo. It'll launch the SK host which should spin up the Dart VM and launch the flutter bundle.

## Status

Flutter runs, Dart code executes, etc, etc. However, no pixels are flowing... I have it set up with a software renderer config to at least get the VM to shut up because SK isn't respecting the `-DSKG_FORCE_OPENGL` switch and keeps going back to DirectX. It isn't actually pinging the frame submit callbacks which is weird...
