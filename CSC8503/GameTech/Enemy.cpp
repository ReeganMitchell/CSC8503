#include "Enemy.h"
#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/NavigationGrid.cpp"

NCL::CSC8503::Enemy::Enemy()
{
	name = "Enemy";

	BehaviourAction* findTargetNode = new BehaviourAction("Find Target Node", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			if (pathNodes.empty()) {
				return Failure;
			}
			else if (targetNode >= pathNodes.size()){
				return Failure;
			}
			else {
				targetPos = pathNodes[targetNode];
				return Success;
			}
		}
		return state;
	});
	BehaviourAction* MoveToTarget = new BehaviourAction("Move To Target", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise || state == Ongoing) {
			Vector3 delta = this->GetTransform().GetPosition() - targetPos;

			Vector3 direction = delta.Normalised();

			this->GetPhysicsObject()->AddForce(direction * 10);

			if (delta.Length() < 1) {
				return Success;
			}
			else {
				state = Ongoing;
			}
			return state;
		}
	});
	BehaviourAction* ChooseNextTarget = new BehaviourAction(" Choose Next Target", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			targetNode++;
			return Success;
		}
		return state;
	});

	BehaviourSequence* MoveSequence = new BehaviourSequence("Move Sequence");
	MoveSequence->AddChild(findTargetNode);
	MoveSequence->AddChild(MoveToTarget);
	MoveSequence->AddChild(ChooseNextTarget);


	BehaviourAction* chooseAction = new BehaviourAction("Choose Action", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			pauseCounter = 3.0f;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (pauseCounter <= 0.0f) {
				int i = rand() % 3;
				endPoint = points[i];
				return Success;
			}
			else {
				pauseCounter = pauseCounter - dt;
				state = Ongoing;
			}
		}
		return state;
	});
	BehaviourAction* runPathfinding = new BehaviourAction("Run Pathfinding", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			FindNewPath();
			return Success;
		}
		else {
			return Failure;
		}
	});

	BehaviourSequence* DecideSequence = new BehaviourSequence("Decide Sequence");
	DecideSequence->AddChild(chooseAction);
	DecideSequence->AddChild(runPathfinding);

	rootSelector = new BehaviourSelector("Root Selector");
	rootSelector->AddChild(MoveSequence);
	rootSelector->AddChild(DecideSequence);
}

NCL::CSC8503::Enemy::~Enemy()
{

}

void NCL::CSC8503::Enemy::Update(float dt)
{
	rootSelector->Execute(dt);
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		Reset();
	}
}

void NCL::CSC8503::Enemy::FindNewPath()
{
	pathNodes.clear();
	outPath.Clear();

	NavigationGrid grid("MazeGrid.txt");

	Vector3 startPos = this->GetTransform().GetPosition();

	bool found = grid.FindPath(startPos, endPoint, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pathNodes.push_back(pos);
	}
	pauseCounter = 3.0f;
	targetNode = 0;
}
