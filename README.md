# Arcade-Learning-Environment-based interface to the Atari Grand Challenge

This is a library that's API-compatible with the ALE. Hence, agents that were
written to target the ALE can be rebuilt to use the Atari Grand Challenge (AGC)
dataset without (many) changes.

Notice that although one can train agents using this API, trajectories in the
Atari Grand Challenge Dataset **are not** compatible, as shown in the YouTube
video below.

[![Differences between ALE and AGC](http://img.youtube.com/vi/64DQi_7tnuA/0.jpg)](https://www.youtube.com/watch?v=64DQi_7tnuA "Different behavior in ALE / AGC")

Notice that if you have an agent that reads frames from the AGC directly, you
will receive images that are *not* in the NTSC format used by the ALE. This
library fixes this.

# Requirements

You need the following libraries to use this:

 * Develpment tools: a compiler, `make`, etc.
 * libpng - to read and save PNGs
 * libSDL - to display the game on screen
 * hdf5 - to convert the AGC data to something fast for computation

# Usage

First of all, you need to convert the AGC data to HDF5. To do so, build the
HDF5 converter in the tools directory:

```bash
cd tools
make
```

To convert the data, just call the newly-built tool as

```bash
./agcd-to-hdf5 /path/to/atari_v2_release /path/to/agcd-v2.h5
```

After conversion, you will have and HDF5 that's **way smaller** than the
original data and that works *way* faster for "sequential" access:

```
$ du -hs atari_v2_release atari_v2.tar.gz atari-grand-challenge-dataset-v2.h5
126G    atari_v2_release
26G     atari_v2.tar.gz
16G     atari-grand-challenge-dataset-v2.h5
```

Once built, the HDF5 will have a tree-like structure. At the top level you will
have all the games and the NTSC palette (`h5ls` output):

```
mspacman                 Group
palette                  Dataset {256, 3}
pinball                  Group
qbert                    Group
revenge                  Group
spaceinvaders            Group
```

And in each game you will have a branch for the screens and a branch for the
trajectories:

```
$ h5ls atari-grand-challenge-dataset-v2.h5/mspacman
screens                  Group
trajectories             Group
```

To actually use the dataset, you have to change the `ale.loadROM` call to point
to the hdf5 file and the game name. For example:

```c
ale.loadROM("atari-grand-challenge-dataset-v2.h5/revenge") // load Montezuma's
                                                           // Revenge
```

To get the current and next action from the trajectory, you can request these
ints from the fake ALE:

```
Action current = (Action) ale.getInt("current_action");
Action next = (Action) ale.getInt("next_action");
```

That's it. All basic ALE functions should be implemented.

# License

This project borrows code from the ALE and Stella (color averaging and the SDL
rendering code). Since both projects are GPL-2, this is GPL-2 as well.
