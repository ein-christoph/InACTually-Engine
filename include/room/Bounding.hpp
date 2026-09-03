
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "stddef.hpp"
#include "cinder/gl/gl.h"


namespace act {
	namespace room {
		
		class BoundingBase {
		public:
			BoundingBase(glm::vec3 position, glm::quat orientation) {
				setPosition(position);
				setOrientation(orientation);
			}
			~BoundingBase() {}

			glm::vec3		getPosition()							{ return m_position; }
			virtual void	setPosition(glm::vec3 position)			{ m_position = position; updateTransform(); };
			glm::quat		getOrientation()						{ return m_orientation; }
			virtual void	setOrientation(glm::quat orientation)	{ m_orientation = orientation; updateTransform(); };

			virtual bool	contains(glm::vec3 pt) = 0;
			virtual bool	intersects(ci::Ray ray) = 0;
			virtual void	draw() = 0;

		protected:
			glm::vec3		m_position;
			glm::quat		m_orientation;
			glm::vec3		m_scalation;

			ci::mat4		m_transform;
			void updateTransform() {
				m_transform = ci::translate(m_position) * glm::toMat4(m_orientation) * ci::scale(glm::vec3(1.0f));
			}
		}; using BoundingRef = std::shared_ptr<BoundingBase>;

		class BoundingSphere : public BoundingBase {
		public:
			BoundingSphere(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float radius = 1.0f)
			: BoundingBase(position, glm::quat(1.0f, 0.0f, 0.0f, 1.0f)) {
				setRadius(radius);
			};

			static std::shared_ptr<BoundingSphere> create(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float radius = 1.0f) { return std::make_shared<BoundingSphere>(position, radius); };

			float	getRadius() { return m_radius; }
			void	setRadius(float radius) { m_radius = radius; m_radiusSq	= radius + radius; }

			bool	contains(glm::vec3 pt) override {
						return glm::length(m_position - pt) <= m_radius;
					}
			bool	intersects(ci::Ray ray) override {
						glm::vec3 m = ray.getOrigin() - m_position;
						float b = glm::dot(m, ray.getDirection());
						float c = glm::dot(m, m) - m_radiusSq;

						// origin outside and r pointing away from s (b > 0) 
						if (c > 0.0f && b > 0.0f) 
							return false;
						float discr = b * b - c;
						
						// negative discriminant -> ray missing sphere 
						if (discr < 0.0f) 
							return false;

						//float t = 0.0f;
						// compute smallest t value of intersection
						// t = -b - sqrt(discr);
						// if t is negative, ray started inside sphere so glm::clamp t to zero 
						//if (t < 0.0f) t = 0.0f;
						// glm::vec3 intersection = ray.calcPosition(t);
						return true;
					};
			void	draw() override {
						ci::gl::drawSphere(m_position, m_radius);
					};
		private:
			float	m_radius;
			float	m_radiusSq;
		};

		class BoundingCylinder : public BoundingBase {
		public:
			BoundingCylinder(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float radius = 1.0f)
				: BoundingBase(position, glm::quat(1.0f, 0.0f, 0.0f, 1.0f)) {
				setRadius(radius);
				m_triMesh = ci::TriMesh::create(ci::geom::Cylinder());
			};

			static std::shared_ptr<BoundingCylinder> create(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float radius = 1.0f) { return std::make_shared<BoundingCylinder>(position, radius); };

			float	getRadius() { return m_radius; }
			void	setRadius(float radius) { m_radius = radius; m_radiusSq = radius + radius; }

			bool	contains(glm::vec3 pt)	override {
						return false;
					}
			bool	intersects(ci::Ray ray) override {
						return false;
					};
			void	draw() override {
						ci::gl::pushMatrices();
						ci::gl::translate(m_position);
						ci::gl::multModelMatrix(glm::toMat4(m_orientation));
						//ci::gl::scale(m_scalation);
						ci::gl::draw(*m_triMesh);
						ci::gl::popMatrices();
					};
		private:
			float	m_radius;
			float	m_radiusSq;
			ci::TriMeshRef		m_triMesh;
		};

		class BoundingMesh : public BoundingBase {
		public:
			BoundingMesh(ci::TriMeshRef triMesh = ci::TriMesh::create(ci::geom::Cube()), glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
				: BoundingBase(position, glm::quat(1.0f, 0.0f, 0.0f, 1.0f)) {
				setTriMesh(triMesh);
				updateTransform();
			};

			static std::shared_ptr<BoundingMesh> create(ci::TriMeshRef triMesh = ci::TriMesh::create(ci::geom::Cube()), glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f)) { return std::make_shared<BoundingMesh>(triMesh, position); };

			ci::TriMeshRef	getTriMesh() { return m_triMesh; }
			void			setTriMesh(ci::TriMeshRef triMesh) { 
								m_triMesh = triMesh; 
								m_bounds = m_triMesh->calcBoundingBox();
							}

			bool	contains(glm::vec3 pt)		override {
						ci::AxisAlignedBox worldBoundsApprox = m_bounds.transformed(m_transform); // fast
						if (!worldBoundsApprox.contains(pt))
							return false;
						glm::vec3 point;
						if (intersection(ci::Ray(m_position, pt - m_position), pt)) {
							if (glm::length(point - m_position) > glm::length(pt - m_position))
								return true;
						}
						return false;
					}
			bool	intersects(ci::Ray ray)		override {
						ci::AxisAlignedBox worldBoundsApprox = m_bounds.transformed(m_transform); // fast

						if (!worldBoundsApprox.intersects(ray))
							return false;

						glm::vec3 pt;
						// Did we have a hit?
						if (intersection(ray, pt)) {
							// Calculate the exact position of the hit.
							//*pickedPoint = ray.calcPosition(result);

							return true;
						}
						else
							return false;
					};

			bool	intersection(ci::Ray ray, glm::vec3 &point) {
						// Set initial distance to something far, far away.
						float result = FLT_MAX;

						// Traverse triangle list and find the closest intersecting triangle.
						const size_t polycount = m_triMesh->getNumTriangles();

						float distance = 0.0f;
						for (size_t i = 0; i < polycount; ++i) {
							// Get a single triangle from the mesh.
							glm::vec3 v0, v1, v2;
							m_triMesh->getTriangleVertices(i, &v0, &v1, &v2);

							// Transform triangle to world space.
							v0 = glm::vec3(m_transform * ci::vec4(v0, 1.0));
							v1 = glm::vec3(m_transform * ci::vec4(v1, 1.0));
							v2 = glm::vec3(m_transform * ci::vec4(v2, 1.0));

							// Test to see if the ray intersects this triangle.
							if (ray.calcTriangleIntersection(v0, v1, v2, &distance)) {
								// Keep the result if it's closer than any intersection we've had so far.
								if (distance < result) {
									result = distance;

									// Assuming this is the closest triangle, we'll calculate our normal
									// while we've got all the points handy.
									// *pickedNormal = normalize(cross(v1 - v0, v2 - v0));
								}
							}
						}

						if (distance > 0) {
							// Calculate the exact position of the hit.
							point = ray.calcPosition(result);

							return true;
						}
						else
							return false;
					}

			void	draw() override {
						ci::gl::pushMatrices();
						ci::gl::multModelMatrix(m_transform);
						ci::gl::draw(*m_triMesh);
						ci::gl::popMatrices();
					};
		private:
			ci::TriMeshRef		m_triMesh;
			ci::AxisAlignedBox	m_bounds;
		};
	}
}