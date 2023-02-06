# StereoFlit

Stereoflit is an experimental Flutter embedder for [StereoKit](http://www.stereokit.net), the code-first Mixed Reality engine. StereoFlit allows you to run and embed a pre-bundled Dart+Flutter application as a worldspace UI layer within your StereoKit app. 

<p align="middle">
<img src="https://user-images.githubusercontent.com/5544935/216882882-c8b0b8fb-ad3d-4ed7-9da8-7d5287e78775.png" width=40%/>
<img src="https://user-images.githubusercontent.com/5544935/216883357-4ab88b14-a939-4cc8-a516-57ea4a8962ec.png" width=40%/>
</p>

## Reqs

- Flutter must be installed on your system in order to build Flutter bundles.
- Build scripts are hardcoded to `windows-x64-embedder`. Hypothetically, this could run on any platform that the Flutter embedder can run on.

## Setup

Run `setup.ps1`. It should build the flutter bundle, build the StereoKit app, and launch. If you're unfamiliar with how StereoKit's simulator controls work, [check the docs here.](https://stereokit.net/Pages/Guides/Using-The-Simulator.html)

<img src="https://user-images.githubusercontent.com/5544935/216883027-6c80133c-200d-4132-86a6-0f5f0ebc9a9b.mp4" width="100" height="100">

## Status

There are some big lingering issues. Primarily...

- Only the right hand is supported right now. Nick added some features to a recent preview build that will allow both hands to function correctly, but I haven't had time to add them yet. (Basically, it's tricky to keep track of individual pointers interacting with a single UI behavior, and the presence of the left hand can squash events from the right hand and get Flutter into a broken state!)
- Ideally, the Flutter surface would animate and be pressed in/out by the finger. This requires some vertex shader magic I haven't had time to do yet
- Obviously this workflow is terrible (separate Flutter project for every UI view, every UI view is its own engine, gross!)
- `ui_button_behavior` seems to not like being offset from the UI surface. This makes having the Flutter view float in the air in front of the window break the button behavior
- Ideally we'd have the window body not be there. However, in the current verison of StereoKit, the "empty" window visuals option breaks grabbing/manipulation.
