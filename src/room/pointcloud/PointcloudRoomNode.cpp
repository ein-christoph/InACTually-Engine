
/*
    InACTually
    > interactive theater for actual acts
    > this file is part of the "InACTually Engine", a MediaServer for driving all technology

    Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
    Copyright (c) 2025 InACTually Community
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    This file is created and substantially modified: 2023-2024

    contributors:
    Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "pointcloud/PointcloudRoomNode.hpp"


act::room::PointcloudRoomNode::PointcloudRoomNode(glm::vec3 position, float radius, std::string name, act::UID replyUID)
	: RoomNodeBase("pointcloud", position, glm::vec3(0.0f), radius, replyUID)
{
	//setTriMesh(ci::TriMesh::create(ci::geom::Sphere()));

    m_numPoints = 0;
    // m_pointcloud = ;


    auto vs = CI_GLSL(150,

        uniform mat4 ciModelViewProjection;
        in vec4      ciPosition;
        in glm::vec3      ciColor;
        in float     vSize;

        out glm::vec3     Color;

        void main()
        {
            gl_PointSize = vSize;
            gl_Position = ciModelViewProjection * ciPosition;

            ci::Color = ciColor;
        }

    );
    auto fs = CI_GLSL(150,

        uniform sampler2D uTexture;
        in glm::vec3           Color;
        out vec4          FinalColor;

        void main()
        {
            vec4 color = vec4(ci::Color, 1);
           // color *= texture(uTexture, gl_PointCoord);

            if (color.a < 0.01f) discard;

            Finalci::Color = color;
        }

    );

    //m_shader = ci::gl::GlslProg::create(vs, fs);
    //m_shader->uniform("uTexture", 0);
}

act::room::PointcloudRoomNode::~PointcloudRoomNode()
{
}

void act::room::PointcloudRoomNode::setup()
{
}

void act::room::PointcloudRoomNode::update()
{
	
}

void act::room::PointcloudRoomNode::draw()
{


	ci::gl::ScopedColor color(ci::Color(0.6f, 0.6f, 0.6f));

	//enableStatusColor();	

	// Draw the mesh.
	{
		//ci::gl::ScopedColor color(ci::Color::white());

		ci::gl::ScopedModelMatrix model;
		ci::gl::multModelMatrix(m_transform);

		//m_mesh->draw();

        if(m_geometry)
            m_geometry->draw();

		/*for (int i = 0; i < m_pointcloud->size(); i += 10) { // skipping a lot of points for debug/performance reasons
			auto pt = m_pointcloud->at(i);
			ci::gl::drawCube(glm::vec3(pt.x, pt.y, pt.z), glm::vec3(0.05f));
			glVertex3f(pt.x, pt.y, pt.z);
		}*/

	}
	
}

void act::room::PointcloudRoomNode::setPointcloud(act::room::Pointcloud pointcloud)
{
    if (!pointcloud || pointcloud->size() == 0)
        return;

    /*
    float size = 0.01f;
    pcl::VoxelGrid<pcl::PointXYZRGB> grid;
    grid.setInputCloud(pointcloud);
    grid.setLeafSize(size, size, size);
    grid.filter(*m_pointcloud);
    */

    m_pointcloud = pointcloud;

    if (m_numPoints != m_pointcloud->size())
        createPointCloud();
    else
        updatePointCloud();
}

void act::room::PointcloudRoomNode::createPointCloud()
{
    m_numPoints = m_pointcloud->size();
    m_positions.resize(m_numPoints);
    m_colors.resize(m_numPoints);
    m_sizes.resize(m_numPoints);

    for (int i = 0; i < m_numPoints; i++)
    {
        auto&& pt = m_pointcloud->at(i);
        m_positions[i] = glm::vec3(pt.x, pt.y, pt.z);

        m_sizes[i] = 0.5f; //pt.z * 0.5f;
        m_colors[i] = ci::Colorf(pt.r, pt.g, pt.b);
    }

    auto posLayout = ci::gl::VboMesh::Layout().attrib(ci::geom::POSITION, 3).usage(GL_DYNAMIC_DRAW); // Because these update every frame
    auto colorAndSizeLayout = ci::gl::VboMesh::Layout().attrib(ci::geom::COLOR, 3).attrib(ci::geom::CUSTOM_0, 1).usage(GL_DYNAMIC_DRAW); // GL_STATIC_DRAW if dont

    auto mesh = ci::gl::VboMesh::create(m_numPoints, GL_POINTS, { posLayout, colorAndSizeLayout });
    mesh->bufferAttrib(ci::geom::POSITION, m_positions);
    mesh->bufferAttrib(ci::geom::COLOR, m_colors);
    mesh->bufferAttrib(ci::geom::CUSTOM_0, m_sizes);

    m_geometry = ci::gl::Batch::create(mesh, m_shader, { { ci::geom::Attrib::CUSTOM_0, "vSize" } });
}

void act::room::PointcloudRoomNode::updatePointCloud()
{
    for (int i = 0; i < m_numPoints; i++)
    {
        auto&& pt = m_pointcloud->at(i);
        m_positions[i] = glm::vec3(pt.x, pt.y, pt.z);

        //m_sizes[i] = pt.z * 0.5f;
        m_colors[i] = ci::Colorf(pt.r / 255.0f, pt.g / 255.0f, pt.b / 255.0f);
    }

    m_geometry->getVboMesh()->bufferAttrib(ci::geom::POSITION, m_positions);
    m_geometry->getVboMesh()->bufferAttrib(ci::geom::COLOR, m_colors);
    //m_geometry->getVboMesh()->bufferAttrib(ci::geom::CUSTOM_0, m_sizes);
}

void act::room::PointcloudRoomNode::drawSpecificSettings() 
{
}