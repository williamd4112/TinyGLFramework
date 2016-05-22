#include <glew.h>
#include <freeglut.h>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <tiny_obj_loader.h>
#include "fbxloader.h"
#include <IL/il.h>
#include "GLPerspectiveCamera.h"
#include <unordered_map>

#define SetArray4f(arr, x, y, z, w) (arr)[0] = (x); \
					(arr)[1] = (y); \
					(arr)[2] = (z); \
					(arr)[3] = (w) 

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define MENU_ABSTRACTION 4
#define MENU_LAPLACIAN_FILTER 5
#define MENU_SHARPNESS_FILTER 6
#define MENU_PIXELATION 7
#define MENU_BLOOM 8
#define MENU_HALFTONE 9
#define MENU_RADIAL_BLUR 10
#define MENU_RIPPLE 11

#define NUM_TEXTURE_MAP 1
#define DIFFUSE_MAP 0

#define NUM_VBO 4
#define VBO_POS 0
#define VBO_NORMAL 1
#define VBO_TEXCOORD 2
#define VBO_INDEX 3

#define POSITION_LOC 0
#define NORMAL_LOC 1
#define TEXCOORD_LOC 2

#define LIGHTSOURCE_NUM_PROPS 12
#define POS_AMBIENT 0
#define POS_DIFFUSE 1
#define POS_SPECULAR 2
#define POS_POSITION 3
#define POS_HALFVECTOR 4
#define POS_SPOT_DIR 5
#define POS_SPOT_EXP 6
#define POS_SPOT_CUTOFF 7
#define POS_SPOT_COSTCUTOFF 8
#define POS_CONSTANT_ATTENUATION 9
#define POS_CONSTANT_LINEAR_ATTENUATION 10
#define POS_CONSTANT_QUADRATIC_ATTENUATION 11

#define MATERIAL_NUM_PROPS 4
#define POS_MAT_AMBIENT 0
#define POS_MAT_DIFFUSE 1
#define POS_MAT_SPECULAR 2
#define POS_MAT_SHININESS 3

#define NUM_LIGHTS 3
#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

#define NUM_PASS 4

static const GLfloat window_vertex[] = {
	//vec2 position vec2 texture_coord 
	1.0f,-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,0.0f,
	-1.0f,1.0f,0.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};

static GLuint draw_buffers[NUM_PASS] = {
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
};

struct LightSourceParameters {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float position[4];
	float halfVector[4];
	float spotDirection[4];
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
}typedef LightSource;
LightSource lightsource[3];

struct ObjModel
{
	struct TextureGroup
	{
		GLuint diffuseid;
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};

	struct IndexGroup
	{
		GLuint ibo;
		GLint matid;
		unsigned int numIndices;
	};

	struct Group
	{
		std::string name;
		unsigned int numIndices;
		GLuint vaoid;
		std::vector<GLint> matids;
		GLuint vboid[NUM_VBO];
		std::vector<IndexGroup> indexGroups;
	};

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials; // materials[0] 's texture => texture[0]

	std::vector<Group> groups;
	std::vector<TextureGroup> textures;
	fbx_handles myFbx;
	Transform transform;
	bool isFBX;

	void Load(std::string name);
};

struct FrameBufferObject
{
	int width;
	int height;
	int numPass;
	GLuint fb_vao;
	GLuint fbo; 
	GLuint depth_rbo;
	GLuint fbo_texs[NUM_PASS];
	GLuint fb_vbo;
	GLuint fb_shaderProgram;
	GLuint buffer;

	void init(GLuint program, int numPass);
	void reshape(int w, int h);
	void attach(GLuint attach);
	void bindFBO();
	void render(GLuint target);
private:
	void genPass();
};

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 15;
int wndWidth, wndHeight;
GLint mouse;
float forward_speed = 4.5f;

using namespace glm;

mat4 mvp;
GLint um4mvp;
GLint sampler;

