# cgol

A simple implemention of Conway's Game of Life using SDL2.

## Usage

* Spacebar: Toggle seed mode to change the state of cells in the world.
* Right click:  Toggle seed mode.
* Left click:  Toggle the state of a cell.  Only works in seed mode.
* 'F' key: Speed up the simulation to a min of 1/10th per second.
* 'S' key: Slow down the simulation to a max of once per second.

By default the simulation updates twice per second but can be adjusted
in 0.1 second increments using the 'F' and 'S' keys.  This is capped to
the range of [0.1, 1.0].  The frame rate is capped to 30 FPS.

## Building

### Mac OS

Run `setup.bash` to download the SDL2 dependency to the vendor directory
then run `build_mac.bash`.

### Windows

Run `setup.bat` to download the SDL2 dependency to the vendor directory
then run `build.bat`.

### Linux

I currently don't have a Linux environment to test things but `setup.bash`
should work without changes.  To build you will have to modify `build_mac.bash`
to remove the MacOS specifics.

## License

MIT
