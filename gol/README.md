# gol

the classic Conway's Game of Life

I think I've written versions of this like 5 times, but this time im trying to make it a little more fancy.

1. ~write the naive implementation: fixed grid size, finite world, static viewpoint etc~
1. ~use a big 2d array as the grid~
1. use a different data structure to hold only the alive cells in a more efficient way (hashtable?)
1. figure out camera, zooming, panning etc.
1. think about "infinite world"

rules from [Wikipedia](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life):

1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
1. Any live cell with two or three live neighbours lives on to the next generation.
1. Any live cell with more than three live neighbours dies, as if by overpopulation.
1. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.