GLuint program_scene;
GLuint program_fbo;
GLuint program_final;

// Shader attributes
GLint iLocPosition;
GLint iLocColor;
GLint iLocNormal;
GLint iLocLightsource[3][LIGHTSOURCE_NUM_PROPS];
GLint iLocMaterial[MATERIAL_NUM_PROPS];
GLint uLocMVP;
GLint uLocM;
GLint uLocV;
GLint uLocP;
GLint uLocN; // Transform normal to view space
GLint uLocE;
GLint uLocMode;

// FBO Shader attribute
GLint iLocFBSampler;
GLuint uFBLocRenderMode;
GLuint fFBLocTime;

// FinalFBO Shader
GLuint fLocMidRatio;

int lighting_mode = 0x2;
int render_mode = 0;

std::vector<ObjModel> models;
FrameBufferObject gFrameBufferObj, gFinalFBO;
int zombie_index = 0;
GLPerspectiveCamera gPerspectiveCam(60.0f, 0.3f, 3000.0f, (800 / 800));
struct { int x, y; } last{0, 0};

void setLightsource()
{
	for (int i = 0; i < 3; i++)
	{
		glUniform4fv(iLocLightsource[i][POS_AMBIENT], 1, lightsource[i].ambient);
		glUniform4fv(iLocLightsource[i][POS_DIFFUSE], 1, lightsource[i].diffuse);
		glUniform4fv(iLocLightsource[i][POS_SPECULAR], 1, lightsource[i].specular);
		glUniform4fv(iLocLightsource[i][POS_POSITION], 1, lightsource[i].position);
		glUniform4fv(iLocLightsource[i][POS_HALFVECTOR], 1, lightsource[i].halfVector);
		glUniform4fv(iLocLightsource[i][POS_SPOT_DIR], 1, lightsource[i].spotDirection);
		glUniform1fv(iLocLightsource[i][POS_SPOT_EXP], 1, &lightsource[i].spotExponent);
		glUniform1fv(iLocLightsource[i][POS_SPOT_CUTOFF], 1, &lightsource[i].spotCutoff);
		glUniform1fv(iLocLightsource[i][POS_SPOT_COSTCUTOFF], 1, &lightsource[i].spotCosCutoff);
		glUniform1fv(iLocLightsource[i][POS_CONSTANT_ATTENUATION], 1, &lightsource[i].constantAttenuation);
		glUniform1fv(iLocLightsource[i][POS_CONSTANT_LINEAR_ATTENUATION], 1, &lightsource[i].linearAttenuation);
		glUniform1fv(iLocLightsource[i][POS_CONSTANT_QUADRATIC_ATTENUATION], 1, &lightsource[i].quadraticAttenuation);
	}
}

void checkError(const char *functionName)
{
    GLenum error;
    while (( error = glGetError() ) != GL_NO_ERROR) {
        fprintf (stderr, "GL error 0x%X detected in %s\n", error, functionName);
    }
}

// Print OpenGL context related information.
void dumpInfo(void)
{
    printf("Vendor: %s\n", glGetString (GL_VENDOR));
    printf("Renderer: %s\n", glGetString (GL_RENDERER));
    printf("Version: %s\n", glGetString (GL_VERSION));
    printf("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
}

char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void freeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}

void shaderLog(GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf("%s\n", errorLog);
		delete errorLog;
	}
}

