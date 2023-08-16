#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/opencv.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "shader.h"
#include "camera.h"
using namespace std;

struct Vertex {
	int index;
	glm::vec3 Pos;
	glm::vec3 Pre;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Vlc = glm::vec3(0.0f);
	glm::vec3 force = glm::vec3(0);
	bool isAnchored = false;
};

struct Edge {
	int startVertex, endVertex;
	float OriLength;
	float NowLength;
	bool operator==(const Edge& other) const {
		return (startVertex == other.startVertex && endVertex == other.endVertex) ||
			(startVertex == other.endVertex && endVertex == other.startVertex);
	}
	bool operator<(const Edge& other) const {
		return (startVertex < other.startVertex) ||
			(startVertex == other.startVertex && endVertex < other.endVertex);
	}
};

struct Mesh {
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Edge> edges;
};

//settings
const unsigned int SCR_WIDTH = 1260;
const unsigned int SCR_HEIGHT = 960;
string inputPath;
string outputPath = "asserts/output/";
float del = 0.001;
float jiao = 0.0f;
float gravity = 0.98f;

//camera
Camera kam(glm::vec3(0.0f, 2.5f, 13.0f));
//functions
// render
// Initializes GLFW.
void initializeGLFW();

// Creates a window and returns its pointer.
GLFWwindow* createWindow();

// Initializes GLAD.
void initializeGLAD();

// Sets up the viewport.
void setupViewport();

// Sets up callback functions.
void setupCallbacks(GLFWwindow* window);

// Initializes ImGui.
void initializeImGui(GLFWwindow* window);

// The main rendering loop.
void renderLoop(GLFWwindow* window);
//set force
void GiveaG(Mesh &mesh, float gravity);
void GiveaSpringforce(Mesh &mesh, float gravity);
void Giveawind(Mesh &mesh, float windPower);
void ClearForces(Mesh& mesh);
void UpdateVelocityAndPosition(Mesh& mesh, float del);
//update mesh
void TerrainRefresh(Mesh &mesh, float del);
void TerrainWithBallRefresh(Mesh &mesh, float del);
void RotationBallScene(Mesh &mesh, float del);
void RefreshGiveaGandSpringforce(Mesh &mesh, float del);
void RefreshAllForce(Mesh &mesh, float del);


Mesh MeshofTerrain(float width, float height);
void WriteVertices(ofstream& outFile, const vector<Vertex>& vertices);
void WriteTextureCoords(ofstream& outFile, const vector<Vertex>& vertices);
void WriteNormals(ofstream& outFile, const vector<Vertex>& vertices);
void WriteFaces(ofstream& outFile, const vector<unsigned int>& indices);
void WriteAnewOBJ(Mesh mesh, string name);
void ShowMyMesh(Mesh mesh);
Mesh CreateMyBall(float radius, unsigned int rings, unsigned int sectors, glm::vec3 position);
Mesh CreateMyCloth(float width, float height, unsigned int complexity, glm::vec3 center);
void rotateMyBall(Mesh& ballMesh, float angle);

//callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

//simu para
bool BallWithTerrain = false;
bool Terrain = false;
bool RotateTheScene = false;
bool GandSpringforce = false;
bool AllForce = false;
bool liitleone = false;
bool BIgone = false;
bool updateWithBall=false;
bool updateWithCloth=false;

int  main() {
initializeGLFW();
GLFWwindow* window = createWindow();
initializeGLAD();
setupViewport();
setupCallbacks(window);
glEnable(GL_DEPTH_TEST);
initializeImGui(window);
renderLoop(window);
glfwTerminate();
return 0;
}

