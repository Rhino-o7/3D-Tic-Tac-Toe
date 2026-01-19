#include "Camera.h"
#include <gtc/matrix_transform.hpp>

Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
	: m_Position(0.0f), m_Rotation(0.0f), m_Target(0.0f), m_UseTarget(false),
	  m_Fov(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
{
}

void Camera::SetPosition(const glm::vec3& position)
{
	m_Position = position;
}

void Camera::SetRotation(const glm::vec3& rotation)
{
	m_Rotation = rotation;
	m_UseTarget = false;
}

void Camera::SetTarget(const glm::vec3& target)
{
	m_Target = target;
	m_UseTarget = true;
}

void Camera::DisableTargetTracking()
{
	m_UseTarget = false;
}

const glm::vec3& Camera::GetPosition() const
{
	return m_Position;
}

const glm::vec3& Camera::GetRotation() const
{
	return m_Rotation;
}

const glm::vec3& Camera::GetTarget() const
{
	return m_Target;
}

glm::mat4 Camera::GetViewMatrix() const
{
	if (m_UseTarget)
	{
		return glm::lookAt(m_Position, m_Target, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	const float pitch = glm::radians(m_Rotation.x);
	const float yaw = glm::radians(m_Rotation.y);

	glm::vec3 forward;
	forward.x = cosf(pitch) * sinf(yaw);
	forward.y = sinf(pitch);
	forward.z = cosf(pitch) * cosf(yaw);
	forward = glm::normalize(forward);

	const glm::vec3 target = m_Position + forward;
	return glm::lookAt(m_Position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearClip, m_FarClip);
}

void Camera::SetAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
}

float Camera::GetAspectRatio() const
{
	return m_AspectRatio;
}