void My_Init()
{
	static const char *lightsource_loc_str[] =
	{
		"ambient",
		"diffuse",
		"specular",
		"position",
		"halfVector",
		"spotDirection",
		"spotExponent",
		"spotCutoff", // (range: [0.0,90.0], 180.0)
		"spotCosCutoff", // (range: [1.0,0.0],-1.0)
		"constantAttenuation",
		"linearAttenuation",
		"quadraticAttenuation"
	};

	static const char *material_loc_str[] =
	{
		"ambient",
		"diffuse",
		"specular",
		"shininess"
	};

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

    program_scene = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
	shaderLog(vertexShader);
    shaderLog(fragmentShader);
    glAttachShader(program_scene, vertexShader);
    glAttachShader(program_scene, fragmentShader);
    glLinkProgram(program_scene);
	um4mvp = glGetUniformLocation(program_scene, "um4mvp");
	sampler = glGetUniformLocation(program_scene, "sampler");
	glUniform1i(sampler, 0);
    glUseProgram(program_scene);

	program_fbo = glCreateProgram();
	GLuint vertexShader_fbo = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader_fbo = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource_fbo = loadShaderSource("vertex_fbo.vs.glsl");
	char** fragmentShaderSource_fbo = loadShaderSource("fragment_fbo.fs.glsl");
	glShaderSource(vertexShader_fbo, 1, vertexShaderSource_fbo, NULL);
	glShaderSource(fragmentShader_fbo, 1, fragmentShaderSource_fbo, NULL);
	freeShaderSource(vertexShaderSource_fbo);
	freeShaderSource(fragmentShaderSource_fbo);
	glCompileShader(vertexShader_fbo);
	glCompileShader(fragmentShader_fbo);
	shaderLog(vertexShader_fbo);
	shaderLog(fragmentShader_fbo);
	glAttachShader(program_fbo, vertexShader_fbo);
	glAttachShader(program_fbo, fragmentShader_fbo);
	glLinkProgram(program_fbo);
	iLocFBSampler = glGetUniformLocation(program_fbo, "tex");

	program_final = glCreateProgram();
	GLuint vertexShader_final = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader_final = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource_final = loadShaderSource("vertex_final.vs.glsl");
	char** fragmentShaderSource_final = loadShaderSource("fragment_final.fs.glsl");
	glShaderSource(vertexShader_final, 1, vertexShaderSource_final, NULL);
	glShaderSource(fragmentShader_final, 1, fragmentShaderSource_final, NULL);
	freeShaderSource(vertexShaderSource_final);
	freeShaderSource(fragmentShaderSource_final);
	glCompileShader(vertexShader_final);
	glCompileShader(fragmentShader_final);
	shaderLog(vertexShader_final);
	shaderLog(fragmentShader_final);
	glAttachShader(program_final, vertexShader_final);
	glAttachShader(program_final, fragmentShader_final);
	glLinkProgram(program_final);

	uLocMVP = glGetUniformLocation(program_scene, "mvp");
	uLocM = glGetUniformLocation(program_scene, "M");
	uLocV = glGetUniformLocation(program_scene, "V");
	uLocP = glGetUniformLocation(program_scene, "P");
	uLocN = glGetUniformLocation(program_scene, "N");
	uLocE = glGetUniformLocation(program_scene, "E");
	uLocMode = glGetUniformLocation(program_scene, "Mode");


	char str[256];
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < LIGHTSOURCE_NUM_PROPS; j++)
		{
			sprintf(str, "LightSource[%d].%s", i, lightsource_loc_str[j]);
			iLocLightsource[i][j] =
				glGetUniformLocation(program_scene, str);
		}
	}

	for (int i = 0; i < MATERIAL_NUM_PROPS; i++)
	{
		sprintf(str, "Material.%s", material_loc_str[i]);
		iLocMaterial[i] =
			glGetUniformLocation(program_scene, str);
	}
}

void My_LoadModels(const char *scene_file)
{
	static const char *zombies_fbxs[] = 
	{
		"zombie_fury.FBX",
		"zombie_walk.FBX",
		"zombie_dead.FBX"
	};

	models.resize(4);
	models[0].Load(scene_file);
	for (int i = 1; i < 4; i++)
	{
		models[i].Load(zombies_fbxs[i - 1]);
		models[i].transform.Rotate(vec3(270, 180, 0));
		models[i].transform.ScaleTo(vec3(10, 10, 10));
		models[i].transform.Translate(vec3(0, 100, 0));
	}
}

