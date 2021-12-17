#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "..\..\Common\Window.h"
#include "..\..\Common\Keyboard.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class ThrusterObject : public GameObject {
		public:
			ThrusterObject();
			~ThrusterObject();

			virtual void Update(float dt);

		protected:
			void MoveUp(float dt);
			void MoveDown(float dt);

			StateMachine* stateMachine;
			float counter;
		};
	}
}