Mesh MeshofTerrain(float width, float height) {
	Mesh terrainMesh;

	// Define the base vertices, normals, and texture coordinates
	glm::vec3 baseVertices[] = {
		{-width / 2.0f, 0.0f, height / 2.0f},
		{width / 2.0f, 0.0f, height / 2.0f},
		{-width / 2.0f, 0.0f, -height / 2.0f},
		{width / 2.0f, 0.0f, -height / 2.0f},
		{0.0f, 0.0f, 0.0f} // Center vertex
	};

	glm::vec3 normal(0.0f, 1.0f, 0.0f);

	glm::vec2 texCoords[] = {
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{0.5f, 0.5f} // Center vertex texCoord
	};

	// Add the vertices to the mesh using a loop
	for (int i = 0; i < 5; ++i) { // Notice the loop now goes up to 5
		terrainMesh.vertices.push_back({ i, baseVertices[i], baseVertices[i], normal, texCoords[i] });
	}

	// Indices for the rectangle (now six triangles)
	terrainMesh.indices = {
		0, 2, 4, // Triangle touching center and bottom-left
		2, 3, 4, // Triangle touching center and bottom-right
		3, 1, 4, // Triangle touching center and top-right
		1, 0, 4, // Triangle touching center and top-left
		0, 1, 2, // Original bottom-left triangle
		1, 3, 2  // Original bottom-right triangle
	};

	// Edges (optional, but you may need them for other purposes)
	terrainMesh.edges = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0},
		{0, 4}, {1, 4}, {2, 4}, {3, 4} // Edges connecting to the center vertex
	};

	return terrainMesh;
}
void WriteVertices(ofstream& outFile, const vector<Vertex>& vertices) {
	for (const Vertex& vertex : vertices) {
		outFile << "v " << vertex.Pos.x << " " << vertex.Pos.y << " " << vertex.Pos.z << endl;
	}
}

void WriteTextureCoords(ofstream& outFile, const vector<Vertex>& vertices) {
	for (const Vertex& vertex : vertices) {
		outFile << "vt " << vertex.TexCoords.x << " " << vertex.TexCoords.y << endl;
	}
}

void WriteNormals(ofstream& outFile, const vector<Vertex>& vertices) {
	for (const Vertex& vertex : vertices) {
		outFile << "vn " << vertex.Normal.x << " " << vertex.Normal.y << " " << vertex.Normal.z << endl;
	}
}

void WriteFaces(ofstream& outFile, const vector<unsigned int>& indices) {
	for (size_t i = 0; i < indices.size(); i += 3) {
		// OBJ format uses 1-based indexing
		unsigned int index1 = indices[i] + 1;
		unsigned int index2 = indices[i + 1] + 1;
		unsigned int index3 = indices[i + 2] + 1;
		outFile << "f "
			<< index1 << "/" << index1 << "/" << index1 << " "
			<< index2 << "/" << index2 << "/" << index2 << " "
			<< index3 << "/" << index3 << "/" << index3 << endl;
	}
}

void WriteAnewOBJ(Mesh mesh, string name) {
	string outputFilename = outputPath + name + ".obj";

	// Open the output file
	ofstream outFile(outputFilename);

	if (!outFile) {
		cerr << "Error: Failed to open output file: " << outputFilename << endl;
		return;
	}

	WriteVertices(outFile, mesh.vertices);
	WriteTextureCoords(outFile, mesh.vertices);
	WriteNormals(outFile, mesh.vertices);
	WriteFaces(outFile, mesh.indices);

	// Check for write errors
	if (!outFile) {
		cerr << "Error: Failed to write to file: " << outputFilename << endl;
	}

	// Close the file
	outFile.close();
}

void ShowMyMesh(Mesh mesh) {
	vector<float> vertices;

	// Extract vertex positions from the mesh
	for (Vertex& vertex : mesh.vertices) {
		vertices.push_back(vertex.Pos.x);
		vertices.push_back(vertex.Pos.y);
		vertices.push_back(vertex.Pos.z);
	}

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// Bind and set vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	// Bind and set index buffer data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(int), mesh.indices.data(), GL_STATIC_DRAW);

	// Define vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Draw the mesh using index data
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Cleanup
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}