void My_InitFBO()
{
	gFrameBufferObj.init(program_fbo, NUM_PASS);
	uFBLocRenderMode = glGetUniformLocation(program_fbo, "renderMode");
	fFBLocTime = glGetUniformLocation(program_fbo, "elapsed_time");
	glUniform1i(uFBLocRenderMode, render_mode);

	gFinalFBO.init(program_final, NUM_PASS);
	fLocMidRatio = glGetUniformLocation(program_final, "midRatio");
	glUniform1f(fLocMidRatio, 0.5f);
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
	gFrameBufferObj.bindFBO();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program_scene);
	static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, black);

	setLightsource();

	for (int i = 0; i < models.size(); i++)
	{
		if (i > 0 && i != 1 + zombie_index)
			continue;

		ObjModel &model = models[i];

		glm::mat4 &m = model.transform.GetTransformMatrix();
		glm::mat4 v = gPerspectiveCam.GetViewMatrix();
		glm::mat4 p = gPerspectiveCam.GetProjectionMatrix();
		glm::mat4 &vp = gPerspectiveCam.GetViewProjectionMatrix();
		glm::mat4 n = transpose(inverse(v * m));

		glm::mat4 mvp = vp * m * mat4(1.0);
		glUniformMatrix4fv(uLocMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(uLocM, 1, GL_FALSE, glm::value_ptr(m));
		glUniformMatrix4fv(uLocV, 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(uLocP, 1, GL_FALSE, glm::value_ptr(p));
		glUniformMatrix4fv(uLocN, 1, GL_FALSE, glm::value_ptr(n));
		glUniform3fv(uLocE, 1, glm::value_ptr(gPerspectiveCam.GetTransform().Position()));
		glUniform1i(uLocMode, lighting_mode);

		glEnableVertexAttribArray(iLocPosition);
		glEnableVertexAttribArray(iLocColor);
		glEnableVertexAttribArray(iLocNormal);

		if (model.isFBX)
		{
			std::vector<tinyobj::shape_t> new_shapes;
			GetFbxAnimation(model.myFbx, new_shapes, (float)timer_cnt / 255.0f);
			for (int j = 0; j < new_shapes.size(); j++)
			{
				glBindBuffer(GL_ARRAY_BUFFER, model.groups[j].vboid[VBO_POS]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, new_shapes[j].mesh.positions.size() * sizeof(float), &new_shapes[j].mesh.positions[0]);
			}
		}

		for (int j = 0; j < model.groups.size(); j++)
		{
			ObjModel::Group group = model.groups[j];
			glBindVertexArray(group.vaoid);
			for (int k = 0; k < group.indexGroups.size(); k++)
			{
				GLint matid = group.indexGroups[k].matid;
				GLuint ibo = group.indexGroups[k].ibo;
				tinyobj::material_t mat = model.materials[matid];

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, model.textures[matid].diffuseid);

				glUniform4fv(iLocMaterial[POS_MAT_AMBIENT], 1, mat.ambient);
				glUniform4fv(iLocMaterial[POS_MAT_DIFFUSE], 1, mat.diffuse);
				glUniform4fv(iLocMaterial[POS_MAT_SPECULAR], 1, mat.specular);
				glUniform1f(iLocMaterial[POS_MAT_SHININESS], mat.shininess);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glDrawElements(GL_TRIANGLES, group.indexGroups[k].numIndices, GL_UNSIGNED_INT, 0);
			}
		}
	}

	glUseProgram(gFrameBufferObj.fb_shaderProgram);
	glUniform1f(fFBLocTime, glutGet(GLUT_ELAPSED_TIME) * 0.001);

	gFrameBufferObj.render(gFinalFBO.fbo);
	
	gFinalFBO.bindFBO();
	gFinalFBO.render(0);

    glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	wndWidth = width;
	wndHeight = height;

	gFrameBufferObj.reshape(wndWidth, wndHeight);
	gFinalFBO.reshape(wndWidth, wndHeight);
}

