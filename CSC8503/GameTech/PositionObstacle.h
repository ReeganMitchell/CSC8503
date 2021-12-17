#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "..\..\Common\Window.h"
#include "..\..\Common\Keyboard.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class PositionObstacle : public GameObject {
		public:
			PositionObstacle();
			~PositionObstacle();

			virtual void Update(float dt);

		protected:
			void MoveRight(float dt);
			void MoveLeft(float dt);

			StateMachine* stateMachine;
			float counter;
		};
	}
}