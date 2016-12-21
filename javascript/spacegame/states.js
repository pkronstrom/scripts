function initializeStates(FSM) {

	/* Initialize the game state machine */
	var IntroState = new State("INTRO", "GAME", introStateRun);
	var GameState = new State("GAME", "GAMEOVER", gameStateRun, gameStateInit);
	var GameOverState = new State("GAMEOVER", "INTRO", gameOverStateRun, gameOverStateInit);

	FSM.addState(IntroState);
	FSM.addState(GameState);
	FSM.addState(GameOverState);

	function introStateRun() {
		/* Check the input */
		if (keyIsPressed) {  // SPACEBAR
		  FSM.next();
		}
  	}

	function gameStateInit() {
		select('#overlay').hide();
	}

	function gameStateRun() {
		/* Check the input */
		ship.tilt.set(0, 0, 0);

		if (keyIsDown(LEFT_ARROW)) {
			ship.move(MOVESPEED,0, 0);
			ship.tilt.add(0, 0, -PI/8);
		}
		if (keyIsDown(RIGHT_ARROW)) {
			ship.move(-MOVESPEED,0,0);
			ship.tilt.add(0, 0, PI/8);
		}
		if (keyIsDown(UP_ARROW)) {
			ship.move(0, -MOVESPEED, 0);
			ship.tilt.add(-PI/8, 0, 0);
		}
		if (keyIsDown(DOWN_ARROW)) {
			ship.move(0, MOVESPEED,0);
			ship.tilt.add(PI/8, 0 , 0);
		}

		if (keyIsDown(ESCAPE)) {
			FSM.next();
		}

		/* Draw */
		rotateY(Math.PI);

		world.update();
		ship.update();
		world.draw();
		ship.draw();
	}

	function gameOverStateInit() {
		var textbox = select('#overlay');
		textbox.html("GAME OVER!<br />SCORE: " + world.score);
		textbox.show();

		world.reset();
		ship.reset();
	}

	function gameOverStateRun() {
		if (keyIsDown(32)) {
			FSM.next();
		}
	}
}

