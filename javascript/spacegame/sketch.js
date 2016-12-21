var MOVESPEED = 10;

function setup() {
  var canvas = createCanvas(window.innerWidth, window.innerHeight, WEBGL);

  ship = new Ship();
  world = new World();

  FSM = new FiniteStateMachine("INTRO");
  initializeStates(FSM);

  counter = 0;
}

function draw(){

  background(0,24,72);

  ambientLight(1);
  pointLight(50, 50, 50, 0, 300, 0);
  pointLight(50, 50, 50, 0, -300, 0);
  directionalLight(1,1,1, 100,100,100);

  FSM.run();
}