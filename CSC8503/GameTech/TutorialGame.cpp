#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/PositionConstraint.h"
#include <sstream>
#include "Enemy.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = true;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	score = 0;
	menu = 0;
	win = 0;
	lose = 0;

	InitCameraMenu();
	InitWorld();
	InitMenu();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	UpdateKeys();

	if (useGravity) {
		//Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		//Debug::Print("(G)ravity off", Vector2(5, 95));
	}

	if (menu) {
		renderer->DrawString("Play Part A", Vector2(45, 50));
		renderer->DrawString("Play Part B", Vector2(45, 65));
		renderer->DrawString("Exit", Vector2(45, 80));
	}
	if (win) {
		string s = "score = ";
		s.append(std::to_string(score));
		renderer->DrawString(s, Vector2(42, 30));
		renderer->DrawString("You Win!", Vector2(45, 50));
		renderer->DrawString("Return to Menu", Vector2(45, 65));
		renderer->DrawString("Exit", Vector2(45, 80));
	}
	if (lose) {
		string s = "score = ";
		s.append(std::to_string(score));
		renderer->DrawString(s, Vector2(42, 30));
		renderer->DrawString("You Lose.", Vector2(45, 50));
		renderer->DrawString("Return to Menu", Vector2(45, 65));
		renderer->DrawString("Exit", Vector2(45, 80));
	}

	SelectObject();
	MoveSelectedObject();
	int result = physics->Update(dt);

	if (result == 1) {
		//Dead
		nextLevel = 5;
		lose = true;
	}
	else if (result == 2) {
		//Next Level / win
		if (stage != 2) {
			stage++;
			nextLevel = 1;
			score = score + 100;
		}
		else {
			win = true;
			nextLevel = 4;
			score = score + 100;
		}
	}
	else if (result == 3) {
		score = score + 50;
	}

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}

	if (testStateObject) {
		testStateObject->Update(dt);
	}
	if (thruster) {
		thruster->Update(dt);
	}
	if (posObs) {
		posObs->Update(dt);
	}
	if (rotObs) {
		rotObs->Update(dt);
	}
	if (enemy1) {
		enemy1->Update(dt);
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();

	if (nextLevel != 0) {
		selectionObject = nullptr;
		thruster = nullptr;
		posObs = nullptr;
		rotObs = nullptr;
		if (nextLevel == 6) {
			close = true;
		}
		else if (nextLevel == 1) {
 			ChooseStage();
			nextLevel = 0;
			menu = false;
			win = false;
			lose = false;
		}
		else if (nextLevel == 2) {
			InitLevel2();
			nextLevel = 0;
			menu = false;
			win = false;
			lose = false;
		}
		else if (nextLevel == 3) {
			InitMenu();
			nextLevel = 0;
			menu = true;
			win = false;
			lose = false;
		}
		else if (nextLevel == 4) {
			InitResultScreen(true);
			nextLevel = 0;
			menu = false;
			win = true;
			lose = false;
		}
		else if (nextLevel == 5) {
			InitResultScreen(false);
			nextLevel = 0;
			menu = false;
			win = false;
			lose = true;
		}
	}
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCameraMenu(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCameraMenu() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-90.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	world->GetMainCamera()->SetPosition(Vector3(0, 40, 0));
	lockedObject = nullptr;
}

void TutorialGame::InitCamera1() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(0.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	world->GetMainCamera()->SetPosition(Vector3(0, 10, 100));
	lockedObject = nullptr;
}

void TutorialGame::InitCamera2() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-90.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	world->GetMainCamera()->SetPosition(Vector3(12, 40, 12));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	//InitCamera1();
	selectionObject = nullptr;

	//InitMixedGridWorld(2, 1, 3.5f, 3.5f);

	AddCubeToWorld(Vector3(0, 0, 0), Vector3(1, 1, 1), 0.0f, "Menu");
	AddCubeToWorld(Vector3(1, 0, 0), Vector3(1, 1, 1), 0.0f, "Menu");
	AddCubeToWorld(Vector3(2, 0, 0), Vector3(1, 1, 1), 0.0f, "Menu");
	AddCubeToWorld(Vector3(3, 0, 0), Vector3(1, 1, 1), 0.0f, "Menu");

	//InitGameExamples();
	//BridgeConstraintTest();
	//InitDefaultFloor();
	//testStateObject = AddStateObjectToWorld(Vector3(40, 10, 0));
}

