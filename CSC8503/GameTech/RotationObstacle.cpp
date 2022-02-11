#include "RotationObstacle.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"

NCL::CSC8503::RotationObstacle::RotationObstacle()
{
	name = "Rotation Obstacle";
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* A = new State([&](float dt)->void {
		this->RotateLeft(dt);
	});
	State* B = new State([&](float dt)->void {
		this->RotateRight(dt);
	});
	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
		return this->counter > 5.0f;
	});
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return this->counter < 0.0f;
	});

	stateMachine->AddState(A);
	stateMachine->AddState(B);
	stateMachine->AddTransition(stateAB);
	stateMachine->AddTransition(stateBA);
}

NCL::CSC8503::RotationObstacle::~RotationObstacle()
{
	delete stateMachine;
}

void NCL::CSC8503::RotationObstacle::Update(float dt)
{
	stateMachine->Update(dt);
}

void NCL::CSC8503::RotationObstacle::RotateRight(float dt)
{
	GetPhysicsObject()->ApplyAngularImpulse(Vector3(0.0, 0, 10.0));
	counter -= dt;
}

void NCL::CSC8503::RotationObstacle::RotateLeft(float dt)
{
	GetPhysicsObject()->ApplyAngularImpulse(Vector3(-0.0, -0, -10.0));
	counter += dt;
}
