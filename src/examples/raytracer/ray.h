#pragma once

inline glm::vec3 operator/(const glm::vec3& vec, double t)
{
	return glm::vec3{vec.x/t, vec.y/t, vec.z/t};
};

inline glm::vec3 operator*(double t, const glm::vec3& vec)
{
	return glm::vec3{t*vec.x, t*vec.y, t*vec.z};
};

class ray
{
	public:
	ray(){};
	
	ray(const glm::vec3& origin, const glm::vec3& direction)
	: orig(origin), dir(direction)	
	{};
		
	const glm::vec3& origin() const {return orig;};
	const glm::vec3& direction() const {return dir;};
	
	glm::vec3 at(double t) const
	{	
		return orig + t*dir;
	};
	
	private:
	glm::vec3 orig;
	glm::vec3 dir;
};
