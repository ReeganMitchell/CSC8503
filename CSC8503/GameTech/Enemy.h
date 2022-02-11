#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "..\..\Common\Window.h"
#include "..\..\Common\Keyboard.h"
#include "../CSC8503Common/NavigationPath.h"
#include "../CSC8503Common/BehaviourSequence.h"
#include "../CSC8503Common/BehaviourSelector.h"

namespace NCL {
	namespace CSC8503 {
		class Enemy : public GameObject {
		public:
			Enemy();
			~Enemy();

			void Update(float dt);

			void FindNewPath();

			void Reset() {
				outPath.Clear();
				pathNodes.clear();
				rootSelector->Reset();
				this->GetTransform().SetPosition(Vector3(18, 0, 2));
			}
		protected:
			float pauseCounter;

			NavigationPath outPath;
			vector<Vector3> pathNodes;
			int targetNode = 0;
			Vector3 targetPos;
			Vector3 endPoint;
			Vector3 points[3] = {
				Vector3(14,0,22),
				Vector3(2,0,2),
				Vector3(22,0,22)
			};

			BehaviourSelector* rootSelector;
		};
	}
}