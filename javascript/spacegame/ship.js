var LAZORSPEED = 10;
var LAZOR_CLIP = 1500;
var CONSTRAIN_X = 500;
var CONSTRAIN_Y = 300;

function Ship() {
  
  this.model = loadModel('assets/spaceship.obj', true);
  this.texture = loadImage('assets/spaceship.png');
  this.position = new p5.Vector(0,0,0);
  this.tilt = new p5.Vector(0,0,0);
  this.lazors = [];
  
  this.shootingCounter = 0;
  
  this.draw = function() {
    /* Movement */
    translate(this.position.x, this.position.y, this.position.z);
    
    /* Rotation and ship */
    push();

    rotateX(this.tilt.x);
    rotateY(this.tilt.y);
    rotateZ(this.tilt.z);

    texture(this.texture);
    model(this.model);

    pop();
  }
  
  this.update = function() {
    /* Remove projectiles that are too far away */
    for (i=0; i<this.lazors.length; i++) {
      this.lazors[i].add(0, 0, LAZORSPEED);
      if (this.lazors[i].z >= LAZOR_CLIP) {
        this.lazors.shift();
      }
    }
  }
  
  this.move = function(x, y, z) {
    if ((this.position.x + x) < CONSTRAIN_X && (this.position.x + x) > -CONSTRAIN_X) {
      this.position.x += x; //.add(x,y,z);
    }
    if ((this.position.y + y) < CONSTRAIN_Y && (this.position.y + y) > -CONSTRAIN_Y){
      this.position.y += y; //.add(x,y,z);
    }
  }
  
  this.rotate = function(pitch, yaw, roll) {
    this.tilt.add(pitch, roll, yaw)
  }
  
  this.shoot = function() {
    if (millis() - this.shootingCounter >= 100) {
      var location = this.position.copy();
      this.lazors.push(location)
      this.shootingCounter = millis();
    }
  }

  this.reset = function() {
    this.position = new p5.Vector(0,0,0);
    this.tilt = new p5.Vector(0,0,0);
    this.lazors = [];
  }
  
}
