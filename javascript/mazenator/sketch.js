var COLS = 40;
var ROWS = 20;
var SIZE = 15;

var maze;

function setup() {
  createCanvas(COLS*SIZE+1, ROWS*SIZE+1);
  frameRate(100);
  maze = new Maze(COLS, ROWS, SIZE);
  maze.draw();  // show grid
}

function draw() {
  maze.tick();
}