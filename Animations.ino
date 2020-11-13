int animationState;

void startAnimation() {
  animationState = 0;
  pitchStepper.setCurrentPosition(0);
  yawStepper.setCurrentPosition(0);

  if(animation == SWEEP_HORIZONTAL) {
    pitchStepper.moveTo(175);
  } else if(animation == SWEEP_VERTICAL) {
    yawStepper.moveTo(50);
  } else if(animation == SWEEP_DIAGONAL_LR) {
    pitchStepper.moveTo(50);
    yawStepper.moveTo(75);
  } else if(animation == SWEEP_DIAGONAL_RL) {
    pitchStepper.moveTo(-50);
    yawStepper.moveTo(-75);
  }
}

void loopAnimation() {
  if(animation == SWEEP_HORIZONTAL) {
    runPitch();
  } else if(animation == SWEEP_VERTICAL) {
    runYaw();
  } else if(animation == SWEEP_DIAGONAL_LR || animation == SWEEP_DIAGONAL_RL) {
    runPitch();
    runYaw();
  } else if(animation == CIRCLE) {
    
    
  }
}

void runPitch() {
  if(pitchStepper.distanceToGo() == 0) {
    pitchStepper.moveTo(-pitchStepper.currentPosition());
  }
  pitchStepper.run();
}

void runYaw() {
  if(yawStepper.distanceToGo() == 0) {
    yawStepper.moveTo(-yawStepper.currentPosition());
  }
  yawStepper.run();
}