Mesh CreateMyBall(float radius, unsigned int rings, unsigned int sectors, glm::vec3 position) {
	Mesh ballMesh;

	const float PI = 3.14f;

	for (unsigned int i = 0; i <= rings; ++i) {
		float theta = i * PI / rings;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);

		for (unsigned int j = 0; j <= sectors; ++j) {
			float phi = j * 2 * PI / sectors;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);

			float x = cosPhi * sinTheta;
			float y = cosTheta;
			float z = sinPhi * sinTheta;

			Vertex vertex;
			vertex.Pos = glm::vec3(x * radius, y * radius, z * radius) + position;
			vertex.Normal = glm::normalize(vertex.Pos - position);
			vertex.TexCoords = glm::vec2((float)j / sectors, 1.0f - (float)i / rings); // Inverted the V coordinate

			ballMesh.vertices.push_back(vertex);
		}
	}

	for (unsigned int i = 0; i < rings; ++i) {
		for (unsigned int j = 0; j < sectors; ++j) {
			unsigned int first = i * (sectors + 1) + j;
			unsigned int second = first + sectors + 1;

			// Modified the order of indices for a different winding order
			ballMesh.indices.push_back(first);
			ballMesh.indices.push_back(first + 1);
			ballMesh.indices.push_back(second);

			ballMesh.indices.push_back(second);
			ballMesh.indices.push_back(first + 1);
			ballMesh.indices.push_back(second + 1);
		}
	}

	return ballMesh;
}

Mesh CreateMyCloth(float width, float height, unsigned int complexity, glm::vec3 center) {
	Mesh clothMesh;

	// Vertex generation
	for (unsigned int i = 0; i <= complexity; ++i) {
		for (unsigned int j = 0; j <= complexity; ++j) {
			Vertex vertex;
			vertex.Pos = glm::vec3(j * width / complexity - width / 2, 0, i * height / complexity - height / 2) + center;
			vertex.Pre = vertex.Pos;
			vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
			vertex.TexCoords = glm::vec2((float)j / complexity, (float)i / complexity);
			clothMesh.vertices.push_back(vertex);
		}
	}

	// Index generation
	for (unsigned int i = 0; i < complexity; ++i) {
		for (unsigned int j = 0; j < complexity; ++j) {
			unsigned int first = i * (complexity + 1) + j;
			unsigned int second = first + complexity + 1;

			clothMesh.indices.push_back(first);
			clothMesh.indices.push_back(second);
			clothMesh.indices.push_back(first + 1);

			clothMesh.indices.push_back(second);
			clothMesh.indices.push_back(second + 1);
			clothMesh.indices.push_back(first + 1);
		}
	}

	// Edge generation with deduplication
	std::set<Edge> edgeSet;
	for (int i = 0; i < clothMesh.indices.size(); i += 3) {
		std::array<int, 3> triangle = { clothMesh.indices[i], clothMesh.indices[i + 1], clothMesh.indices[i + 2] };
		std::sort(triangle.begin(), triangle.end());

		edgeSet.insert(Edge{ triangle[0], triangle[1] });
		edgeSet.insert(Edge{ triangle[1], triangle[2] });
		edgeSet.insert(Edge{ triangle[0], triangle[2] });
	}

	for (const Edge& e : edgeSet) {
		Edge edge = e;
		edge.OriLength = glm::distance(clothMesh.vertices[edge.startVertex].Pos, clothMesh.vertices[edge.endVertex].Pos);
		clothMesh.edges.push_back(edge);
	}

	return clothMesh;
}

void rotateMyBall(Mesh& ballMesh, float angle) {
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	for (Vertex& v : ballMesh.vertices) {
		v.Pos = glm::vec3(rotationMatrix * glm::vec4(v.Pos, 1.0f));
	}
}


const float MIN_Y_POS = 0.1f;

