#include <iostream>
/*
inline std::ostream& operator << (std::ostream& os, InstanceData& i)
{
	os << i.pos;
	os << i.scale;
	os << "Rotation: " << i.rot << '\n';
	os << "Id: " << i.id << '\n';
	return os;
}
*/

inline std::ostream& operator << (std::ostream& os, const glm::vec2& v)
{
	os << "x: " << v.x << " | y: " << v.y << '\n';
	return os;
}

inline std::ostream& operator << (std::ostream& os, const glm::vec3& v)
{
	os << "x: " << v.x << " | y: " << v.y << " | z: " << v.z << '\n';
	return os;
}