void TutorialGame::InitMenu() {
	world->ClearAndErase();
	physics->Clear();
	InitCameraMenu();
	selectionObject = nullptr;
	useGravity = false;
	physics->UseGravity(useGravity);

	score = 0;
	stage = 0;

	menu = true;
	renderer->DrawString("Play Part A", Vector2(10, 10));
	renderer->DrawString("Play Part B", Vector2(10, 15));
	renderer->DrawString("Exit", Vector2(10, 20));

	AddCubeToWorld(Vector3(-7,0,0),Vector3(1,1,1),0,"World1");
	AddSphereToWorld(Vector3(-7, 0, 5), 1, 0,"World2");
	AddCubeToWorld(Vector3(-7, 0, 10), Vector3(1, 1, 1), 0, "Exit");
}

void TutorialGame::InitResultScreen(bool didWin)
{
	world->ClearAndErase();
	physics->Clear();
	InitCameraMenu();
	selectionObject = nullptr;
	useGravity = false;
	physics->UseGravity(useGravity);

	menu = false;
	win = didWin;
	lose = !didWin;
	renderer->DrawString("You Win!", Vector2(10, 5));
	renderer->DrawString("Return to Menu", Vector2(10, 10));
	renderer->DrawString("Exit", Vector2(10, 20));

	AddCubeToWorld(Vector3(-7, 0, 5), Vector3(1, 1, 1), 0, "Menu");
	AddCubeToWorld(Vector3(-7, 0, 10), Vector3(1, 1, 1), 0, "Exit");
}

void TutorialGame::ChooseStage()
{
	useGravity = true;
	physics->UseGravity(useGravity);
	if (stage == 0) {
		InitLevel1A();
	}
	if (stage == 1) {
		InitLevel1B();
	}
	if (stage == 2) {
		InitLevel1C();
	}
}

void TutorialGame::InitLevel1A()
{
	world->ClearAndErase();
	physics->Clear();
	InitCamera1();
	useGravity = true;

	AddCubeToWorld(Vector3(0, 40, 0), Vector3(4, 1, 2), 0, "End");
	thruster = AddThrusterToWorld(Vector3(0, 0, 0), Vector3(4, 1, 2));
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddPlaneToWorld(Vector3(0, 0, 0), 1);

	selectionObject = nullptr;
}

void TutorialGame::InitLevel1B()
{
	world->ClearAndErase();
	physics->Clear();
	InitCamera1();
	useGravity = true;

	AddCubeToWorld(Vector3(0, 40, 0), Vector3(4, 1, 2), 0, "End");
	thruster = AddThrusterToWorld(Vector3(0, 0, 0), Vector3(4, 1, 2));
	posObs = AddPositionObstacleToWorld(2);
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddPlaneToWorld(Vector3(0, 0, 0), 1);

	selectionObject = nullptr;
}

void TutorialGame::InitLevel1C()
{
	world->ClearAndErase();
	physics->Clear();
	InitCamera1();
	useGravity = true;

	AddCubeToWorld(Vector3(0, 40, 0), Vector3(4, 1, 2), 0, "End");
	thruster = AddThrusterToWorld(Vector3(0, 0, 0), Vector3(4, 1, 2));
	rotObs = AddRotationObstacleToWorld();
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddPlaneToWorld(Vector3(0, 0, 0), 1);
	AddBonusToWorld(Vector3(-10, 15, 0));

	selectionObject = nullptr;
}

