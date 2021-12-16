#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector3.h"
namespace NCL {
	class PlaneVolume : CollisionVolume
	{
	public:
		PlaneVolume(const int a) {
			type = VolumeType::Plane;
			axis = a;
		}
		~PlaneVolume() {}

		int GetAxis() const {
			return axis;
		}
	protected:
		int axis; //0 = x, 1 = y, 2 = z
	};
}