void GiveaSpringforce(Mesh &mesh, float gravity)
{
	vector<Edge>& edges = mesh.edges;
	vector<Vertex>& vertices = mesh.vertices;

	for (Edge& e : edges) {
		Vertex& startVertex = vertices[e.startVertex];
		Vertex& endVertex = vertices[e.endVertex];

		glm::vec3 dir = glm::normalize(startVertex.Pos - endVertex.Pos);
		float currentLength = glm::distance(startVertex.Pos, endVertex.Pos);
		float k = gravity / e.OriLength;
		glm::vec3 springforce = dir * (currentLength - e.OriLength) * k;

		// Check if the current length exceeds twice the original length
		if (currentLength > 2 * e.OriLength) {
			startVertex.Pos = startVertex.Pre;
			endVertex.Pos = endVertex.Pre;
			startVertex.Vlc *= 0.9f;
			endVertex.Vlc *= 0.9f;
			startVertex.force += gravity;
			endVertex.force += gravity;
			if (dir.z > 0) {
				dir.z *= -1;
			}
		}

		startVertex.force -= springforce;
		endVertex.force += springforce;
	}
}

void GiveaG(Mesh& mesh, float gravity) {
	glm::vec3 gravityForce = glm::vec3(0, -gravity, 0);
	for (auto& vertex : mesh.vertices) {
		vertex.force += gravityForce;
	}
}

void Giveawind(Mesh& mesh, float windPower) {
	glm::vec3 windForce = glm::vec3(windPower * 0.1f, 0, 0);
	for (auto& vertex : mesh.vertices) {
		vertex.force += windForce;
	}
}

void TerrainRefresh(Mesh& mesh, float del) {
	GiveaG(mesh, gravity);
	// Assuming GiveaSpringforce is another function you have
	GiveaSpringforce(mesh, gravity);

	for (auto& vertex : mesh.vertices) {
		vertex.Vlc += vertex.force * del;
		vertex.Pos += vertex.Vlc;

		if (vertex.Pos.y <= MIN_Y_POS) {
			vertex.Pos.y = MIN_Y_POS;
			vertex.Vlc.y = 0;
		}
	}
}

void TerrainWithBallRefresh(Mesh& mesh, float del) {
	const glm::vec3 ballCenter(0, 1.0f, 0);
	const float boundary = 1.2f;

	for (auto& vertex : mesh.vertices) {
		vertex.force = glm::vec3(0);
	}

	GiveaG(mesh, gravity);
	GiveaSpringforce(mesh, gravity);

	for (auto& vertex : mesh.vertices) {
		vertex.Vlc += vertex.force * del;
		vertex.Pre = vertex.Pos;
		vertex.Pos += vertex.Vlc * del;

		float toCircleCenter = glm::distance(vertex.Pos, ballCenter);

		if (toCircleCenter <= boundary) {
			glm::vec3 dir = (vertex.Pos - ballCenter) / toCircleCenter;
			vertex.Pos = dir * boundary + ballCenter;
			vertex.Vlc *= 0.95f;
		}

		if (vertex.Pos.y <= MIN_Y_POS) {
			vertex.Pos.y = MIN_Y_POS;
			vertex.Vlc *= 0.95f;
		}
	}
}
void ClearForces(Mesh &mesh) {
    for (auto &vertex : mesh.vertices) {
        vertex.force = glm::vec3(0);
    }
}

void UpdateVelocityAndPosition(Mesh &mesh, float del) {
    for (auto &vertex : mesh.vertices) {
        if (vertex.isAnchored) {
            continue;
        }

        vertex.Vlc += vertex.force * del;
        vertex.Pre = vertex.Pos;
        vertex.Pos += vertex.Vlc * del;
    }
}

