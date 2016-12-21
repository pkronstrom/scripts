var SPAWNRATE = 50;
var DUST_SPAWNRATE = 100;
var OBJ_SIZE = 100;
var SHIP_DIMENSIONS = 50;
var SPAWN_WINDOW = 2500;
var SPAWN_DEPTH = 8000;

function World() {
  this.model = loadModel('assets/asteroid.obj', true);
  this.obstacles = [];
  this.dust = [];
  this.speed = 0.1;
  this.score = 0;

  this.draw = function() {
    for (i=0; i<this.obstacles.length; i++) {
      var loc = this.obstacles[i].location;
      push();
        ambientMaterial(128,128,128, 255);
        translate(loc.x, loc.y, loc.z);
        scale(this.obstacles[i].size/OBJ_SIZE);
        model(this.model);
      pop();
    }

    for (i=0; i<this.dust.length; i++) {
      var loc = this.dust[i].location;
      push();
        ambientMaterial(255,255,0, 255);
        translate(loc.x, loc.y, loc.z);
        sphere(this.dust[i].size);
      pop();
    }

  }
  
  this.update = function() {
    if (random(0, 100) < SPAWNRATE * this.speed) {
      this.spawnMeteorite();
    }
    if (random(0, 100) < DUST_SPAWNRATE) {
      this.spawnStarDust();
    }
    if (frameCount % 10 === 0) {
      this.speed += 0.01
      this.score += 10;
    };

    for (i=0; i<this.obstacles.length; i++) {
      var loc = this.obstacles[i].location;
      loc.add(0, 0, -this.obstacles[i].speed * this.speed);

      var dist = loc.dist(ship.position);
      if ( dist < (this.obstacles[i].size + SHIP_DIMENSIONS)) {
        /* COLLISION!! */
        FSM.next();
      }

      if (loc.z <= -1000) {
        this.obstacles.splice(i, 1);
      }
    }

    for (i=0; i<this.dust.length; i++) {
      var loc = this.dust[i].location;
     loc.add(0, 0, -this.dust[i].speed);

      if (loc.z <= -200) {
        this.dust.splice(i, 1);
      }
    }

  }

  this.spawnMeteorite = function() {
    var meteorite = {
      location: new p5.Vector(random(-SPAWN_WINDOW, SPAWN_WINDOW), random(-SPAWN_WINDOW, SPAWN_WINDOW), SPAWN_DEPTH),
      speed: random(80, 130),
      size: random(50, 400)
    }
    this.obstacles.push(meteorite);
  }

  this.spawnStarDust = function() {
    var meteorite = {
      location: new p5.Vector(random(-500, 500), random(-500, 500), 700),
      speed: random(50, 200),
      size: random(2, 7)
    }
    this.dust.push(meteorite);
  }

  this.reset = function() {
      this.obstacles = [];
      this.speed = 0.1;
      this.score = 0;
  }
}
