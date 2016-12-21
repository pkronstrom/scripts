function Maze(columns, rows, cellSize) {
  // Initializations
  var columns;
  var rows;
  var cellSize;
  var grid;

  this.initializeGrid = function() {
    grid = [];
    for (var y = 0; y < rows; y++) {
      for (var x = 0; x < columns; x++) {
        var cell = new Cell(x, y);
        grid.push(cell);
      }
    }
  }

  this.initializeGrid();

  var stack = [];         // stack for backtracking
  // var beginAt = floor(random(0,grid.length));
  var beginAt = 0;
  var current = grid[beginAt];  // keep track of current cell
  current.visited = true;
  var previous = new Cell(0,0);

  this.draw = function() {
     for (var i = 0; i < grid.length; i++) {
      grid[i].draw();
    }   
  }

  this.tick = function() {
    /*
      One tick of recursive backtracker algorithm.

      From Wikipedia:
      1. Make the initial cell the current cell and mark it as visited
      2. While there are unvisited grid
        1. If the current cell has any neighbours which have not been visited
          1. Choose randomly one of the unvisited neighbours
          2. Push the current cell to the stack
          3. Remove the wall between the current cell and the chosen cell
          4. Make the chosen cell the current cell and mark it as visited
        2. Else if stack is not empty
          1. Pop a cell from the stack
          2. Make it the current cell
    */

    // For animation, we want to draw the maze every tick

    // current.draw();

    noStroke();
    fill(255, 0, 0);  // draw current in different color
    rect(current.x * cellSize, current.y * cellSize, cellSize, cellSize);

    previous.draw();
    previous = current;

    var neighbors = checkNeighbors(current.x, current.y);

    if (neighbors.length) {
      var randomNeighbor = neighbors[floor(random(0, neighbors.length))];
      stack.push(randomNeighbor);
      removeWall(current, randomNeighbor);
      randomNeighbor.visited = true;
      current = randomNeighbor;
    }
    else if (stack.length) {
      current = stack.pop();
    }
  }

  function bitTest(num, bit) {
    return ((num >> bit) % 2 != 0);
  }

  function bitClear(num, bit) {
      return num & ~(1 << bit);
  }

  function getCell(x, y) {
    if (x < 0 || x > columns-1 || y < 0 || y > rows)
      return undefined;
    else
      return grid[x + (y * columns)];
      
  }

  function checkNeighbors(x, y) {
    var unvisited = [];
    var top = getCell(x, y-1);
    var right = getCell(x+1, y);
    var bottom = getCell(x, y+1);
    var left = getCell(x-1, y);

    if (top && !top.visited)
      unvisited.push(top);
    if (right && !right.visited)
      unvisited.push(right);
    if (bottom && !bottom.visited)
      unvisited.push(bottom);
    if (left && !left.visited)
      unvisited.push(left);

    return unvisited;
  }

  function removeWall(cell1, cell2) {
    if (cell1.y > cell2.y) { // top wall
      cell1.walls = bitClear(cell1.walls, 0);
      cell2.walls = bitClear(cell2.walls, 2);
    }
    else if (cell1.x < cell2.x) { // right wall
      cell1.walls = bitClear(cell1.walls, 1);
      cell2.walls = bitClear(cell2.walls, 3);
    }
    else if (cell1.y < cell2.y) { // bottom
      cell1.walls = bitClear(cell1.walls, 2);
      cell2.walls = bitClear(cell2.walls, 0);
    }
    else if (cell1.x > cell2.x) { // left wall
      cell1.walls = bitClear(cell1.walls, 3);
      cell2.walls = bitClear(cell2.walls, 1);
    }
  }

  // Cell object
  function Cell(x, y) {
    this.x = x;
    this.y = y;
    this.visited = false;

    // present each wall as a bit
    // 0b - top - right - bottom - left
    this.walls = parseInt('1111', 2);

    this.draw = function() {
      var i = this.x * cellSize;
      var j = this.y * cellSize;

      noStroke();
      if (this.visited) {
        fill(255,180,217);
        rect(i, j, cellSize , cellSize);
      } else {
        fill(255,240,247);
        rect(i, j, cellSize , cellSize);   
      }

      strokeCap(SQUARE);
      stroke(0);
      if (bitTest(this.walls, 0)) { // top
        line(i, j, i+cellSize, j);
      }
      if (bitTest(this.walls, 1)) { // right
        line(i+cellSize, j, i+cellSize, j+cellSize);
      }
      if (bitTest(this.walls, 2)) { // bottom
        line(i, j+cellSize, i+cellSize, j+cellSize);
      }
      if (bitTest(this.walls, 3)) { // left
        line(i, j, i, j+cellSize);
      }
    }
  }
}