void RotationBallScene(Mesh& mesh, float del) {
	const glm::vec3 ballCenter(0.01f, 1.0f, 0.01f);
	const float boundary = 1.15f;
	const glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);

	ClearForces(mesh);

	GiveaG(mesh, gravity);
	GiveaSpringforce(mesh, gravity);

	for (auto& vertex : mesh.vertices) {
		float toCircleCenter = glm::distance(vertex.Pos, ballCenter);

		if (toCircleCenter <= boundary) {
			glm::vec3 dir = (vertex.Pos - ballCenter) / toCircleCenter;
			vertex.Pos = dir * boundary + ballCenter;
			vertex.Vlc *= 0.95f;

			glm::vec3 tangentDir = glm::cross(rotationAxis, vertex.Pos - ballCenter);
			float tangentDirLength = glm::length(tangentDir);

			if (tangentDirLength > 1e-6) {
				tangentDir /= tangentDirLength;
				float cos = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), dir);
				glm::vec3 frictionForce = 300.0f * 0.98f * cos * tangentDir;
				vertex.Vlc -= frictionForce * del;
			}
		}

		if (vertex.Pos.y <= MIN_Y_POS) {
			vertex.Pos.y = MIN_Y_POS;
			vertex.Vlc *= 0.95f;
		}
	}

	UpdateVelocityAndPosition(mesh, del);
}

void RefreshGiveaGandSpringforce(Mesh& mesh, float del) {
	ClearForces(mesh);
	GiveaG(mesh, gravity);
	GiveaSpringforce(mesh, gravity);
	UpdateVelocityAndPosition(mesh, del);
}

void RefreshAllForce(Mesh& mesh, float del) {
	const float wen = 15.0f;

	ClearForces(mesh);
	GiveaG(mesh, gravity);
	GiveaSpringforce(mesh, gravity);
	Giveawind(mesh, wen);
	UpdateVelocityAndPosition(mesh, del);
}







void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


void initializeGLFW() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* createWindow() {
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ClothSimulation", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);
	return window;
}

void initializeGLAD() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}
}

