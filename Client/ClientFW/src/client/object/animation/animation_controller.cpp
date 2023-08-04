#include "stdafx.h"
#include "client/asset/bone/skeleton.h"
#include "client/asset/core/asset_manager.h"
#include "client/asset/core/asset_store.h"
#include "client/asset/mesh/mesh.h"
#include "client/renderer/core/render.h"
#include "client/object/component/mesh/skeletal_mesh_component.h"
#include "client/object/animation/animation_controller.h"

namespace client_fw 
{
    AnimationController::AnimationController()
    {
    }

    bool AnimationController::Initialize()
    {
        m_time_pos = 0;
        m_curr_frame = 0;
        if(m_anim_seq) m_anim_seq->GetDefaultTime(m_start_time, m_end_time);
        m_ready_animation_name.clear();
        m_ready_anim_seq = nullptr;
        m_is_end_animation = false;
        return true;
    }

    void AnimationController::Update(float delta_time)
    {
        AnimToPlay(delta_time);
        CopyBoneTransformData();
    }

    void AnimationController::AnimToPlay(float delta_time)
    {
        if (m_looping == false && m_is_end_animation == true)
            return;

        if(m_ready_anim_seq != nullptr)
        {
            m_anim_seq = m_ready_anim_seq;
            m_animation_name = m_ready_animation_name;
            Initialize();
        }

        if (m_anim_seq) {

            m_is_update_animation = true;

            float time_pos = m_time_pos;
            int prev_frame = m_curr_frame;

            time_pos += delta_time * m_animation_speed;

            m_anim_seq->AnimToPlay(m_curr_frame, time_pos);

            auto it = m_notify_map.find(m_animation_name);
            if (it != m_notify_map.end()) {
                for (int i = prev_frame; i <= m_curr_frame; ++i)
                {
                    if (i == it->second.frame_index)
                    {
                        it->second.notify_function();
                    }
                }
            }
            

            if (time_pos >= m_end_time)
			{
				time_pos = m_start_time;
                m_curr_frame = 0;
                m_is_end_animation = true;
			}
			m_time_pos = time_pos;

            m_is_update_animation = false;
        }
        
    }

    void AnimationController::SetAnimation(bool looping)
    {
        m_looping = looping;

        const auto& skeleton = m_owner.lock()->GetSkeletalMesh()->GetSkeleton();

        if (m_is_update_animation == true)
            m_ready_anim_seq = AssetStore::LoadAnimation(GetAnimationPath(m_ready_animation_name), skeleton);
        else
        {
            m_anim_seq = AssetStore::LoadAnimation(GetAnimationPath(m_animation_name), skeleton);
            Initialize();
        }
    }

    void AnimationController::SetAnimationName(const std::string& animation_name)
    {
        if (m_is_update_animation == true)
            m_ready_animation_name = animation_name;
        else
            m_animation_name = animation_name;
    }
   
    void AnimationController::SetBoneData()
    {
        auto& skeleton = m_owner.lock()->GetSkeletalMesh()->GetSkeleton();
        auto& bone_data = m_owner.lock()->GetSkeletalMesh()->GetBoneData();

        if (m_cahce_skeleton.empty())
        {
            UINT index = 0;
            for (auto& name : bone_data->bone_names)//인덱스형 루프로 바꾸자
            {
                auto cache_skeleton = skeleton->FindBone(name);
                m_cahce_skeleton.emplace_back(cache_skeleton);
                m_bone_socket_info.insert({ cache_skeleton->GetBoneName(),index });
                index++;
            }
        }
        m_bone_transform_resource.resize(m_cahce_skeleton.size());
        m_bone_offset = bone_data->bone_offsets;
    }

    void AnimationController::AddNotify(const std::string name, const std::string animation_name, int frame_index, const std::function<void()>& function)
    {
        if (m_notify_map.find(name) == m_notify_map.cend())
        {
            m_notify_map.insert({ name,{frame_index,animation_name,function} });
        }
        else
            LOG_WARN("{0} is already existed Notify Name!");
    }

    const Mat4& AnimationController::FindTransformToSocketName(const std::string& socket_name)
    {
        if (m_cahce_skeleton.empty() == false)
        {
            if (m_bone_socket_info.find(socket_name) != m_bone_socket_info.cend())
            {
                auto index = m_bone_socket_info.at(socket_name);
                    return m_bone_transform_resource[index];
            }
            LOG_WARN("Does not exist socket name {0}", socket_name);
        }
        
        return mat4::IDENTITY;
    }

    void AnimationController::CopyBoneTransformData()
    {
        for (UINT index = 0; index < m_cahce_skeleton.size(); ++index)
        {
            Mat4 final_transform =  m_bone_offset[index] * m_cahce_skeleton[index].lock()->GetBoneTransform();
            final_transform.Transpose();
            m_bone_transform_resource[index] = final_transform;
        }
    }
    const std::string AnimationController::GetAnimationPath(const std::string& animation_name)
    {
        std::string mesh_path = m_owner.lock()->GetMesh()->GetPath();
        std::string parent_path = file_help::GetParentPathFromPath(mesh_path);
        std::string stem = file_help::GetStemFromPath(mesh_path);
        std::string animation_path = parent_path + "/" + stem + "_" + animation_name + ".rev";
        return animation_path;
    }
}