void TutorialGame::InitLevel2()
{
	world->ClearAndErase();
	physics->Clear();
	InitCamera2();

	useGravity = true;
	physics->UseGravity(useGravity);

	AddSphereToWorld(Vector3(14, 0, 22),0.5,10.0f,"Player");
	AddEnemyToWorld(Vector3(18, 0, 2));

	//Init Maze

	AddFloorToWorld(Vector3(12, -2, 12));

	for (int i = 0; i < 13; i++) {
		AddCubeToWorld(Vector3(i, 0, 0) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	}
	for (int i = 1; i < 13; i++) {
		AddCubeToWorld(Vector3(0, 0, i) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	}
	for (int i = 1; i < 13; i++) {
		AddCubeToWorld(Vector3(12, 0, i) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	}
	for (int i = 1; i < 12; i++) {
		AddCubeToWorld(Vector3(i, 0, 12) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	}
	AddCubeToWorld(Vector3(8, 0, 1) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(1, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(2, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(4, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(5, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(6, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(9, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(10, 0, 2) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(4, 0, 3) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 3) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(2, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(3, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(4, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(5, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(6, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(10, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(11, 0, 4) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(2, 0, 5) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 5) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(2, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(3, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(4, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(5, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(6, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(10, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(9, 0, 6) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(2, 0, 7) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(6, 0, 7) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(10, 0, 7) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(2, 0, 8) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(4, 0, 8) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(6, 0, 8) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(7, 0, 8) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 8) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(10, 0, 8) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(2, 0, 9) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(4, 0, 9) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 9) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(10, 0, 9) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(2, 0, 10) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(4, 0, 10) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(5, 0, 10) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(6, 0, 10) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 10) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(10, 0, 10) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	AddCubeToWorld(Vector3(6, 0, 11) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");
	AddCubeToWorld(Vector3(8, 0, 11) * 2, Vector3(1, 1, 1) * 0.99f, 0.0f, "Wall");

	selectionObject = nullptr;
}
void TutorialGame::BridgeConstraintTest() {

	Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 24; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(00, 00, 00);

	GameObject * start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject * end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject * previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject * block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint * constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint * constraint = new PositionConstraint(previous,end, maxDistance);
	world->AddConstraint(constraint);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("Floor");

	Vector3 floorSize	= Vector3(12, 1, 12);
	AABBVolume* volume	= new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddPlaneToWorld(const Vector3& position, const int axis) {
	GameObject* plane = new GameObject("KillPlane");

	PlaneVolume* volume = new PlaneVolume(1);
	plane->SetBoundingVolume((CollisionVolume*)volume);
	plane->GetTransform().SetPosition(Vector3(0, 0, 0));

	plane->SetPhysicsObject(new PhysicsObject(&plane->GetTransform(), plane->GetBoundingVolume()));

	plane->GetPhysicsObject()->SetInverseMass(0);
	plane->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(plane);

	return plane;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, string name) {
	GameObject* sphere = new GameObject(name);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass, string name) {
	GameObject* capsule = new GameObject(name);

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, string name ) {
	GameObject* cube = new GameObject(name);

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

ThrusterObject* TutorialGame::AddThrusterToWorld(const Vector3& position, Vector3 dimensions)
{
	GameObject* cube = new GameObject("Base");
	AABBVolume* volumeC = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volumeC);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);


	ThrusterObject* newThruster = new ThrusterObject();
	AABBVolume* volumeT = new AABBVolume(dimensions);
	
	newThruster->SetBoundingVolume((CollisionVolume*)volumeT);
	newThruster->GetTransform()
		.SetPosition(Vector3(position.x, position.y + 2, position.z))
		.SetScale(dimensions * 2);

	newThruster->SetRenderObject(new RenderObject(&newThruster->GetTransform(), cubeMesh, basicTex, basicShader));
	PhysicsObject* po = new PhysicsObject(&newThruster->GetTransform(), newThruster->GetBoundingVolume());
	po->SetElasticity(0.5f);
	newThruster->SetPhysicsObject(po);

	newThruster->GetPhysicsObject()->SetInverseMass(10.0f);
	newThruster->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(newThruster);

	PositionConstraint* constraint = new PositionConstraint(cube, newThruster, 5);

	world->AddConstraint(constraint);

	return newThruster;
}

PositionObstacle* TutorialGame::AddPositionObstacleToWorld(float radius)
{
	GameObject* clamp = new GameObject("Obstacle Clamp");
	AABBVolume* volume = new AABBVolume(Vector3(0.5f, 0.5f, 0.5f));

	clamp->SetBoundingVolume((CollisionVolume*)volume);

	clamp->GetTransform().SetPosition(Vector3(0,41,0)).SetScale(Vector3(1,1,1));

	clamp->SetRenderObject(new RenderObject(&clamp->GetTransform(), cubeMesh, basicTex, basicShader));
	clamp->SetPhysicsObject(new PhysicsObject(&clamp->GetTransform(), clamp->GetBoundingVolume()));

	clamp->GetPhysicsObject()->SetInverseMass(0);
	clamp->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(clamp);

	PositionObstacle* obs = new PositionObstacle();
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volumeS = new SphereVolume(radius);
	obs->SetBoundingVolume((CollisionVolume*)volumeS);

	obs->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(Vector3(2,20,0));

	obs->SetRenderObject(new RenderObject(&obs->GetTransform(), sphereMesh, basicTex, basicShader));
	obs->SetPhysicsObject(new PhysicsObject(&obs->GetTransform(), obs->GetBoundingVolume()));

	obs->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	obs->GetPhysicsObject()->SetInverseMass(5.0f);
	obs->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(obs);

	PositionConstraint* constraint = new PositionConstraint(obs, clamp, 20, 19);

	world->AddConstraint(constraint);

	return obs;
}

RotationObstacle* TutorialGame::AddRotationObstacleToWorld()
{
	GameObject* clamp = new GameObject("Obstacle Clamp");
	AABBVolume* volumeC = new AABBVolume(Vector3(0.5f, 0.5f, 0.5f));

	clamp->SetBoundingVolume((CollisionVolume*)volumeC);

	clamp->GetTransform().SetPosition(Vector3(5, 41, 0)).SetScale(Vector3(1, 1, 1));

	clamp->SetRenderObject(new RenderObject(&clamp->GetTransform(), cubeMesh, basicTex, basicShader));
	clamp->SetPhysicsObject(new PhysicsObject(&clamp->GetTransform(), clamp->GetBoundingVolume()));

	clamp->GetPhysicsObject()->SetInverseMass(0);
	clamp->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(clamp);


	RotationObstacle* obs = new RotationObstacle();
	OBBVolume* volume = new OBBVolume(Vector3(10,1,1));

	obs->SetBoundingVolume((CollisionVolume*)volume);

	obs->GetTransform().SetPosition(Vector3(5, 21, 0)).SetScale(Vector3(20, 2, 2));

	obs->SetRenderObject(new RenderObject(&obs->GetTransform(), cubeMesh, basicTex, basicShader));
	obs->SetPhysicsObject(new PhysicsObject(&obs->GetTransform(), obs->GetBoundingVolume()));

	obs->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	obs->GetPhysicsObject()->SetInverseMass(0.1);
	obs->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(obs);

	PositionConstraint* constraint = new PositionConstraint(obs, clamp, 20, 19);
	world->AddConstraint(constraint);

	return obs;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	GameObject* sphere = new GameObject("Player");

	Vector3 sphereSize = Vector3(1.0f, 1.0f, 1.0f);
	SphereVolume* volume = new SphereVolume(1.0f);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	PhysicsObject* po = new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume());
	po->SetElasticity(2.0f);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(po);

	sphere->GetPhysicsObject()->SetInverseMass(10.0f);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

//GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
//	float meshSize = 3.0f;
//	float inverseMass = 0.5f;
//
//	GameObject* character = new GameObject();
//
//	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);
//
//	character->SetBoundingVolume((CollisionVolume*)volume);
//
//	character->GetTransform()
//		.SetScale(Vector3(meshSize, meshSize, meshSize))
//		.SetPosition(position);
//
//	if (rand() % 2) {
//		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
//	}
//	else {
//		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
//	}
//	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
//
//	character->GetPhysicsObject()->SetInverseMass(inverseMass);
//	character->GetPhysicsObject()->InitSphereInertia();
//
//	world->AddGameObject(character);
//
//	//lockedObject = character;
//
//	return character;
//}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	Enemy* enemy = new Enemy();

	float radius = 0.5;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volumeS = new SphereVolume(radius);
	enemy->SetBoundingVolume((CollisionVolume*)volumeS);

	enemy->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	enemy->SetRenderObject(new RenderObject(&enemy->GetTransform(), sphereMesh, basicTex, basicShader));
	enemy->SetPhysicsObject(new PhysicsObject(&enemy->GetTransform(), enemy->GetBoundingVolume()));

	enemy->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	enemy->GetPhysicsObject()->SetInverseMass(10.0f);
	enemy->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(enemy);
	enemy1 = enemy;

	return enemy;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("Bonus");

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));

	apple->GetPhysicsObject()->SetInverseMass(0.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		//renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				lockedObject	= nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		//renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		//renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		//renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));

		string name = "Name = ";
		name.append(selectionObject->GetName());

		string pos = "Position = ";
		pos.append(std::to_string(selectionObject->GetTransform().GetPosition().x));
		pos.append(", ");
		pos.append(std::to_string(selectionObject->GetTransform().GetPosition().y));
		pos.append(", ");
		pos.append(std::to_string(selectionObject->GetTransform().GetPosition().z));

		string vel = "Velocity = ";
		vel.append(std::to_string(selectionObject->GetPhysicsObject()->GetLinearVelocity().x));
		vel.append(", ");
		vel.append(std::to_string(selectionObject->GetPhysicsObject()->GetLinearVelocity().y));
		vel.append(", ");
		vel.append(std::to_string(selectionObject->GetPhysicsObject()->GetLinearVelocity().z));

		renderer->DrawString("Debug", Vector2(5, 75));
		renderer->DrawString(name, Vector2(5, 80));
		renderer->DrawString(pos, Vector2(5, 85));
		renderer->DrawString(vel, Vector2(5, 90));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	//renderer->DrawString("Click Force: " + std::to_string(forceMagnitude), Vector2(10, 20));

	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;
	}

	if (menu || win || lose) {
		if (selectionObject->GetName() == "Exit") {
			close = true;
		}
		if (selectionObject->GetName() == "World1") {
			nextLevel = 1;
		}
		if (selectionObject->GetName() == "World2") {
			nextLevel = 2;
		}
		if (selectionObject->GetName() == "Menu") {
			nextLevel = 3;
		}
	}

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;

		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				//selectionObject->GetPhysicsObject()->AddForce(ray.GetDirection() * forceMagnitude);

				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}