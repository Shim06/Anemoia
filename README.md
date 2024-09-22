<h1 align="center">
  <br>
  <img src="https://raw.githubusercontent.com/Shim06/Anemoia/main/assets/Anemoia.png" alt="Anemoia" width="150">
  <br>
  <b>Anemoia</b>
  <br>
</h1>

<p align="center">
  Anemoia is a modern Nintendo Entertainment System (NES) emulator, written in C++.
  It aims to provide a performant and modern-looking emulator while ensuring a user-friendly experience for users.  
  This emulator is inspired by the open-source Nintendo Switch emulator, <a href="">Ryujinx</a>.
  Anemoia is available on Github under the <a href="https://github.com/Ryujinx/Ryujinx/blob/master/LICENSE.txt" target="_blank">MIT license</a>.
  <br/>
</p>

<p align="center">
    <br>
    <img src="https://raw.githubusercontent.com/Shim06/Anemoia/main/assets/preview.png">
</p>

## Compatibility
As of now, Anemoia has implemented nine major memory mappers:
totaling around 86% of games, or around 2080 games.

Anyone is free to open an issue if a bug or error is found within a game.

## Getting Started
1. Download the latest zip file from the releases section.
2. Extract the contents.
3. Run `Anemoia.exe` to start the program.

## How to build

If you wish to build the emulator yourself, follow these steps:

### Step 1

Either use `git clone https://github.com/Shim06/Anemoia.git` on the command line to clone the repository or use Code --> Download zip button to get the files.

### Step 2
Open `Raycasting Engine.sln` with Visual Studio and configure the project. Under Configuration Properties, ensure the correct Configuration (Debug or Release) and Platform (x64 or Win32) are selected.

### Step 3
To Build the Project in Visual Studio, select Build from the menu. Click Build Solution or press `Ctrl+Shift+B` to start the build process.

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for more details.
