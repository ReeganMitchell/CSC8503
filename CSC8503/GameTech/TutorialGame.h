#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "StateGameObject.h"
#include "ThrusterObject.h"
#include "PositionObstacle.h"
#include "RotationObstacle.h"
#include "Enemy.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			bool close = false;

		protected:
			void InitialiseAssets();

			void InitCameraMenu();
			void InitCamera1();
			void InitCamera2();

			void UpdateKeys();

			void InitWorld();

			int score; 
			float totalTime = 0;

			bool menu;
			bool win;
			bool lose;
			int nextLevel;
			int stage;

			void InitMenu();
			void InitResultScreen(bool win);

			void ChooseStage();
			void InitLevel1A();
			void InitLevel1B();
			void InitLevel1C();
			void InitLevel2();

			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddPlaneToWorld(const Vector3& position, const int axis);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, string name = "");
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, string name = "");
			ThrusterObject* AddThrusterToWorld(const Vector3& position, Vector3 dimensions);
			ThrusterObject* thruster;
			PositionObstacle* AddPositionObstacleToWorld(float radius);
			PositionObstacle* posObs;
			RotationObstacle* AddRotationObstacleToWorld();
			RotationObstacle* rotObs;
			Enemy* enemy1;
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f, string name = "");

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			//states

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject * testStateObject;
		};
	}
}

