# Weather Alert App
Have you ever wore those noise-cancelling headphones of yours, listening to deep bass or classical music, having a jolly good time, only to find out that it is raining **after** the fact?

Well, worry no more! _Weather Alert App_ is for you!

> After downloading _Weather Alert App_, rain stopped occuring! Is this the legendary effect of placebo?
>
> Stranger, 2018

Note: The above is satire.
Note: If you are on GitHub, then you are looking at the mirror repository. Check [GitLab](https://gitlab.com/jameshi16/weather-alert/) for the real repository.

---

## What is Weather Alert App?
_Weather Alert App_ is a GUI application written in C++ that alerts the end-user when it is going to rain. _Weather Alert App_ will eventually sport 3 different modes of alerting the user - normal, ducking, and exclusive. These modes are explained in detail in a later section.

_Weather Alert App_ is written as a *personal utility tool*, meaning that the code is not exactly maintanable. Furthermore, due to its dependence on WinAPI and the lack of preprocessor variables to disable/enable certain parts of the program based on the environment, the application is not built with cross-platform compatibility in mind.

Hence, this application only supports the Windows platform, and is only guaranteed to be fully supported on [the author](https://ahorribleprogrammer.wordpress.com)'s machine.

---

## Usage
Using the _Weather Alert App_ is quite simple. Upon launch, _Weather Alert App_ will have 2 fields:

- **API Key**: An API Key generated from [OpenWeatherMap](http://openweathermap.org/). Usually, a free-tier key is more than enough for a single end-user's use.
- **Location**: The location of the end-user. This can be any location that [OpenWeatherMap](http://openweathermap.org/) supports, and the full list can be found [here](http://bulk.openweathermap.org/sample/city.list.json.gz) (an archive file).

_Weather Alert App_ will have 2 buttons:

- **Hide**: Hides the window into the system tray. Summon the window again by double clicking onto the system tray.
- **Set**: Sets the API Key and Location fields and commits it to memory. For the entire session the _Weather Alert App_ is running, the application will use the API Key and Location stated to obtain weather data.

Currently, _Weather Alert App_ is extremely buggy and is therefore in Alpha, hence, if you encounter any bug that does not already have an issue, please do not hesitate to open one.

---

## Modes
There are 3 planned modes, and 1 fully implemented mode.
- Normal (Background) mode
- Ducking mode
- Exclusive mode

These make use of the features in WinAPI to achieve maximum alerting potential.

_Normal (Background) mode_: This is the default mode, and **currently the only one that is implemented**. This will play music through the audio device like a normal application, until completion.

_Ducking mode_: A planned mode. Ducking mode makes use of the ducking feature of Windows. For end-users, this is more commonly known as "communications activity", and it uses the user's communication settings in their Sound settings to better alert the end-user.

_Exclusive mode_: A planned mode. Exclusive mode takes exclusive control of the main audio device, and plays sound. This means that all other audio sources will be 'muted', and the end-user will only here the alert sound playing through their audio device.

---

## Build instructions
### Pre-requisites
- A Windows computer, running Vista or later
- GNU GCC Compiler if you want to use the `makefile` included in the project. Otherwise, the VC++ compiler is fine too
    - If you picked the GNU GCC compiler, please also download the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk). You will need to replace the "mfidl.h" header that came with the GNU GCC Compiler.

### How to build
Due to the nature of this project, building it on a Windows computer is easier than eating pie. If you are using the **GNU GCC Compiler**, simply just execute:

```
make debug32/release32/debug64/release64
```
in a shell. Choose between `debug32` to `release64` based on your requirements. Generally, if you are building for your own use, build for `release32` or `release64`.

If you are using the **VC++ compiler**, and have Visual Studio, just opening the project in Visual Studio and building straight away should do the trick. Otherwise, compile all of the files in the project, linking the libraries: `Winhttp`, `Gdi32`, `Mf`, `Mfplat`, `Mfuuid`, and `Ole32`.

---

## License
The license file can be found in LICENSE.md, in the same folder as this README.md file.

In short, the license we are using is the Apache License 2.0.

---

## Final Notes
Be sure to visit [my blog](https://ahorribleprogrammer.com) to find out more about this project, and other projects that I have worked on/working on! 