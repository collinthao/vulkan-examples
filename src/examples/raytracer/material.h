#pragma once

#include "hittable.h"

class material
{
	public:
	virtual ~material() = default;
	
	virtual bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const
	{
		return false;
	};
};

class lambertian : public material
{
	public:
	lambertian(const glm::vec3& albedo)
	: albedo(albedo) {};

	bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		auto scatter_direction = rec.normal + glm_ext::random_unit_vector();
		
		if (glm_ext::near_zero(scatter_direction))
		{
			scatter_direction = rec.normal;
		};

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	};
	
	private:
	glm::vec3 albedo;
};

class metal : public material
{
	public:
	metal(const glm::vec3& albedo, double fuzz)
	: albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1)
	{};

	bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 reflected = reflect(r_in.direction(), rec.normal);
		reflected = glm::normalize(reflected) + (fuzz * glm_ext::random_unit_vector());
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return (glm::dot(scattered.direction(), rec.normal) > 0); // we make sure that the random unit vector does not fall beneath 0 and return a negative color contribution
	};
	
	private:
	glm::vec3  albedo;
	double fuzz;
};

class dielectric : public material
{
	public:
	dielectric(double refraction_index)
	: refraction_index(refraction_index)	
	{};

	bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		attenuation = glm::vec3(1.);
		double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

		glm::vec3 unit_direction = glm::normalize(r_in.direction());
		double cos_theta = std::fmin(glm::dot(-unit_direction, rec.normal), 1.0);
		double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
		
		bool cannot_refract = ri * sin_theta > 1.0;
		glm::vec3 direction;
	
		if (cannot_refract)
		{
			direction = glm_ext::reflect(unit_direction, rec.normal);
		}
		else
		{
			direction = glm_ext::refract(unit_direction, rec.normal, ri);
		};

		scattered = ray(rec.p, direction);
		return true;
	};
	
	private:
	double refraction_index;
};