void My_Timer(int val)
{
	float time = glutGet(GLUT_ELAPSED_TIME);

	//glUniform1f(fFBLocTime, time * 0.1);

	float r = 500.0f;
	float scale = 0.001f;
	lightsource[POINT_LIGHT].position[0] = cos(time * scale) * r;
	lightsource[POINT_LIGHT].position[2] = sin(time * scale) * r;

	timer_cnt++;
	glutPostRedisplay();
	if(timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
	}
}

void My_Mouse(int button, int state, int x, int y)
{
	if(state == GLUT_DOWN)
	{
		printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
		mouse = button;
	}
	else if(state == GLUT_UP)
	{
		printf("Mouse %d is released at (%d, %d)\n", button, x, y);
		mouse = 0;
	}

	last.x = x;
	last.y = y;
	printf("(%d, %d) vs (%d, %d)\n",last.x, last.y, x, y);
}

void My_Passive_Mouse(int x, int y)
{
	static float unit = 2.5;

	vec2 diff(x - last.x, y - last.y);
	if (length(diff) > 0)
	{
		diff = normalize(diff);

		if(mouse == GLUT_LEFT_BUTTON)
			gPerspectiveCam.GetTransform().Rotate(vec3(diff.y, diff.x, 0.0) * unit);
		last.x = x;
		last.y = y;
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	
	if (key >= '0' && key <= '9')
	{
		render_mode ^= 1 << (int)(key - '0');
		glUniform1i(uFBLocRenderMode, render_mode);
	}

	Transform &t = gPerspectiveCam.GetTransform();
	switch (key)
	{
	case 's': case 'S':
		t.Translate(gPerspectiveCam.Forward() * forward_speed);
		break;
	case 'w': case 'W':
		t.Translate(gPerspectiveCam.Forward() * -forward_speed);
		break;
	case 'Z': case 'z':
		t.Translate(vec3(0, 1, 0) * forward_speed);
		break;
	case 'X': case 'x':
		t.Translate(vec3(0, 1, 0) * -forward_speed);
		break;
	case 'A': case 'a':
		t.Translate(gPerspectiveCam.Right() * forward_speed);
		break;
	case 'D': case 'd':
		t.Translate(gPerspectiveCam.Right() * -forward_speed);
		break;
	case 'N': case 'n':
		zombie_index++;
		zombie_index %= 3;
		break;
	default:
		break;
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	static float unit = 1.0f;
	Transform &t = models[zombie_index + 1].transform;
	switch(key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		//t.Rotate(vec3(0, 1, 0) * unit);
		t.Rotate(vec3(0, 5, 0));
		break;
	case GLUT_KEY_RIGHT:
		printf("Right arrow is pressed at (%d, %d)\n", x, y);
		//t.Rotate(vec3(0, 1, 0) * -unit);
		t.Rotate(vec3(0, -5, 0));
		break;
	case GLUT_KEY_UP:
		printf("Up arrow is pressed at (%d, %d)\n", x, y);
		//t.Rotate(vec3(1, 0, 0) * unit);
		t.Rotate(vec3(5, 0, 0));
		break;
	case GLUT_KEY_DOWN:
		printf("Down arrow is pressed at (%d, %d)\n", x, y);
		//t.Rotate(vec3(1, 0, 0) * -unit);
		t.Rotate(vec3(-5, 0, 0));
		break;
	case GLUT_KEY_CTRL_L:
		lighting_mode ^= 0x2;
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch(id)
	{
	case MENU_TIMER_START:
		if(!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	case MENU_ABSTRACTION:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 1);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 1);
		}
		break;
	case MENU_LAPLACIAN_FILTER:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 1);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 2);
		}
		break;
	case MENU_SHARPNESS_FILTER:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 1);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 3);
		}
		break;
	case MENU_PIXELATION:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 1);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 4);
		}
		break;
	case MENU_BLOOM:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 2);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 5);
		}
		break;
	case MENU_HALFTONE:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 1);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 6);
		}
		break;
	case MENU_RADIAL_BLUR:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 1);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 7);
		}
		break;
	case MENU_RIPPLE:
		{
			glUseProgram(gFinalFBO.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFinalFBO.fb_shaderProgram, "finalMode"), 1);
			glUseProgram(gFrameBufferObj.fb_shaderProgram);
			glUniform1i(glGetUniformLocation(gFrameBufferObj.fb_shaderProgram, "renderMode"), 8);
		}
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	wndWidth = 1024;
	wndHeight = 768;

	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(wndWidth, wndHeight);
	glutCreateWindow("Assignment 02 102062171"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
	glewInit();
	ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	dumpInfo();
	My_Init();
	My_LoadModels("sponza.obj");
	My_InitFBO();

	////////////////////

	// Create a menu and bind it to mouse right button.
	////////////////////////////
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAddMenuEntry("Abstraction", MENU_ABSTRACTION);
	glutAddMenuEntry("Laplacian Filter", MENU_LAPLACIAN_FILTER);
	glutAddMenuEntry("Sharpness Filter", MENU_SHARPNESS_FILTER);
	glutAddMenuEntry("Pixelation", MENU_PIXELATION);
	glutAddMenuEntry("Bloom", MENU_BLOOM);
	glutAddMenuEntry("Halftone", MENU_HALFTONE);
	glutAddMenuEntry("Radial Blur", MENU_RADIAL_BLUR);
	glutAddMenuEntry("Ripple", MENU_RIPPLE);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	////////////////////////////

	// Register GLUT callback functions.
	///////////////////////////////
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutMotionFunc(My_Passive_Mouse);
	glutTimerFunc(timer_speed, My_Timer, 0); 
	glClearColor(0,0,0,0);
	///////////////////////////////

	// Enter main event loop.
	//////////////
	gPerspectiveCam.GetTransform().TranslateTo(vec3(200, 150, 0));
	gPerspectiveCam.GetTransform().Rotate(vec3(0, -90, 0));
	SetArray4f(lightsource[DIR_LIGHT].ambient, 0.01, 0.01, 0.01, 1.0);
	SetArray4f(lightsource[DIR_LIGHT].diffuse, 0.5, 0.05, 0.3, 1.0);
	SetArray4f(lightsource[DIR_LIGHT].specular, 1.0, 1.0, 1.0, 1.0);
	SetArray4f(lightsource[DIR_LIGHT].position, 10.0, 10.0, 10.0, 0.0);

	SetArray4f(lightsource[POINT_LIGHT].ambient, 0.03, 0.005, 0.001, 1.0);
	SetArray4f(lightsource[POINT_LIGHT].diffuse, 255.0/255.0, 205.0/255.0, 148.0/255.0, 1.0);
	SetArray4f(lightsource[POINT_LIGHT].specular, 1.0, 1.0, 1.0, 1.0);
	SetArray4f(lightsource[POINT_LIGHT].position, 0.0, 500.0, 0.0, 1.0);
	lightsource[POINT_LIGHT].constantAttenuation = 0.0f;
	lightsource[POINT_LIGHT].linearAttenuation = 0.00002f;
	lightsource[POINT_LIGHT].quadraticAttenuation = 0.00001f;

	SetArray4f(lightsource[SPOT_LIGHT].ambient, 0.003, 0.005, 0.01, 0.0);
	SetArray4f(lightsource[SPOT_LIGHT].diffuse, 0.8, 0.8, 0.8, 0.0);
	SetArray4f(lightsource[SPOT_LIGHT].specular, 1.0, 1.0, 1.0, 0.0);
	SetArray4f(lightsource[SPOT_LIGHT].position, 0.0, 0.0, 50.0, 1.0);
	SetArray4f(lightsource[SPOT_LIGHT].spotDirection, 0.0, -0.0, -1.0, 0.0);
	lightsource[SPOT_LIGHT].spotExponent = 0.0001;
	lightsource[SPOT_LIGHT].spotCutoff = 5.0f;
	lightsource[SPOT_LIGHT].constantAttenuation = 0.0f;
	lightsource[SPOT_LIGHT].linearAttenuation = 0.0002f;
	lightsource[SPOT_LIGHT].quadraticAttenuation = 0.0005f;
	glutMainLoop();
	//////////////
	return 0;
}

