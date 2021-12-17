#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "..\..\Common\Window.h"
#include "..\..\Common\Keyboard.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class RotationObstacle : public GameObject {
		public:
			RotationObstacle();
			~RotationObstacle();

			virtual void Update(float dt);

		protected:
			void RotateRight(float dt);
			void RotateLeft(float dt);

			StateMachine* stateMachine;
			float counter;
		};
	}
}