#include "stdafx.h"
#include "GameUI.h"
#include "Vertex.h"
#include "GenMap.h"
#include "TileMap.h"
extern int WindowWidth;
extern int WindowHeight;
//-----------------------------------------------------------------------------
static constexpr const char* vertexShader = R"(
#version 330 core

layout(location = 0) in vec2 vertexPosition;

uniform mat4 pm;

void main()
{
	gl_Position = pm * vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0); 
}
)";
//-----------------------------------------------------------------------------
static constexpr const char* fragmentShader = R"(
#version 330 core

out vec4 fragColor;

uniform vec3 color;

void main()
{
	fragColor = vec4(color.x, color.y, color.z, 1.0);
}
)";

bool GameUI::Init()
{
	m_shaderProgramQuad.CreateFromMemories(vertexShader, fragmentShader);
	m_shaderProgramQuad.Bind();
	m_matrixID = m_shaderProgramQuad.GetUniformVariable("pm");
	m_colorID = m_shaderProgramQuad.GetUniformVariable("color");


	// Quad vertices
	static constexpr Vertex_Pos2 quadVertices[] = {
		{{-0.5f,  0.5f}},
		{{ 0.5f,  0.5f}},
		{{ 0.5f, -0.5f}},

		{{ 0.5f, -0.5f}},
		{{-0.5f, -0.5f}},
		{{-0.5f,  0.5f}},
	};

	m_vertexBufQuad.Create(RenderResourceUsage::Static, 6, sizeof(Vertex_Pos2), quadVertices);
	std::vector<VertexAttribute> attribs =
	{
		{.size = 2, .type = GL_FLOAT, .normalized = false, .stride = sizeof(Vertex_Pos2), .pointer = (void*)offsetof(Vertex_Pos2, position)}
	};
	m_vaoQuad.Create({ &m_vertexBufQuad }, nullptr, attribs);


	m_ortho = glm::ortho(0.0f, 320.0f, 240.0f, 0.0f, -10.0f, 10.0f);
	return true;
}

void GameUI::Close()
{
	m_vaoQuad.Destroy();
	m_vertexBufQuad.Destroy();
	m_shaderProgramQuad.Destroy();
}

void GameUI::Draw(const glm::vec3& newPlayerPos, const TileMap& map)
{
	if (m_windowWidth != WindowWidth || m_windowHeight != WindowHeight)
	{
		m_windowWidth = WindowWidth;
		m_windowHeight = WindowHeight;

		m_uiHeight = 240.0f;
		m_uiWidth = m_uiHeight * ((float)m_windowWidth/(float)m_windowHeight);
		

		m_ortho = glm::ortho(0.0f, m_uiWidth, m_uiHeight, 0.0f, -10.0f, 10.0f);
	}

	glDisable(GL_DEPTH_TEST);

	m_shaderProgramQuad.Bind();

	glm::vec3 pos = glm::vec3{ 0 };

	static constexpr glm::vec3 colorWall = glm::vec3(0.8f, 0.6f, 0.6f);
	static constexpr glm::vec3 colorFloor = glm::vec3(1.0f, 0.8f, 0.8f);
	static constexpr glm::vec3 colorPlayer = glm::vec3(0.2f, 0.4f, 0.8f);

	for (int x = 0; x < SizeMap; x++)
	{
		for (int y = 0; y < SizeMap; y++)
		{
#if !FullVisibleMap
			if (!map.GetTileVisible(x, y)) continue;
#endif
			bool isVisible = false;

			pos.x = m_uiWidth - SizeMap + x;
			pos.y = y;

			switch (map.GetMapTileData().getTile(x, y))
			{
			case GenTile::Floor:
			case GenTile::Corridor:
			case GenTile::ClosedDoor:
			case GenTile::OpenDoor:
			case GenTile::UpStairs:
			case GenTile::DownStairs:
				isVisible = true;
				m_shaderProgramQuad.SetUniform(m_colorID, colorFloor);
				break;

			case GenTile::Wall:
				isVisible = true;
				m_shaderProgramQuad.SetUniform(m_colorID, colorWall);
				break;
			default:
				break;
			}

			if (isVisible)
			{
				glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
				glm::mat4 pm = m_ortho * translate;
				m_shaderProgramQuad.SetUniform(m_matrixID, pm);
				m_vaoQuad.Draw();
			}
		}
	}

	pos.x = m_uiWidth - SizeMap + newPlayerPos.x;
	pos.y = newPlayerPos.z;
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
	glm::mat4 pm = m_ortho * translate;
	m_shaderProgramQuad.SetUniform(m_matrixID, pm);
	m_shaderProgramQuad.SetUniform(m_colorID, colorPlayer);
	m_vaoQuad.Draw();


	glEnable(GL_DEPTH_TEST);
}