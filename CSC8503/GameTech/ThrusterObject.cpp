#include "ThrusterObject.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"


using namespace NCL;
using namespace CSC8503;

NCL::CSC8503::ThrusterObject::ThrusterObject()
{
	name = "Thruster";
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* A = new State([&](float dt)->void {
		this->MoveUp(dt);
	});
	State* B = new State([&](float dt)->void {
		this->MoveDown(dt);
	});
	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
		return !Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE);
	});
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE);
	});

	stateMachine->AddState(A);
	stateMachine->AddState(B);
	stateMachine->AddTransition(stateAB);
	stateMachine->AddTransition(stateBA);
}

NCL::CSC8503::ThrusterObject::~ThrusterObject()
{
	delete stateMachine;
}

void NCL::CSC8503::ThrusterObject::Update(float dt)
{
	stateMachine->Update(dt);
}

void NCL::CSC8503::ThrusterObject::MoveUp(float dt)
{
	GetPhysicsObject()->AddForce(Vector3(0, 15, 0));
}

void NCL::CSC8503::ThrusterObject::MoveDown(float dt)
{
	GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, -0.1, 0));
}