void setupViewport() {
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void setupCallbacks(GLFWwindow* window) {
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

void initializeImGui(GLFWwindow* window) {
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
}

void renderLoop(GLFWwindow* window) {
	Shader empty("asserts/shaders/empty.vs", "asserts/shaders/empty.fs");
	Mesh terrain = MeshofTerrain(10, 20);
	Mesh ball = CreateMyBall(1.0f, 10, 10, glm::vec3(0.0f, 1.0f, 0.0f));
	Mesh cloth = CreateMyCloth(2, 2, 10, glm::vec3(0.0f, 5.0f, 0.0f));
	Mesh clothIni = cloth;
	Mesh clothIniBigger = CreateMyCloth(5, 5, 10, glm::vec3(0.0f, 5.0f, 0.0f));

	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.0f, 0.8f, 0.86f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		empty.use();

		glm::mat4 projection = glm::perspective(glm::radians(kam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		empty.setMat4("projection", projection);
		glm::mat4 view = kam.GetViewMatrix();
		empty.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		empty.setMat4("model", model);
		float buttonWidth = 200;
		float buttonHeight = 40.0f; // 新增的按钮高度

		// 面板1
		ImGui::SetNextWindowSize(ImVec2(buttonWidth + 20, 0.0f));
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		if (ImGui::Begin("SCENE 1")) {
			if (ImGui::Button("Terrain ", ImVec2(buttonWidth, buttonHeight))) {
				BallWithTerrain = false;
				Terrain = true;
				RotateTheScene = false;
				GandSpringforce = false;
				AllForce = false;
			}

			if (ImGui::Button("SS1", ImVec2(buttonWidth, buttonHeight))) {
				BallWithTerrain = true;
				Terrain = false;
				RotateTheScene = false;
				GandSpringforce = false;
				AllForce = false;
			}
			if (ImGui::Button("rotate the ball", ImVec2(buttonWidth, buttonHeight))) {
				BallWithTerrain = false;
				Terrain = false;
				RotateTheScene = true;
				GandSpringforce = false;
				AllForce = false;
			}
			// Button to write object
			if (ImGui::Button("Write ball Object", ImVec2(buttonWidth, buttonHeight))) {
				WriteAnewOBJ(ball, std::string("ballout"));
			}
			if (ImGui::Button("Load ball Object", ImVec2(buttonWidth, buttonHeight))) {
				updateWithBall = true;
				Terrain = false;
				RotateTheScene = false;
				GandSpringforce = false;
				AllForce = false;
			}
			if (ImGui::Button("CreateLitterCloth", ImVec2(buttonWidth, buttonHeight))) {
				liitleone = true;
			}

			if (ImGui::Button("CreateBiggerCloth", ImVec2(buttonWidth, buttonHeight))) {
				BIgone = true;
			}

			if (ImGui::Button("+Add Gravity", ImVec2(buttonWidth, buttonHeight))) {
				gravity *= 10;
			}

			if (ImGui::Button("-Minus Gravity", ImVec2(buttonWidth, buttonHeight))) {
				gravity *= 0.1f;
			}
		}
		ImGui::End();

		// 面板2
		ImGui::SetNextWindowSize(ImVec2(buttonWidth + 20, 0.0f));
		ImGui::SetNextWindowPos(ImVec2(10 + buttonWidth + 30, 10));  // 设置面板2的位置，使其在面板1的右侧

		if (ImGui::Begin("SCENE 2")) {
			{
				if (ImGui::Button("SS2", ImVec2(buttonWidth, buttonHeight))) {
					BallWithTerrain = false;
					Terrain = false;
					RotateTheScene = false;
					GandSpringforce = true;
					AllForce = false;
				}
				if (ImGui::Button("SS2 with Wind", ImVec2(buttonWidth, buttonHeight))) {
					BallWithTerrain = false;
					Terrain = false;
					RotateTheScene = false;
					GandSpringforce = false;
					AllForce = true;
				}

			}

			if (ImGui::Button("Create LitterCloth", ImVec2(buttonWidth, buttonHeight))) {
				liitleone = true;
			}

			if (ImGui::Button("Create BiggerCloth", ImVec2(buttonWidth, buttonHeight))) {
				BIgone = true;
			}
			if (ImGui::Button("+Add Gravity", ImVec2(buttonWidth, buttonHeight))) {
				gravity *= 10;
			}

			if (ImGui::Button("-Minus Gravity", ImVec2(buttonWidth, buttonHeight))) {
				gravity *= 0.1f;
			}
		}
		ImGui::End();

		if (liitleone) {
			cloth = clothIni;
			liitleone = false;
		}

		if (BIgone) {
			cloth = clothIniBigger;
			BIgone = false;
		}


		if (updateWithBall) {
			empty.setVec3("color", glm::vec3(1, 1, 0));
			ShowMyMesh(ball);
		}
		if (Terrain) {
			empty.setVec3("color", glm::vec3(1.0f, 0.6f, 0.8f));

			ShowMyMesh(terrain);
			TerrainRefresh(cloth, del);
			empty.setVec3("color", glm::vec3(0, 1, 0));
			ShowMyMesh(cloth);
		}


		if (BallWithTerrain) {
			empty.setVec3("color", glm::vec3(1.0f, 0.6f, 0.8f));

			ShowMyMesh(terrain);

			empty.setVec3("color", glm::vec3(1, 1, 0));
			ShowMyMesh(ball);

			TerrainWithBallRefresh(cloth, del);
			empty.setVec3("color", glm::vec3(0, 1, 0));
			ShowMyMesh(cloth);
		}

		if (RotateTheScene) {
			empty.setVec3("color", glm::vec3(1.0f, 0.6f, 0.8f));
			ShowMyMesh(terrain);

			empty.setVec3("color", glm::vec3(1, 1, 0));
			jiao += 0.001f;
			rotateMyBall(ball, jiao);
			ShowMyMesh(ball);

			RotationBallScene(cloth, del);
			empty.setVec3("color", glm::vec3(0, 1, 0));
			ShowMyMesh(cloth);
		}
		if (GandSpringforce) {
			RefreshGiveaGandSpringforce(cloth, del);
			empty.setVec3("color", glm::vec3(0, 1, 0));
			ShowMyMesh(cloth);
		}
		if (AllForce) {
			RefreshAllForce(cloth, del);
			empty.setVec3("color", glm::vec3(0, 1, 0));
			ShowMyMesh(cloth);
		}


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
}