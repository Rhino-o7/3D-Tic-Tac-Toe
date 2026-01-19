#pragma once

#include <glm.hpp>


class Camera
{
private:
	glm::vec3 m_Position;
	glm::vec3 m_Rotation;
	glm::vec3 m_Target;
	bool m_UseTarget;
	float m_Fov;
	float m_AspectRatio;
	float m_NearClip;
	float m_FarClip;
public:
	Camera(float fov, float aspectRatio, float nearClip, float farClip);
	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::vec3& rotation);
	void SetTarget(const glm::vec3& target);
	void DisableTargetTracking();
	const glm::vec3& GetPosition() const;
	const glm::vec3& GetRotation() const;
	const glm::vec3& GetTarget() const;
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;
	void SetAspectRatio(float aspectRatio);
	float GetAspectRatio() const;
};

