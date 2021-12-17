#include "PositionObstacle.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"

NCL::CSC8503::PositionObstacle::PositionObstacle()
{
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* A = new State([&](float dt)->void {
		this->MoveLeft(dt);
	});
	State* B = new State([&](float dt)->void {
		this->MoveRight(dt);
	});
	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
		return this->counter > 2.0f;
	});
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return this->counter < 0.0f;
	});

	stateMachine->AddState(A);
	stateMachine->AddState(B);
	stateMachine->AddTransition(stateAB);
	stateMachine->AddTransition(stateBA);
}

NCL::CSC8503::PositionObstacle::~PositionObstacle()
{
	delete stateMachine;
}

void NCL::CSC8503::PositionObstacle::Update(float dt)
{
	stateMachine->Update(dt);
}

void NCL::CSC8503::PositionObstacle::MoveRight(float dt)
{
	GetPhysicsObject()->AddForce({ 2,0,0 });
	GetPhysicsObject()->ApplyAngularImpulse(Vector3(0.1, 0.1, 0.1));
	counter -= dt;
}

void NCL::CSC8503::PositionObstacle::MoveLeft(float dt)
{
	GetPhysicsObject()->AddForce({ -2,0,0 });
	GetPhysicsObject()->ApplyAngularImpulse(Vector3(-0.1, -0.1, -0.1));
	counter += dt;
}
