import 'package:flutter/material.dart';

void main() {
  // This affects things like whether scrollbars appear, etc
  // debugDefaultTargetPlatformOverride = TargetPlatform.fuchsia;
  runApp(const MyApp());
}

// For enabling "mouse" drag scrolling.
// class MyCustomScrollBehavior extends MaterialScrollBehavior {
//   // Override behavior methods and getters like dragDevices
//   @override
//   Set<PointerDeviceKind> get dragDevices => { 
//     PointerDeviceKind.touch,
//     PointerDeviceKind.mouse,
//     // etc.
//   };
// }

class AppTile extends StatelessWidget{
  const AppTile({required this.icon, required this.title, required this.color});

  final IconData icon;
  final String title;
  final Color color;

  @override
  Widget build(BuildContext context) {
    return Container(
      
      padding: const EdgeInsets.all(5),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        // mainAxisAlignment: MainAxisAlignment.center,
        // crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Expanded(
            child: AspectRatio(
              aspectRatio: 1,
              child: FittedBox(
                fit: BoxFit.fill,
                child: Container(
                  width: 64,
                  height: 64,
                  child: ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: color,
                      shape: const RoundedRectangleBorder(borderRadius: BorderRadius.all(Radius.circular(20.0))),
                    ),
                    onPressed: () { },
                    child: Icon(icon, color: Colors.white, )
                  ),
                )
              )
            )
          ),
          const SizedBox(height: 5),
          Text(title, style: const TextStyle(color: Colors.white), textAlign: TextAlign.center)
        ]
      )
    );
  }
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      // scrollBehavior: MyCustomScrollBehavior(), // for "mouse" (i.e., vr) scrolling
      debugShowCheckedModeBanner: false, // Building kernel_blobs and embedding always shows the debug banner
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // Try running your application with "flutter run". You'll see the
        // application has a blue toolbar. Then, without quitting the app, try
        // changing the primarySwatch below to Colors.green and then invoke
        // "hot reload" (press "r" in the console where you ran "flutter run",
        // or simply save your changes to "hot reload" in a Flutter IDE).
        // Notice that the counter didn't reset back to zero; the application
        // is not restarted.
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, required this.title}) : super(key: key);

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  final List<AppTile> appTiles = const [
    AppTile(icon: Icons.settings, title: 'Settings', color: Colors.blueGrey),
    AppTile(icon: Icons.airline_seat_flat, title: 'Flights', color: Colors.cyan),
    AppTile(icon: Icons.account_balance_wallet, title: 'Wallet', color: Colors.green),
    AppTile(icon: Icons.account_balance, title: 'Bank', color: Colors.orange),
    AppTile(icon: Icons.account_circle, title: 'Profile', color: Colors.purple)
  ];

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  List<int> counters = List<int>.filled(100, 0);

  void _incrementCounter(int index) {
    setState(() {
      // This call to setState tells the Flutter framework that something has
      // changed in this State, which causes it to rerun the build method below
      // so that the display can reflect the updated values. If we changed
      // _counter without calling setState(), then the build method would not be
      // called again, and so nothing would appear to happen.
      counters[index] += 1;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black.withAlpha(0),
      body: Container(
      // color: Colors.white,
      child: ListView.builder(
        padding: const EdgeInsets.all(5),
        scrollDirection: Axis.horizontal,
        shrinkWrap: true,
        itemCount: 10,
        physics: const BouncingScrollPhysics(),
        itemBuilder: (BuildContext context, int index) {
          return widget.appTiles[index % widget.appTiles.length];
        }
      )
    )
    );
  }
}


// // A super to-the-metal way of drawing Flutter UI with raw SceneBuilder calls.

// // Copyright 2013 The Flutter Authors. All rights reserved.

// // Redistribution and use in source and binary forms, with or without modification,
// // are permitted provided that the following conditions are met:

// //     * Redistributions of source code must retain the above copyright
// //       notice, this list of conditions and the following disclaimer.
// //     * Redistributions in binary form must reproduce the above
// //       copyright notice, this list of conditions and the following
// //       disclaimer in the documentation and/or other materials provided
// //       with the distribution.
// //     * Neither the name of Google Inc. nor the names of its
// //       contributors may be used to endorse or promote products derived
// //       from this software without specific prior written permission.

// // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// // ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// // WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// // DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// // ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// // (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// // LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// // ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// // (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// // SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// import 'dart:ui' as ui;

// void beginFrame(Duration timeStamp) {
//   final double devicePixelRatio = ui.window.devicePixelRatio;
//   final ui.Size logicalSize = ui.window.physicalSize / devicePixelRatio;

//   final ui.ParagraphBuilder paragraphBuilder = ui.ParagraphBuilder(
//     ui.ParagraphStyle(textDirection: ui.TextDirection.ltr),
//   )
//     ..addText('Hello, world. ' + timeStamp.toString());
//   final ui.Paragraph paragraph = paragraphBuilder.build()
//     ..layout(ui.ParagraphConstraints(width: logicalSize.width));

//   final ui.Rect physicalBounds = ui.Offset.zero & (logicalSize * devicePixelRatio);
//   final ui.PictureRecorder recorder = ui.PictureRecorder();
//   final ui.Canvas canvas = ui.Canvas(recorder, physicalBounds);
//   canvas.scale(devicePixelRatio, devicePixelRatio);
//   canvas.drawParagraph(paragraph, ui.Offset(
//     (logicalSize.width - paragraph.maxIntrinsicWidth) / 2.0,
//     (logicalSize.height - paragraph.height) / 2.0,
//   ));
//   final ui.Picture picture = recorder.endRecording();

//   final ui.SceneBuilder sceneBuilder = ui.SceneBuilder()
//     // TODO(abarth): We should be able to add a picture without pushing a
//     // container layer first.
//     ..pushClipRect(physicalBounds)
//     ..addPicture(ui.Offset.zero, picture)
//     ..pop();

//   ui.window.render(sceneBuilder.build());

//   print("hello from dart! the time is.... " + timeStamp.toString());

//   ui.PlatformDispatcher.instance.scheduleFrame();
// }

// // This function is the primary entry point to your application. The engine
// // calls main() as soon as it has loaded your code.
// void main() {

//   // Added by Finn; have to fool the platform into thinking we're Fuschia (lol)
//   debugDefaultTargetPlatformOverride = TargetPlatform.fuchsia;
  
//   // The engine calls onBeginFrame whenever it wants us to produce a frame.
//   ui.PlatformDispatcher.instance.onBeginFrame = beginFrame;
//   // Here we kick off the whole process by asking the engine to schedule a new
//   // frame. The engine will eventually call onBeginFrame when it is time for us
//   // to actually produce the frame.
//   ui.PlatformDispatcher.instance.scheduleFrame();
// }