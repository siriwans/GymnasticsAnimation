/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>
#include <algorithm>
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "Texture.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "Particle.h"
#include "WindowManager.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> progFloor;
	std::shared_ptr<Program> cubeProg;
	std::shared_ptr<Program> particleProg;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh;
   shared_ptr<Shape> cube;
   vector<vector<shared_ptr<Shape>>> allShapesArr;

   //PARTICLES
	std::vector<std::shared_ptr<Particle>> particles;
   int numP = 30;
	GLfloat points[900];
	GLfloat pointColors[1200];

	GLuint pointsbuffer;
	GLuint colorbuffer;
   int gMat = 0;

	// Display time to control fps
	float t0_disp = 0.0f;
	float t_disp = 0.0f;

	bool keyToggles[256] = { false };
	float t = 0.0f; //reset in init
	float h = 0.01f;
	glm::vec3 g = glm::vec3(0.0f, -0.01f, 0.0f);

   shared_ptr<Texture> texture;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vector<vec3> gMinArr;
	vector<vec3> gMaxArr;
   vec3 gMinCube;
   vec3 gMaxCube;


   vec3 lookAt = vec3(-0.014388, 0.003000, -0.999903);
   vec3 eye = vec3(-2.717521, 2.431891, 12.950317);

   vec3 up = vec3(0.0,1,0.0);

   float deltY = 0;
   float deltX = 3*3.14/2;

   unsigned int cubeMapTexture;


   //cpu data for textures
   shared_ptr<Texture> textureMAT;
   shared_ptr<Texture> textureBar;
   shared_ptr<Texture> textureBoard;
   shared_ptr<Texture> textureBeam;
   shared_ptr<Texture> textureRunway;
   shared_ptr<Texture> textureLandMat;


   //material number
   int matBluePlastic = 0;
   int matFlatGrey = 1;
   int matBrass = 2;
   int change = 0;

   //animation data
   float test = 0.0;
   float offset2 = 0.0;
   float down = 0.0;
   float offset = 0;
   float roundDown = 11;
   float backHP = 0.0;
   float cartwheel= 6.58;
   float waistTwist = 0;
   float hip = 0;
   float lean = 0;
	float sTheta = 0;
   float sTheta2 = 0;
	float sTheta3 = 0;
   float sTheta4 = 0;
   float roundOffThigh = 0;
   float roundOffShin = 0;
   float moving = 0;
   float timer = 0.0;
   float timer2 = 4.9;
   float twistTimer = 1.6;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
      if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
      //change material
      if (key == GLFW_KEY_M && action == GLFW_PRESS) {
         change = 1;
		}

      //controlling arm salute animation
		if (key == GLFW_KEY_X && action == GLFW_PRESS) {
         if (moving == 1)
            moving = 0;
         else
            moving = 1;
      }
      if (key == GLFW_KEY_W && action == GLFW_PRESS)
      {
         eye += 0.5f * lookAt;
      }
      if (key == GLFW_KEY_S && action == GLFW_PRESS)
      {
         eye -= 0.5f * lookAt;
      }
      if (key == GLFW_KEY_A && action == GLFW_PRESS)
      {
         eye -= glm::normalize(glm::cross(lookAt, up)) * 0.5f;
      }
      if (key == GLFW_KEY_D && action == GLFW_PRESS)
      {
         eye += glm::normalize(glm::cross(lookAt, up)) * 0.5f;
      }
      if (key == GLFW_KEY_R && action == GLFW_PRESS)
      {
         timer = 0;
         cartwheel = 6.58;
         roundDown = 11;
      }

	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

   void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
   {
      deltX -= deltaX * 0.03;
      deltY += deltaY * 0.03;
      if (deltY > 1.39)
      {
         deltY = 1.39;
      }
      if (deltY < -1.39)
      {
         deltY = -1.39;
      }
   }


	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//CHECKED_GL_CALL(glEnable(GL_BLEND));
		//CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(14.0f));

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag2.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("lightPos");
		prog->addUniform("MatDif");
		prog->addUniform("MatAmb");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("lightColor");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
      prog->addAttribute("vertTex");


		// Initialize the GLSL program.
		progFloor = make_shared<Program>();
		progFloor->setVerbose(true);
		progFloor->setShaderNames(resourceDirectory + "/tex_vert1.glsl", resourceDirectory + "/tex_frag1.glsl" );
		progFloor->init();
		progFloor->addUniform("P");
		progFloor->addUniform("V");
		progFloor->addUniform("M");
      progFloor->addUniform("Texture0");
		progFloor->addUniform("lightPos");
		progFloor->addAttribute("vertPos");
		progFloor->addAttribute("vertNor");
		progFloor->addAttribute("vertTex");


      glEnable(GL_DEPTH_TEST);
      // Initialize the GLSL program.
		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
	   cubeProg->init();
		cubeProg->addUniform("P");
	   cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addUniform("skybox");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");

      // Initialize the GLSL program.
      particleProg = make_shared<Program>();
		particleProg->setVerbose(true);
		particleProg->setShaderNames(
			resourceDirectory + "/lab10_vert.glsl",
			resourceDirectory + "/lab10_frag.glsl");
		if (! particleProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		particleProg->addUniform("P");
		particleProg->addUniform("M");
		particleProg->addUniform("V");
		particleProg->addUniform("alphaTexture");
		particleProg->addAttribute("vertPos");

	}

   // Code to load in the three textures
   void initTex(const std::string& resourceDirectory)
   {
      textureMAT = make_shared<Texture>();
      textureMAT->setFilename(resourceDirectory + "/mat.jpg");
      textureMAT->init();
      textureMAT->setUnit(0);
      textureMAT->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      textureBeam = make_shared<Texture>();
      textureBeam->setFilename(resourceDirectory + "/sandBeam.jpg");
      textureBeam->init();
      textureBeam->setUnit(1);
      textureBeam->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      textureBar = make_shared<Texture>();
      textureBar->setFilename(resourceDirectory + "/wood.jpg");
      textureBar->init();
      textureBar->setUnit(2);
      textureBar->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      textureRunway = make_shared<Texture>();
      textureRunway->setFilename(resourceDirectory + "/carpet.jpg");
      textureRunway->init();
      textureRunway->setUnit(3);
      textureRunway->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      textureLandMat = make_shared<Texture>();
      textureLandMat->setFilename(resourceDirectory + "/landMat.jpg");
      textureLandMat->init();
      textureLandMat->setUnit(4);
      textureLandMat->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      textureBoard = make_shared<Texture>();
      textureBoard->setFilename(resourceDirectory + "/board.jpg");
      textureBoard->init();
      textureBoard->setUnit(5);
      textureBoard->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

      //PARTICLES
		texture = make_shared<Texture>();
		texture->setFilename(resourceDirectory + "/alpha.bmp");
		texture->init();
		texture->setUnit(0);
		texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      
      vector<std::string> faces
      {
         "crowd.jpeg",
         "crowd.jpeg",
         //"orbital-element_lf.tga",
         //"orbital-element_rt.tga",
         //"orbital-element_up.tga",
         "sky.jpeg",
         //"orbital-element_dn.tga",
         "ground.jpeg",
         "crowd.jpeg",
         "crowd.jpeg",
         //"orbital-element_ft.tga",
         //"orbital-element_bk.tga"
      };

      cubeMapTexture = createSky(resourceDirectory + "/cracks/", faces);
   }  

   void initParticles()
	{
		int n = numP;

		for (int i = 0; i < n; ++ i)
		{
			auto particle = make_shared<Particle>();
			particles.push_back(particle);
			particle->load();
		}
	}

	void initGeom(const std::string& resourceDirectory)
	{

		// generate the VAO
		CHECKED_GL_CALL(glGenVertexArrays(1, &VertexArrayID));
		CHECKED_GL_CALL(glBindVertexArray(VertexArrayID));

		// generate vertex buffer to hand off to OGL - using instancing
		CHECKED_GL_CALL(glGenBuffers(1, &pointsbuffer));
		// set the current state to focus on our vertex buffer
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		// actually memcopy the data - only do this once
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));

		CHECKED_GL_CALL(glGenBuffers(1, &colorbuffer));
		// set the current state to focus on our vertex buffer
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		// actually memcopy the data - only do this once
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));

		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::shape_t> TOshapesSP;
 		vector<tinyobj::shape_t> TOshapesBB;
 		vector<tinyobj::shape_t> TOshapesD;
 		vector<tinyobj::shape_t> TOshapesF;
 		vector<tinyobj::shape_t> TOshapesV;
      vector<tinyobj::shape_t> TOshapesCube;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;

		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/unevenBars.obj").c_str());
      vector<shared_ptr<Shape>> allShapes;
      allShapesArr.push_back(allShapes);
      vec3 gMin, gMax;
      gMinArr.push_back(gMin);
      gMaxArr.push_back(gMax);
      loadMeshes(TOshapes, errStr, rc, 0);
 		
      rc = tinyobj::LoadObj(TOshapesBB, objMaterials, errStr, (resourceDirectory + "/balanceBeam.obj").c_str());
      vector<shared_ptr<Shape>> allShapes1;
      allShapesArr.push_back(allShapes1);
      vec3 gMin1, gMax1;
      gMinArr.push_back(gMin1);
      gMaxArr.push_back(gMax1);
      loadMeshes(TOshapesBB, errStr, rc, 1);

      rc = tinyobj::LoadObj(TOshapesD, objMaterials, errStr, (resourceDirectory + "/dummy.obj").c_str());
      vector<shared_ptr<Shape>> allShapes2;
      allShapesArr.push_back(allShapes2);
      vec3 gMin2, gMax2;
      gMinArr.push_back(gMin2);
      gMaxArr.push_back(gMax2);
      loadMeshes(TOshapesD, errStr, rc, 2);
      
      rc = tinyobj::LoadObj(TOshapesF, objMaterials, errStr, (resourceDirectory + "/cubeTex.obj").c_str());
      vector<shared_ptr<Shape>> allShapes3;
      allShapesArr.push_back(allShapes3);
      vec3 gMin3, gMax3;
      gMinArr.push_back(gMin3);
      gMaxArr.push_back(gMax3);
      loadMeshes(TOshapesF, errStr, rc, 3);

      rc = tinyobj::LoadObj(TOshapesV, objMaterials, errStr, (resourceDirectory + "/vault.obj").c_str());
      vector<shared_ptr<Shape>> allShapes4;
      allShapesArr.push_back(allShapes4);
      vec3 gMin4, gMax4;
      gMinArr.push_back(gMin4);
      gMaxArr.push_back(gMax4);
      loadMeshes(TOshapesV, errStr, rc, 4);

      rc = tinyobj::LoadObj(TOshapesSP, objMaterials, errStr, (resourceDirectory + "/board.obj").c_str());
      vector<shared_ptr<Shape>> allShapes5;
      allShapesArr.push_back(allShapes5);
      vec3 gMin5, gMax5;
      gMinArr.push_back(gMin5);
      gMaxArr.push_back(gMax5);
      loadMeshes(TOshapesSP, errStr, rc, 5);

      
      rc = tinyobj::LoadObj(TOshapesCube, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			cube = make_shared<Shape>();
			cube->createShape(TOshapesCube[0]);
			cube->measure();
			cube->init();
		}
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMinCube.x = cube->min.x;
		gMinCube.y = cube->min.y;
		gMinCube.z = cube->min.z;
      gMaxCube.x = cube->max.x;
      gMaxCube.y = cube->max.y;
      gMaxCube.z = cube->max.z;
	}


	// Note you could add scale later for each particle - not implemented
	void updateGeom()
	{
		glm::vec3 pos;
		glm::vec4 col;

		// go through all the particles and update the CPU buffer
		for (int i = 0; i < numP; i++)
		{
			pos = particles[i]->getPosition();
			col = particles[i]->getColor();
			points[i * 3 + 0] = pos.x;
			points[i * 3 + 1] = pos.y;
			points[i * 3 + 2] = pos.z;
			pointColors[i * 4 + 0] = col.r + col.a / 10.f;
			pointColors[i * 4 + 1] = col.g + col.g / 10.f;
			pointColors[i * 4 + 2] = col.b + col.b / 10.f;
			pointColors[i * 4 + 3] = col.a;
		}

		// update the GPU data
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, points));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, pointColors));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
   
   /* note for first update all particles should be "reborn"
	 * which will initialize their positions */
	void updateParticles()
	{
		// update the particles
		for (auto particle : particles)
		{
			particle->update(t, h, g, keyToggles);
		}
		t += h;

		// Sort the particles by Z
		auto temp = make_shared<MatrixStack>();
		//temp->rotate(camRot, vec3(0, 1, 0));

		ParticleSorter sorter;
		sorter.C = temp->topMatrix();
		std::sort(particles.begin(), particles.end(), sorter);
	}

   void loadMeshes(vector<tinyobj::shape_t> TOshapes, string errStr, bool rc, int objIndex)
   {
      int i;
      if (!rc) 
      {
			cerr << errStr << endl;
		} else 
      {
			mesh = make_shared<Shape>();
			mesh->createShape(TOshapes[0]);
			mesh->measure();
			mesh->init();
         gMinArr[objIndex].x = mesh->min.x;
         gMinArr[objIndex].y = mesh->min.y;
         gMinArr[objIndex].z = mesh->min.z;
         gMaxArr[objIndex].x = mesh->max.x;
         gMaxArr[objIndex].y = mesh->max.y;
         gMaxArr[objIndex].z = mesh->max.z;
         
         for (i = 0; i < TOshapes.size(); ++i)
         {
            shared_ptr<Shape> temp = make_shared<Shape>();
            temp->createShape(TOshapes[i]);
            temp->measure();
            temp->init();
            if (temp->min.x <= gMinArr[objIndex].x)
               gMinArr[objIndex].x = temp->min.x;
            if (temp->min.y <= gMinArr[objIndex].y)
               gMinArr[objIndex].y = temp->min.y;
            if (temp->min.z <= gMinArr[objIndex].z)
               gMinArr[objIndex].z = temp->min.z;
            if (temp->max.x >= gMaxArr[objIndex].x)
               gMaxArr[objIndex].x = temp->max.x;
            if (temp->max.y >= gMaxArr[objIndex].y)
               gMaxArr[objIndex].y = temp->max.y;
            if (temp->max.z >= gMaxArr[objIndex].z)
               gMaxArr[objIndex].z = temp->max.z;

            allShapesArr[objIndex].push_back(temp);
         }
		}
   }

   unsigned int createSky(string dir, vector<string> faces)
   {
      unsigned int textureID;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
      int width, height, nrChannels;
      stbi_set_flip_vertically_on_load(false);
      for(GLuint i = 0; i < faces.size(); i++)
      {
         unsigned char *data =
         stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
         if (data)
         {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
         }
         else
         {
            cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
         }
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      //cout << " creating cube map any errors : " << glGetError() << endl;
      return textureID;
   }



	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }

	void setProjection(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> P) {
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
   }


   void drawUnevenBars(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      int i;
      float maxVal = std::max(abs(gMaxArr[0].x), std::max(abs(gMaxArr[0].y), std::max(abs(gMaxArr[0].z), std::max(abs(gMinArr[0].x), std::max(abs(gMinArr[0].y), abs(gMinArr[0].z))))));
      float scaleVal = 1/maxVal;
      Model->pushMatrix();
         Model->translate(vec3(-8, -1.3, 0));
         Model->scale(vec3(scaleVal*3.5,scaleVal*3.5, scaleVal*3.5));
         Model->rotate(3.14/2, vec3(0, 0, 1));
         Model->rotate(3.14*3 + 0.78, vec3(1, 0, 0));
         Model->rotate(3.14/2, vec3(0, 1, 0));
         //translate to origin
         Model->translate(vec3(abs((gMaxArr[0].x - gMinArr[0].x)/2)*-1, abs((gMaxArr[0].y - gMinArr[0].y)/2)*-1, abs((gMaxArr[0].z - gMinArr[0].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i < allShapesArr[0].size(); ++i)
         {
            allShapesArr[0].at(i)->draw(prog);
         }
      Model->popMatrix();
   }

   void drawBackHP(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model, float maxValD, float scaleValD)
   {
      Model->pushMatrix();
         if (timer > 8)
         {
            Model->translate(vec3(0, -4, 8.3 + offset));
            Model->rotate(roundDown, vec3(1, 0, 0));
            Model->rotate(1.57, vec3(1,0,0));
            Model->translate(vec3(0, 4, -8.5));
         }
         if (timer > 4.5)
         {
            Model->translate(vec3(0, -4, 6.4));
            Model->rotate(backHP * 1.2, vec3(1, 0, 0));
            Model->rotate(1.57, vec3(1,0,0));
            Model->translate(vec3(0, 3.5, -5));
         }
         Model->translate(vec3(0, -4.8, 2.4));
         Model->rotate(cartwheel, vec3(1, 0, 0));
         Model->translate(vec3(0, 3.5, -3));//change Z to make it pop off the ground a little (more neg)
         Model->pushMatrix();
            /*!!!!!!!!!!! NEED TO DO ENTIRE ROUND OFF MOVING ENTIRE DUMMY SOME HOW!!!!!!*/
            Model->translate(vec3(0, -4, 0));
            Model->rotate(lean, vec3(1, 0, 0));
            Model->translate(vec3(0, 3, 0));
            //MOVING UPPER BODY FOR ROUND OFF LUNGE
            Model->pushMatrix();
               Model->translate(vec3(-1.0, -1.9, -0.15));
               Model->rotate(waistTwist, vec3(0, 1, 0));
               Model->translate(vec3(1.0, 1.9, 0.15));
               Model->translate(vec3(0, -1.9, -0.18));
               //Model->rotate(hip, vec3(1, 0, 0));
               Model->pushMatrix();
                  //SALUTE LEFT ARM
                  Model->pushMatrix();
                     Model->translate(vec3(0, 1.9, 0.18));
                     Model->translate(vec3(-0.87, -1.4, 0));
                     Model->rotate(sTheta, vec3(0,0,1));
                     Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
                     Model->pushMatrix();
                        Model->translate(vec3(59.8,-1,0));
                        //Model->rotate(1.57, vec3(0,0,1)); //90 degrees
                        if (timer > 11.7)
                        {
                           Model->rotate(sTheta2, vec3(0,0,1)); //little FLICK
                        }
                        Model->rotate(3.14/2, vec3(0, 0, 1));
                        Model->rotate(3.14*3, vec3(1, 0, 0));
                        Model->rotate(3.14/2, vec3(0, 1, 0));
                        Model->translate(vec3(0,12,-48.5));
                        Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                        setModel(prog, Model);
                        allShapesArr[2].at(6)->draw(prog); //left hand
                     Model->popMatrix();
                     Model->rotate(3.14/2, vec3(0, 0, 1));
                     Model->rotate(3.14*3, vec3(1, 0, 0));
                     Model->rotate(3.14/2, vec3(0, 1, 0));
                     Model->translate(vec3(0, 72.5, -48.5));
                     Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                     setModel(prog, Model);
                     allShapesArr[2].at(11)->draw(prog); //left shoulder
                     allShapesArr[2].at(7)->draw(prog); //left wrist
                     allShapesArr[2].at(8)->draw(prog); //left lower arm
                     allShapesArr[2].at(9)->draw(prog); //left elbow
                     allShapesArr[2].at(10)->draw(prog); //left upper arm
                  Model->popMatrix(); 
                  //SALUTE RIGHT ARM
                  Model->pushMatrix();
                     Model->translate(vec3(0, 1.9, 0.18));
                     Model->translate(vec3(-1.12, -1.4, 0));
                     Model->rotate(sTheta3, vec3(0,0,1));
                     Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
                     Model->pushMatrix();
                        Model->translate(vec3(-61,0,0));
                        if (timer > 11.7)
                        {
                           Model->rotate(sTheta4, vec3(0,0,1)); // little FLICK
                        }
                        //Model->rotate(-1.57, vec3(0,0,1)); //90 degrees
                        Model->rotate(3.14/2, vec3(0, 0, 1));
                        Model->rotate(3.14*3, vec3(1, 0, 0));
                        Model->rotate(3.14/2, vec3(0, 1, 0));
                        Model->translate(vec3(0,165,-48.5));
                        Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                        setModel(prog, Model);
                        allShapesArr[2].at(22)->draw(prog); //right hand
                     Model->popMatrix();
                     Model->rotate(3.14/2, vec3(0, 0, 1));
                     Model->rotate(3.14*3, vec3(1, 0, 0));
                     Model->rotate(3.14/2, vec3(0, 1, 0));
                     Model->translate(vec3(0, 103.7, -48.5));
                     Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                     setModel(prog, Model);
                     allShapesArr[2].at(15)->draw(prog); //right shoulder 
                     allShapesArr[2].at(27)->draw(prog); //right lower arm 
                     allShapesArr[2].at(18)->draw(prog); //right elbow
                     allShapesArr[2].at(12)->draw(prog); //right upper arm
                     allShapesArr[2].at(28)->draw(prog); //right wrist
                  Model->popMatrix();
                  if(timer <= 3.3)
                  {
                     Model->translate(vec3(0, 0, -0.05));
                     Model->rotate(3.14/2, vec3(0, 0, 1));
                     Model->rotate(3.14*3, vec3(1, 0, 0));
                     Model->rotate(3.14/2, vec3(0, 1, 0));
                     Model->translate(vec3(0.2, 0, -0.1));
                     Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
                     Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                     setModel(prog, Model);
                     allShapesArr[2].at(0)->draw(prog); //left hip joint
                     allShapesArr[2].at(1)->draw(prog); //left thigh
                     allShapesArr[2].at(2)->draw(prog); //left knee
                     allShapesArr[2].at(3)->draw(prog); //left shin
                     allShapesArr[2].at(4)->draw(prog); //left ankle
                     allShapesArr[2].at(5)->draw(prog); //left foot
                  }
               Model->popMatrix();
               Model->translate(vec3(-1, 0, 0));
               Model->rotate(3.14/2, vec3(0, 0, 1));
               Model->rotate(3.14*3, vec3(1, 0, 0));
               Model->rotate(3.14/2, vec3(0, 1, 0));
               Model->translate(vec3(0.15, 1, -0.1));
               Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
               Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
               setModel(prog, Model);
               allShapesArr[2].at(13)->draw(prog); //neck
               allShapesArr[2].at(17)->draw(prog); //head
               allShapesArr[2].at(21)->draw(prog); //chest
               allShapesArr[2].at(23)->draw(prog); //waist
               if(timer <= 3.3)
               {
                  allShapesArr[2].at(24)->draw(prog); //hips
               }
            Model->popMatrix();

            // right leg lunge
            Model->pushMatrix();
               Model->translate(vec3(-1.0, -1.9, -0.15));
               if (timer <= 4)
               {
                  Model->rotate(waistTwist, vec3(0, 1, 0));
               }
               else if (timer >= 4 && timer <= 5)
               {
                  Model->translate(vec3(0, 1.9, 0.18));
                  Model->translate(vec3(0, -1.9, -0.18));
                  Model->rotate(3.14, vec3(0,1,0));
                  Model->rotate(-2.35619, vec3(1,0,0));
                  Model->rotate(-down, vec3(1, 0, 0));
               }
               else if (timer >= 7.4 && timer < 9)
               {
                  Model->translate(vec3(0, 1.9, 0.18));
                  Model->translate(vec3(0, -1.9, -0.18));
                  Model->rotate(3.14, vec3(0,1,0));
                  Model->rotate(-2.35619, vec3(1,0,0));
                  Model->rotate(-down, vec3(1, 0, 0));
               }
               else if (timer > 10.7 && timer < 13)
               {
                  Model->translate(vec3(0, 1.9, 0.18));
                  Model->translate(vec3(0, -1.9, -0.18));
                  Model->rotate(3.14, vec3(0,1,0));
                  Model->rotate(-2.35619, vec3(1,0,0));
                  Model->rotate(-down, vec3(1, 0, 0));
               }
               else
               {
                  Model->translate(vec3(0, 1.9, 0.18));
                  Model->translate(vec3(0, -1.9, -0.18));
                  Model->rotate(3.14, vec3(0,1,0));
                  Model->rotate(-2.35619, vec3(1,0,0));
                  Model->rotate(-down, vec3(1, 0, 0));
               }

               Model->pushMatrix();
                  Model->pushMatrix();
                     Model->pushMatrix();
                        Model->translate(vec3(0, -1.95, -0.17));
                        Model->translate(vec3(1.0, 1.9, 0.15));
                        Model->rotate(roundOffThigh, vec3(1,0,0));
                        Model->pushMatrix();
                           Model->rotate(3.14/2, vec3(0, 0, 1));
                           Model->rotate(3.14, vec3(1, 0, 0));
                           Model->rotate(3.14/2, vec3(0, 1, 0));
                           Model->translate(vec3(0.165,0, -0.05));
                           Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
                           Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                           setModel(prog, Model);
                           allShapesArr[2].at(25)->draw(prog); //right hip joint
                           allShapesArr[2].at(14)->draw(prog); //right thigh
                        Model->popMatrix();
                        Model->translate(vec3(0, -0.5, 0));
                        Model->rotate(roundOffShin, vec3(1,0,0));
                        Model->rotate(3.14/2, vec3(0, 0, 1));
                        Model->rotate(3.14, vec3(1, 0, 0));
                        Model->rotate(3.14/2, vec3(0, 1, 0));
                        Model->translate(vec3(0.15, 0, 0.45));
                        Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
                        Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                        setModel(prog, Model);
                        allShapesArr[2].at(16)->draw(prog); //right knee
                        allShapesArr[2].at(19)->draw(prog); //right shin
                        allShapesArr[2].at(20)->draw(prog); //right ankle
                        allShapesArr[2].at(26)->draw(prog); //right foot
                     Model->popMatrix();
                     if(timer > 3.3)
                     {
                        Model->translate(vec3(1.0, 1.9, 0.15));
                        Model->translate(vec3(0, -1.9, -0.18));
                        Model->translate(vec3(0, 0, -0.05));
                        Model->rotate(3.14/2, vec3(0, 0, 1));
                        Model->rotate(3.14*3, vec3(1, 0, 0));
                        Model->rotate(3.14/2, vec3(0, 1, 0));
                        Model->translate(vec3(0.2, 0, -0.1));
                        Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
                        Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                        setModel(prog, Model);
                        allShapesArr[2].at(0)->draw(prog); //left hip joint
                        allShapesArr[2].at(1)->draw(prog); //left thigh
                        allShapesArr[2].at(2)->draw(prog); //left knee
                        allShapesArr[2].at(3)->draw(prog); //left shin
                        allShapesArr[2].at(4)->draw(prog); //left ankle
                        allShapesArr[2].at(5)->draw(prog); //left foot
                     }
                  Model->popMatrix();
                  if (timer > 3.3)
                  {
                     Model->translate(vec3(1.0, 1.9, 0.15));
                     Model->translate(vec3(0, -1.9, -0.18));
                     Model->translate(vec3(-1, 0, 0));
                     Model->rotate(3.14/2, vec3(0, 0, 1));
                     Model->rotate(3.14*3, vec3(1, 0, 0));
                     Model->rotate(3.14/2, vec3(0, 1, 0));
                     Model->translate(vec3(0.15, 1, -0.1));
                     Model->scale(vec3(scaleValD*2, scaleValD*2, scaleValD*2));
                     Model->translate(vec3(abs((gMaxArr[2].x - gMinArr[2].x)/2)*-1, abs((gMaxArr[2].y - gMinArr[2].y)/2)*-1, abs((gMaxArr[2].z - gMinArr[2].z)/2)*-1));
                     setModel(prog, Model);
                     allShapesArr[2].at(24)->draw(prog); //hips
                  }
               Model->popMatrix();
            Model->popMatrix();
         Model->popMatrix();
      Model->popMatrix();
   }

   void drawDummy(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      float maxValD = std::max(abs(gMaxArr[2].x), std::max(abs(gMaxArr[2].y), std::max(abs(gMaxArr[2].z), std::max(abs(gMinArr[2].x), std::max(abs(gMinArr[2].y), abs(gMinArr[2].z))))));
      float scaleValD = 1/maxValD;

      Model->pushMatrix();
         Model->translate(vec3(-2, 1.5, -15));
         drawBackHP(prog, Model, maxValD, scaleValD);
      Model->popMatrix();
      
   }

   void animationData(float moving)
   {
      //increment timer (own timer for animation)
      if (moving == 1 && timer < 11.7)
      {
         if (timer > 10.7 && down > 3.5 && timer < 11.4)
         {
            down -= 0.05;
         }
         if (timer > 8)
         {
            roundDown += 0.03;
            offset += 0.0008;
         }
         if (timer < 7.3)
         {
            backHP = timer;
         }
         if (timer >= 7 && down < 4.3)
         {
            down += 0.03;
         }
         if (timer < 4.3)
         {
            down = timer;
         }
         if (timer >= 5 && cartwheel < 10 && down > 3.5 && timer < 7)
         {
            down -= 0.03;
         }
         if (timer < 3.5)
         {
            roundOffShin = fmax(sin(timer+0.2) * 1.4, 0.0) * 1.4;
            roundOffThigh = fmin(sin(-timer+0.2) * 1.4, 0.0) * 1.4;
         }
         if (timer > 3.3 && timer <= 5.5)
         {
            waistTwist += 0.03;
         }

         if (timer < 1.6)
         {
            waistTwist = fmax(0.0,sin(timer));
            waistTwist = fmax(waistTwist, 0.8);
            lean = fmax(sin(timer)*1.5, 0.0);
            hip = fmin(sin(timer), 0.75);
            hip = fmax(hip,0.0);
            sTheta = sin(timer) * 1.4;
            sTheta2 = sin(-timer);
            sTheta3 = sin(-timer) * 1.4;
            sTheta4 = sin(timer);
         }
         else if (cartwheel < 9.4)
         {
            cartwheel += 0.03;
         }
         timer += 0.03;
      }

      if (moving == 1 && timer > 11.7 && timer2 < 5)
      {
         sTheta = sin(-timer2) * 1.4;
         sTheta2 = sin(timer2);
         sTheta3 = sin(timer2) * 1.4;
         sTheta4 = sin(-timer2);
         timer2 += 0.03;
      }
   }
   
   void drawPlatform(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      float maxValF = std::max(abs(gMaxArr[3].x), std::max(abs(gMaxArr[3].y), std::max(abs(gMaxArr[3].z), std::max(abs(gMinArr[3].x), std::max(abs(gMinArr[3].y), abs(gMinArr[3].z))))));
      float scaleValF = 1/maxValF;
      int i;
      Model->pushMatrix();
         Model->translate(vec3(5,-4,100));
         Model->rotate(-0.78, vec3(0,1,0));
         Model->scale(vec3(scaleValF*15000, scaleValF * 0.5, scaleValF*15000));
         Model->translate(vec3(abs((gMaxArr[3].x - gMinArr[3].x)/2)*-1, abs((gMaxArr[3].y - gMinArr[3].y)/2)*-1, abs((gMaxArr[3].z - gMinArr[3].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i< allShapesArr[3].size(); ++i)
         {
            allShapesArr[3].at(i)->draw(prog);
         }
      Model->popMatrix();
   }

   void drawRunway(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      float maxValF = std::max(abs(gMaxArr[3].x), std::max(abs(gMaxArr[3].y), std::max(abs(gMaxArr[3].z), std::max(abs(gMinArr[3].x), std::max(abs(gMinArr[3].y), abs(gMinArr[3].z))))));
      float scaleValF = 1/maxValF;
      int i;
      Model->pushMatrix();
         Model->translate(vec3(8.2,-2.85,-8.9));
         Model->rotate(-0.78, vec3(0,1,0));
         Model->scale(vec3(scaleValF*6.3, scaleValF*0.05, scaleValF* 1.3));
         Model->translate(vec3(abs((gMaxArr[3].x - gMinArr[3].x)/2)*-1, abs((gMaxArr[3].y - gMinArr[3].y)/2)*-1, abs((gMaxArr[3].z - gMinArr[3].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i< allShapesArr[3].size(); ++i)
         {
            allShapesArr[3].at(i)->draw(prog);
         }
      Model->popMatrix();
   }

   void drawFloorEx(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      float maxValF = std::max(abs(gMaxArr[3].x), std::max(abs(gMaxArr[3].y), std::max(abs(gMaxArr[3].z), std::max(abs(gMinArr[3].x), std::max(abs(gMinArr[3].y), abs(gMinArr[3].z))))));
      float scaleValF = 1/maxValF;
      int i;
      Model->pushMatrix();
         Model->translate(vec3(-4,-4.2, 2.0));
         Model->rotate(-0.78, vec3(0,1,0));
         Model->scale(vec3(scaleValF*11, scaleValF * 0.08, scaleValF*11));
         Model->translate(vec3(abs((gMaxArr[3].x - gMinArr[3].x)/2)*-1, abs((gMaxArr[3].y - gMinArr[3].y)/2)*-1, abs((gMaxArr[3].z - gMinArr[3].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i< allShapesArr[3].size(); ++i)
         {
            allShapesArr[3].at(i)->draw(prog);
         }
      Model->popMatrix();
   }

   void drawBalanceBeam(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      int i;
      float maxValBB = std::max(abs(gMaxArr[1].x), std::max(abs(gMaxArr[1].y), std::max(abs(gMaxArr[1].z), std::max(abs(gMinArr[1].x), std::max(abs(gMinArr[1].y), abs(gMinArr[1].z))))));
      float scaleValBB = 1/maxValBB;
      Model->pushMatrix();
         Model->translate(vec3(4, -2.3, 0.0));
         Model->rotate(3.14/2, vec3(0, 0, 1));
         Model->rotate(3.14+2.34, vec3(1, 0, 0));
         Model->rotate(3.14/2, vec3(0, 1, 0));
         Model->scale(vec3(scaleValBB*2, scaleValBB*2, scaleValBB*2));
         Model->translate(vec3(abs((gMaxArr[1].x - gMinArr[1].x)/2)*-1, abs((gMaxArr[1].y - gMinArr[1].y)/2)*-1, abs((gMaxArr[1].z - gMinArr[1].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i< allShapesArr[1].size(); ++i)
         {
            allShapesArr[1].at(i)->draw(prog);
         }
      Model->popMatrix();
   }

   void drawVault(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      int i;
      float maxValBB = std::max(abs(gMaxArr[4].x), std::max(abs(gMaxArr[4].y), std::max(abs(gMaxArr[4].z), std::max(abs(gMinArr[4].x), std::max(abs(gMinArr[4].y), abs(gMinArr[4].z))))));
      float scaleValBB = 1/maxValBB;
      Model->pushMatrix();
         Model->translate(vec3(10, -2.3, -9));
         Model->rotate(3.14/2, vec3(0, 0, 1));
         Model->rotate(3.14+2.34, vec3(1, 0, 0));
         Model->rotate(3.14/2, vec3(0, 1, 0));
         Model->scale(vec3(scaleValBB*3, scaleValBB*3, scaleValBB*3));
         Model->translate(vec3(abs((gMaxArr[4].x - gMinArr[4].x)/2)*-1, abs((gMaxArr[4].y - gMinArr[4].y)/2)*-1, abs((gMaxArr[4].z - gMinArr[4].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i< allShapesArr[4].size(); ++i)
         {
            allShapesArr[4].at(i)->draw(prog);
         }
      Model->popMatrix();
   }

   void drawBoard(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      int i;
      float maxValBB = std::max(abs(gMaxArr[5].x), std::max(abs(gMaxArr[5].y), std::max(abs(gMaxArr[5].z), std::max(abs(gMinArr[5].x), std::max(abs(gMinArr[5].y), abs(gMinArr[5].z))))));
      float scaleValBB = 1/maxValBB;
      Model->pushMatrix();
         Model->translate(vec3(8, -2.5, -9.1));
         Model->rotate(3.14/2, vec3(0, 0, 1));
         Model->rotate(3.14+2.34 + 3.14, vec3(1, 0, 0));
         Model->rotate(3.14/2, vec3(0, 1, 0));
         Model->scale(vec3(scaleValBB*2, scaleValBB*2, scaleValBB*2));
         Model->translate(vec3(abs((gMaxArr[5].x - gMinArr[5].x)/2)*-1, abs((gMaxArr[5].y - gMinArr[5].y)/2)*-1, abs((gMaxArr[5].z - gMinArr[5].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i< allShapesArr[5].size(); ++i)
         {
            allShapesArr[5].at(i)->draw(prog);
         }
      Model->popMatrix();
   }

   void drawLandingMat(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model)
   {
      float maxValF = std::max(abs(gMaxArr[3].x), std::max(abs(gMaxArr[3].y), std::max(abs(gMaxArr[3].z), std::max(abs(gMinArr[3].x), std::max(abs(gMinArr[3].y), abs(gMinArr[3].z))))));
      float scaleValF = 1/maxValF;
      int i;
      Model->pushMatrix();
         Model->rotate(-0.78, vec3(0,1,0));
         Model->scale(vec3(scaleValF*3, scaleValF * 0.1, scaleValF*2));
         Model->translate(vec3(abs((gMaxArr[3].x - gMinArr[3].x)/2)*-1, abs((gMaxArr[3].y - gMinArr[3].y)/2)*-1, abs((gMaxArr[3].z - gMinArr[3].z)/2)*-1));
         setModel(prog, Model);
         for (i = 0; i< allShapesArr[3].size(); ++i)
         {
            allShapesArr[3].at(i)->draw(prog);
         }
      Model->popMatrix();
   }


   void changeMat(int i)
   {
      if (change == 1)
      {
         matBluePlastic = (matBluePlastic + 1) % 4;
         matFlatGrey = (matFlatGrey + 1) % 4;
         matBrass = (matBrass + 1) % 4;
         change = 0;
      }
   }

   void SetMaterial(int i) {
      switch (i) 
      {
         case 0: //shiny blue plastic
            glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
            glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
            glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
            glUniform1f(prog->getUniform("shine"), 120.0);
            break;
         case 1: // flat grey
            glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
            glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
            glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
            glUniform1f(prog->getUniform("shine"), 4.0);
            break;
         case 2: //brass
            glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
            glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
            glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
            glUniform1f(prog->getUniform("shine"), 27.9);
            break;
         case 3: //mint green
            glUniform3f(prog->getUniform("MatAmb"), 0.294, 0.3599, 0.745);
            glUniform3f(prog->getUniform("MatDif"), 0.24, 0.76, 0.53);
            glUniform3f(prog->getUniform("MatSpec"), 0.822, 0.976, 0.384);
            glUniform1f(prog->getUniform("shine"), 16.3);
            break;
   }
}

	void render() 
   {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
      // Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

      float lookAtX = cos(deltY)*cos(deltX);
      float lookAtY = sin(deltY);
      float lookAtZ = cos(deltY)*cos((3.14/2.0)-deltX);

      lookAt = vec3(lookAtX, lookAtY, lookAtZ);

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
      auto View = glm::lookAt(eye, eye + lookAt, up);
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

      cubeProg->bind();
      //set the projection matrix - can use the same one
      glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
      //set the depth function to always draw the box!
      glDepthFunc(GL_LEQUAL);
      glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE,value_ptr(View));

      Model->pushMatrix();
         Model->loadIdentity();
         Model->translate(vec3(0, 0, -10));
         Model->scale(vec3(100, 100, 100));
         setModel(cubeProg, Model);
         glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
         glDisable(GL_DEPTH_TEST);
         cube->draw(cubeProg);
         glEnable(GL_DEPTH_TEST);

         //set the depth test back to normal!
         glDepthFunc(GL_LESS);
      Model->popMatrix();

      //unbind the shader for the skybox
      cubeProg->unbind();


		prog->bind();

		setProjection(prog, Projection);
      glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View));

      glUniform3f(prog->getUniform("lightPos"), -2.0, 5.0, 2.0);
      glUniform3f(prog->getUniform("lightColor"), 1, 1, 1);

		// draw mesh 
      Model->translate(vec3(0, 2, 0)); 
      Model->pushMatrix();
			Model->loadIdentity();
         Model->translate(vec3(0, 2, 0)); 
         
         setProjection(prog, Projection);
         glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View));

         //DUMMMYYY
         SetMaterial(matBrass);
         drawDummy(prog, Model);

         //PLATFORRMMM
         SetMaterial(matFlatGrey);
         Model->translate(vec3(0,-2, 0));
         drawPlatform(prog, Model);

		Model->popMatrix();
      
      prog->unbind();

      progFloor->bind();

      glUniform3f(progFloor->getUniform("lightPos"),  -2, 5.0, 2.0);
      textureBeam->bind(progFloor->getUniform("Texture0"));
      Model->pushMatrix();
         //BALANCE BEAMMMMM
         setProjection(progFloor, Projection);
         glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View)); 
         Model->translate(vec3(0,-1, 0));
         drawBalanceBeam(progFloor, Model);
		Model->popMatrix(); 

      textureBar->bind(progFloor->getUniform("Texture0"));
      Model->pushMatrix();
         //UNEVENBARS
         setProjection(progFloor, Projection);
         glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View));
         Model->translate(vec3(0,-1, 0));
         drawUnevenBars(progFloor, Model);
		Model->popMatrix();

      textureMAT->bind(progFloor->getUniform("Texture0"));
      Model->pushMatrix();
         //FLOOR EXERCISE
         setProjection(progFloor, Projection);
         glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View));
         drawFloorEx(progFloor, Model);
      Model->popMatrix();

      Model->pushMatrix();
         Model->translate(vec3(-4, 0.0, -5.0));
         textureBar->bind(progFloor->getUniform("Texture0"));
         Model->pushMatrix();
            //VAULT
            setProjection(progFloor, Projection);
            glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View));
            Model->translate(vec3(2,-1.2, -10));
            drawVault(progFloor, Model);
         Model->popMatrix();

         textureBoard->bind(progFloor->getUniform("Texture0"));
         Model->pushMatrix();
            //SPRING BOARD
            setProjection(progFloor, Projection);
            glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View));
            Model->translate(vec3(2,-1.2, -10));
            drawBoard(progFloor, Model);
         Model->popMatrix();

         textureRunway->bind(progFloor->getUniform("Texture0"));
         Model->pushMatrix();
            //RUNWAY
            setProjection(progFloor, Projection);
            glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View));
            Model->translate(vec3(2,-1.2, -10));
            drawRunway(progFloor, Model);
         Model->popMatrix();

         textureLandMat->bind(progFloor->getUniform("Texture0"));
         Model->pushMatrix();
            //LANDING MAT
            setProjection(progFloor, Projection);
            glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View));
            Model->translate(vec3(17,-4,-10.5));
            drawLandingMat(progFloor, Model);
         Model->popMatrix();
      Model->popMatrix();

      textureLandMat->bind(progFloor->getUniform("Texture0"));
      Model->pushMatrix();
         //LANDING MAT for bars
         setProjection(progFloor, Projection);
         glUniformMatrix4fv(progFloor->getUniform("V"), 1, GL_FALSE, value_ptr(View));
         Model->translate(vec3(-6,-4, 5));
         Model->scale(vec3(0.7, 1.0, 0.6));
         drawLandingMat(progFloor, Model);
      Model->popMatrix();

      //animation!!!
      animationData(moving);
      
      //change material
      changeMat(change);

      progFloor->unbind();
      
      if (timer > 11.7 && timer < 15)
      {
         CHECKED_GL_CALL(glEnable(GL_BLEND));
         CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
         Model->pushMatrix();
            Model->loadIdentity();

            // Draw
            particleProg->bind();
            updateParticles();
            updateGeom();

            Model->translate(vec3(-3, 1.3, -5.5));

            texture->bind(particleProg->getUniform("alphaTexture"));
            CHECKED_GL_CALL(glUniformMatrix4fv(particleProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())));
            CHECKED_GL_CALL(glUniformMatrix4fv(particleProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix())));
            CHECKED_GL_CALL(glUniformMatrix4fv(particleProg->getUniform("V"), 1, GL_FALSE, value_ptr(View)));


            CHECKED_GL_CALL(glEnableVertexAttribArray(0));
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
            CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0));

            CHECKED_GL_CALL(glEnableVertexAttribArray(1));
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
            CHECKED_GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0));

            CHECKED_GL_CALL(glVertexAttribDivisor(0, 1));
            CHECKED_GL_CALL(glVertexAttribDivisor(1, 1));
            // Draw the points !
            CHECKED_GL_CALL(glDrawArraysInstanced(GL_POINTS, 0, 1, numP));

            CHECKED_GL_CALL(glVertexAttribDivisor(0, 0));
            CHECKED_GL_CALL(glVertexAttribDivisor(1, 0));
            CHECKED_GL_CALL(glDisableVertexAttribArray(0));
            CHECKED_GL_CALL(glDisableVertexAttribArray(1));
            particleProg->unbind();

         Model->popMatrix();
         
         CHECKED_GL_CALL(glDisable(GL_BLEND));
      }
      // Pop matrix stacks.
		Projection->popMatrix();

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initParticles();
   application->initTex(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
