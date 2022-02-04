#include "stdafx.h"
#include "client/object/component/util/camera_component.h"
#include "client/object/actor/core/actor.h"
#include "client/renderer/core/render.h"
#include "client/physics/core/bounding_mesh.h"

namespace client_fw
{
	CameraComponent::CameraComponent(const std::string& name, eCameraUsage usage)
		: Component(name), m_camera_state(eCameraState::kPaused)
		, m_camera_usage(usage), m_projection_mode(eProjectionMode::kPerspective)
	{
	}

	bool CameraComponent::Initialize()
	{
		return RegisterToRenderSystem();
	}

	void CameraComponent::Shutdown()
	{
		UnregisterFromRenderSystem();
	}

	void CameraComponent::UpdateWorldMatrix()
	{
		const auto& owner = GetOwner().lock();
		Vec3 eye = owner->GetPosition();
		Vec3 target, up;
		if (m_owner_controller.expired())
		{
			target = eye + owner->GetForward();
			up = owner->GetUp();
		}
		else
		{
			target = eye + m_owner_controller.lock()->GetForward();
			up = m_owner_controller.lock()->GetUp();
		}

		m_view_matrix = mat4::LookAt(eye, target, up);
		m_inverse_view_matrix = mat4::Inverse(m_view_matrix);
		m_bounding_frustum.Transform(m_bf_projection, m_inverse_view_matrix);
	}

	void CameraComponent::UpdateViewport(float left, float top, float width, float height)
	{
		m_viewport.left = left;
		m_viewport.width = width;
		m_viewport.top = top;
		m_viewport.height = height;
		SetAspectRatio(width / height);
		UpdateProjectionMatrix();
	}

	void CameraComponent::UpdateProjectionMatrix()
	{
		switch (m_projection_mode)
		{
		case client_fw::eProjectionMode::kPerspective:
			m_projection_matrix = GetPerspectiveMatrix();
			break;
		case client_fw::eProjectionMode::kOrthographic:
			m_projection_matrix = GetOrthoMatrix();
			break;
		}
		m_bf_projection = BFrustum(m_projection_matrix);
	}

	bool CameraComponent::RegisterToRenderSystem()
	{
		if (Render::RegisterCameraComponent(shared_from_this()))
			return true;
		return false;
	}

	void CameraComponent::UnregisterFromRenderSystem()
	{
		Render::UnregisterCameraComponent(shared_from_this());
	}

	Mat4 CameraComponent::GetPerspectiveMatrix() const
	{
		return mat4::Perspective(math::ToRadian(m_field_of_view), m_aspect_ratio, m_near_z, m_far_z);
	}

	Mat4 CameraComponent::GetOrthoMatrix() const
	{
		return  mat4::Ortho(m_viewport.left, m_viewport.left + m_viewport.width,
			m_viewport.top + m_viewport.height, m_viewport.top, m_near_z, m_far_z);
	}

}