void ObjModel::Load(std::string name)
{
	std::string err;

	// Check file extension
	isFBX = false;
	std::string fname(name);
	size_t dotPos = fname.find_last_of(".");
	if (dotPos != std::string::npos)
	{
		std::string ext_name = fname.substr(dotPos, fname.size() - dotPos);
		if (ext_name == ".fbx" || ext_name == ".FBX")
		{
			isFBX = true;
		}
	}

	bool ret = (isFBX) ? LoadFbx(myFbx, shapes, materials, err, name.c_str()) :
		tinyobj::LoadObj(shapes, materials, err, name.c_str());

	if (ret)
	{
		// Allocate texture group
		textures.resize(materials.size());

		// For Each Material
		for (int i = 0; i < materials.size(); i++)
		{
			// materials[i].diffuse_texname; // This is the Texture Path You Need
			ILuint ilTexName;
			ilGenImages(1, &ilTexName);
			ilBindImage(ilTexName);

			if (ilLoadImage(materials[i].diffuse_texname.c_str()) && ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
			{
				unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 4];
				ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGBA, IL_UNSIGNED_BYTE, data);
				
				glGenTextures(1, &textures[i].diffuseid);
				glBindTexture(GL_TEXTURE_2D, textures[i].diffuseid);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				delete[] data;

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glGenerateMipmap(GL_TEXTURE_2D);
				
				//printf("Texture: %s : %d\n", materials[i].diffuse_texname.c_str(), textures[i].diffuseid);
			}
			ilDeleteImages(1, &ilTexName);
		}

		// Allocate groups
		groups.resize(shapes.size());

		// For Each Shape (or Mesh, Object)
		for (int i = 0; i < shapes.size(); i++)
		{
			Group &group = groups[i];
		
			tinyobj::shape_t &shape = shapes[i];
			group.name = shape.name;
			bool hasNormal = shape.mesh.normals.size() > 0;
			bool hasTex = shape.mesh.texcoords.size() > 0;

			if (!hasNormal)
				shape.mesh.normals.assign(shape.mesh.positions.size(), 0);
			if (!hasTex)
				shape.mesh.normals.assign(shape.mesh.positions.size() - shape.mesh.positions.size() / 3, 0);

			group.matids = shape.mesh.material_ids;
			group.numIndices = shape.mesh.indices.size();
			
			glGenVertexArrays(1, &group.vaoid);
			glBindVertexArray(group.vaoid);

			glGenBuffers(1, &group.vboid[VBO_POS]);
			glBindBuffer(GL_ARRAY_BUFFER, group.vboid[VBO_POS]);
			glBufferData(GL_ARRAY_BUFFER, shape.mesh.positions.size() * sizeof(float), &shape.mesh.positions[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(POSITION_LOC);
			glVertexAttribPointer(POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glGenBuffers(1, &group.vboid[VBO_NORMAL]);
			glBindBuffer(GL_ARRAY_BUFFER, group.vboid[VBO_NORMAL]);
			glBufferData(GL_ARRAY_BUFFER, shape.mesh.normals.size() * sizeof(float), &shape.mesh.normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(NORMAL_LOC);
			glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glGenBuffers(1, &group.vboid[VBO_TEXCOORD]);
			glBindBuffer(GL_ARRAY_BUFFER, group.vboid[VBO_TEXCOORD]);
			glBufferData(GL_ARRAY_BUFFER, shape.mesh.texcoords.size() * sizeof(float), &shape.mesh.texcoords[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(TEXCOORD_LOC);
			glVertexAttribPointer(TEXCOORD_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

			std::unordered_map<GLint, std::vector<unsigned int>> indexGroupMap;
			for (int j = 0; j < shape.mesh.indices.size() / 3; j++)
			{
				GLint matid = shape.mesh.material_ids[j];
				std::vector<unsigned int> &indexGroup = indexGroupMap[matid];
				indexGroup.push_back(shape.mesh.indices[j * 3]);
				indexGroup.push_back(shape.mesh.indices[j * 3 + 1]);
				indexGroup.push_back(shape.mesh.indices[j * 3 + 2]);
			}
			

			int igIndex = 0;
			group.indexGroups.resize(indexGroupMap.size());
			for (std::unordered_map<GLint, std::vector<unsigned int>>::iterator it = indexGroupMap.begin();
				it != indexGroupMap.end();
				it++)
			{
				group.indexGroups[igIndex].matid = it->first;
				group.indexGroups[igIndex].numIndices = it->second.size();
				glGenBuffers(1, &group.indexGroups[igIndex].ibo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group.indexGroups[igIndex].ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, it->second.size() * sizeof(unsigned int), &it->second[0], GL_STATIC_DRAW);
				igIndex++;
			}
			glBindVertexArray(0);
			
		}
	}
	else
	{
		fprintf(stderr, "Load(): error\n");
	}
}

void FrameBufferObject::init(GLuint program, int _numPass)
{
	fb_shaderProgram = program;
	numPass = _numPass;

	glGenVertexArrays(1, &fb_vao); 
	glBindVertexArray(fb_vao);

	glGenBuffers(1, &fb_vbo); 
	glBindBuffer(GL_ARRAY_BUFFER, fb_vbo); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertex), window_vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0); 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0); 
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glGenFramebuffers(1, &fbo); //Create FBO
	glGenRenderbuffers(1, &depth_rbo); //Create Depth RBO 
	glBindRenderbuffer( GL_RENDERBUFFER, depth_rbo ); 
	glRenderbufferStorage( GL_RENDERBUFFER,GL_DEPTH_COMPONENT32, wndWidth, wndHeight);

	genPass();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBufferObject::reshape(int w, int h)
{
	width = w;
	height = h;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glDeleteTextures(numPass, fbo_texs);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);

	genPass();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBufferObject::attach(GLuint attach)
{
	//which render buffer attachment is written 
	glDrawBuffer(attach);
	glViewport( 0, 0, wndWidth, wndHeight);
}

void FrameBufferObject::bindFBO()
{
	//Bind FBO 
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glDrawBuffers(numPass, draw_buffers);
}

void FrameBufferObject::render(GLuint target)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glViewport(0, 0, wndWidth, wndHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	//Draw Final Rectangle Texture 
	glBindVertexArray(fb_vao);
	glUseProgram(fb_shaderProgram);
	for (int i = 0; i < numPass; i++)
	{
		char buff[64];
		sprintf(buff, "tex%d", i);
		glActiveTexture(GL_TEXTURE0 + i);
		glUniform1i(glGetUniformLocation(fb_shaderProgram, buff), i);
		glBindTexture(GL_TEXTURE_2D, fbo_texs[i]);
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void FrameBufferObject::genPass()
{
	glGenTextures(numPass, fbo_texs); //Create fobDataTexture
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	//Set depthrbo to current fbo 
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);
	for (int i = 0; i < numPass; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, fbo_texs[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wndWidth, wndHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Set buffertexture to current fbo
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, draw_buffers[i], GL_TEXTURE_2D, fbo_texs[i], 0);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
