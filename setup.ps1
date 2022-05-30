# Run me to (hopefully) set up a nice build + dev environment.
# Should cmake all the things, as well as spin up a flutter app and copy in the main.dart
# from the top level of this repo.

cmake --build ./build --config Debug --target ALL_BUILD -j 10 --

# Create the flutter app
flutter create app
Remove-Item app/lib/main.dart
Copy-Item main.dart app/lib/main.dart
Set-Location app
flutter build bundle
Set-Location ..

# Run the thing
.\build\Debug\flutterkit_embedder.exe .\app .\build\Debug\icudtl.dat
