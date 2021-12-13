#include "StateGameObject.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"

NCL::CSC8503::StateGameObject::StateGameObject()
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
		return this->counter > 3.0f;
		});
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return this->counter < 0.0f;
		});

	stateMachine->AddState(A);
	stateMachine->AddState(B);
	stateMachine->AddTransition(stateAB);
	stateMachine->AddTransition(stateBA);
}

NCL::CSC8503::StateGameObject::~StateGameObject()
{
	delete stateMachine;
}

void NCL::CSC8503::StateGameObject::Update(float dt)
{
	stateMachine->Update(dt);
}

void NCL::CSC8503::StateGameObject::MoveLeft(float dt)
{
	GetPhysicsObject()->AddForce({ -100,0,0 });
	counter += dt;
}

void NCL::CSC8503::StateGameObject::MoveRight(float dt)
{
	GetPhysicsObject()->AddForce({ 100,0,0 });
	counter -= dt;
}
