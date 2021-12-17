#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class PositionConstraint : public Constraint {
		public:
			PositionConstraint(GameObject* a, GameObject* b, float d) {
				objectA = a;
				objectB = b;
				distance = d;
				minDistance = 0;
			}
			PositionConstraint(GameObject* a, GameObject* b, float d, float m) {
				objectA = a;
				objectB = b;
				distance = d;
				minDistance = m;
			}
			~PositionConstraint() {}

			void UpdateConstraint(float dt) override;
		protected:
			GameObject* objectA;
			GameObject* objectB;

			float distance;
			float minDistance;
		};
	}
}