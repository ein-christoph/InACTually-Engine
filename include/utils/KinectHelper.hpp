
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
	Fabian Töpfer
*/

#pragma once

#include <string>

#include "k4a2mat.hpp"

#include <k4abt.hpp>
#include "body/Body.hpp"


namespace kh
{
	static cv::Mat k4a_get_mat(k4a::image src, bool deepCopy = true)
	{
		return k4a::get_mat(src, deepCopy);
	}


	static glm::quat getKinectTPoseOrientation(act::room::BodyJointRef joint)
	{
		glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
			
		switch (joint->type)
		{
		case act::room::BJT_PELVIS:
		case act::room::BJT_SPINE_CHEST:
		case act::room::BJT_NECK:
		case act::room::BJT_HEAD:
		case act::room::BJT_KNEE_LEFT:
		case act::room::BJT_FOOT_LEFT:
			return (glm::angleAxis(glm::radians(90.0f), forward) * glm::angleAxis(glm::radians(-90.0f), right));

		case act::room::BJT_KNEE_RIGHT:
		case act::room::BJT_FOOT_RIGHT:
			return glm::angleAxis(glm::radians(-90.0f), forward) * glm::angleAxis(glm::radians(90.0f), right);
			
		case act::room::BJT_SHOULDER_LEFT:
		case act::room::BJT_ELBOW_LEFT:
			return glm::angleAxis(glm::radians(180.0f), up) * glm::angleAxis(glm::radians(90.0f), right);

		case act::room::BJT_HAND_LEFT:
			return glm::angleAxis(glm::radians(180.0f), up);

		case act::room::BJT_SHOULDER_RIGHT:
		case act::room::BJT_ELBOW_RIGHT:
			return  glm::angleAxis(glm::radians(180.0f), up) *  glm::angleAxis(glm::radians(-90.0f), right);

		case act::room::BJT_HAND_RIGHT:
			return glm::angleAxis(glm::radians(180.0f), up) * glm::angleAxis(glm::radians(180.0f), right);

		default:
			return glm::quat();
		}
	}

	// see https://github.com/bibigone/k4a.net/blob/master/K4AdotNet.Samples.Unity/Assets/Scripts/CharacterAnimator.cs

	// Used this page as reference for T-pose orientations
	// https://docs.microsoft.com/en-us/azure/Kinect-dk/body-joints
	// Assuming T-pose as body facing Z+, with head at Y+. Same for target character
	// Coordinate system seems to be left-handed not right handed as depicted
	// Thus inverse T-pose rotation should align Y and Z axes of depicted local coords for a joint with body coords in T-pose

	static glm::quat getKinectTPoseOrientationInverse(act::room::BodyJointRef joint)
	{
		return glm::inverse(getKinectTPoseOrientation(joint));
	}
  

	static act::room::BodyRefList setOrientationFromBodyPose(act::room::BodyRefList bodies) {
		for (auto&& body : bodies) {
			for (int i = 0; i < (int)act::room::BodyJointType::BJT_COUNT; i++) {
				act::room::BodyJointRef currJoint = body->joints[i];


				if (auto info = act::room::bodyMap.find(currJoint->type); info != act::room::bodyMap.end()) {

					act::room::BodyJointRef nextJoint = body->joints[info->second->nextJoint];

					glm::vec3 dir = nextJoint->position - currJoint->position;
					glm::vec3 uDir = glm::normalize(dir);
 
					glm::quat r = glm::quatLookAt(uDir, glm::vec3(0.0f, 1.0f, 0.0f));
 				
					currJoint->orientation = r;
					//CI_LOG_D("New Joint Orientation calculated");

				}
				else {
					//CI_LOG_E("Body Map Joint not defined");
					currJoint->orientation = getKinectTPoseOrientationInverse(currJoint) * getKinectTPoseOrientation(currJoint);
				}
			}


		}

		return bodies;
	}

	static act::room::BodyRefList toGenericBody(std::map<uint32_t, k4abt_skeleton_t> kinectbodies)
	{
		act::room::BodyRefList bodies;

		for (auto&& kinectBody : kinectbodies) {
			k4abt_skeleton_t kinectSkeleton = kinectBody.second;
			act::room::BodyRef body = act::room::Body::create();

			for (int i = 0; i < (int)act::room::BodyJointType::BJT_COUNT; i++) {
				act::room::BodyJointRef currJoint = body->joints[i];
				k4abt_joint_t k4aJoint;

				switch (currJoint->type)
				{
				case act::room::BJT_PELVIS:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_PELVIS];
					break;
				case act::room::BJT_SPINE_CHEST:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_SPINE_CHEST];
					break;
				case act::room::BJT_NECK:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_NECK];
					break;
				case act::room::BJT_SHOULDER_LEFT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_SHOULDER_LEFT];
					break;
				case act::room::BJT_ELBOW_LEFT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_ELBOW_LEFT];
					break;
				case act::room::BJT_HAND_LEFT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_HAND_LEFT];
					break;
				case act::room::BJT_SHOULDER_RIGHT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_SHOULDER_RIGHT];
					break;
				case act::room::BJT_ELBOW_RIGHT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_ELBOW_RIGHT];
					break;
				case act::room::BJT_HAND_RIGHT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_HAND_RIGHT];
					break;
				case act::room::BJT_KNEE_LEFT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_KNEE_LEFT];
					break;
				case act::room::BJT_FOOT_LEFT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_FOOT_LEFT];
					break;
				case act::room::BJT_KNEE_RIGHT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_KNEE_RIGHT];
					break;
				case act::room::BJT_FOOT_RIGHT:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_FOOT_RIGHT];
					break;
				case act::room::BJT_HEAD:
					k4aJoint = kinectSkeleton.joints[K4ABT_JOINT_HEAD];
					break;
				default:
					break;
				}

				k4a_float3_t::_xyz pos = k4aJoint.position.xyz;
				k4a_quaternion_t::_wxyz orientation = k4aJoint.orientation.wxyz;
				int confidenceLevel = static_cast<std::underlying_type<k4abt_joint_confidence_level_t>::type>(k4aJoint.confidence_level);

				currJoint->position = glm::vec3(pos.x, pos.y, pos.z);
				currJoint->orientation = getKinectTPoseOrientationInverse(currJoint) * glm::quat(orientation.w, orientation.x, orientation.y, orientation.z);
				currJoint->confidenceLevel = static_cast<act::room::BodyJointConfidence>(confidenceLevel);

			

			}
			bodies.push_back(body);
		}


		return setOrientationFromBodyPose(bodies);
	}

	
}