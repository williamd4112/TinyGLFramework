#include "tiny_gl.h"
#include "tiny_ctrl.h"

using namespace tiny_gl;
using namespace tiny_ctrl;

static GLubyte timer_cnt = 0;
static bool timer_enabled = true;
static unsigned int timer_speed = 30;
static int wndWidth, wndHeight;

static GLShaderProgram gSceneShaderProg;
static GLShaderProgram gShadowShaderProg;
static GLFrameBuffer gShadowFrameBuffer;
static GLScene gScene;
static MouseControl gMouse;

static void Display();
static void Reshape(int w, int h);
static void Timer(int val);
static void Keyboard(unsigned char key, int x, int y);
static void Mouse(int button, int state, int x, int y);
static void PassiveMouse(int x, int y);

static void RenderSponza(const GLShaderProgram &, const GLScene &);
static void RenderShadow(const GLScene &, glm::mat4 & lightSpaceMatrix);
static void RenderFBO(const GLShaderProgram &, const GLFrameBufferObject &);
static void SetupShaderProgram();
static void SetupScene();

int main(int argc, char *argv[])
{
	wndWidth = 800;
	wndHeight = 800;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(wndWidth, wndHeight);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glutCreateWindow("tiny_gl"); 
	glewInit();
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	DumpInfo();

	SetupShaderProgram();
	SetupScene();

	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutTimerFunc(timer_speed, Timer, 0);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(PassiveMouse);
	
	glutMainLoop();

	return 0;
}

void Display()
{
	RenderSponza(gSceneShaderProg, gScene);
	glutSwapBuffers();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	wndWidth = w;
	wndHeight = h;

	gScene.GetCamera<GLPerspectiveCamera>().SetAspect((float)w / (float)h);
}

void Timer(int val)
{
	float time = glutGet(GLUT_ELAPSED_TIME);
	time *= 0.0004;
	float r = 500.0;
	float scale = 0.008f;

	timer_cnt++;
	glutPostRedisplay();
	if (timer_enabled)
	{
		gScene.Update((float)timer_cnt / 255.0);
		GLLight & light = gScene.GetGLLight(1);
		glm::vec3 v;
		v.x = glm::cos(time) * r;
		v.y = gScene.GetCamera().GetTransform().Position().y;
		v.z = (glm::sin(time) * r);

		light.SetPosition(glm::vec4(v, 1.0));
		glutTimerFunc(timer_speed, Timer, val);
	}
}

void Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);

	float forward_speed = 3.0f;
	Transform &t = gScene.GetCamera().GetTransform();
	glm::vec3 forward = gScene.GetCamera().Forward();
	glm::vec3 right = gScene.GetCamera().Right();
	switch (key)
	{
	case 's': case 'S':
		t.Translate(forward * forward_speed);
		break;
	case 'w': case 'W':
		t.Translate(forward * -forward_speed);
		break;
	case 'Z': case 'z':
		t.Translate(glm::vec3(0, 1, 0) * forward_speed);
		break;
	case 'X': case 'x':
		t.Translate(glm::vec3(0, 1, 0) * -forward_speed);
		break;
	case 'A': case 'a':
		t.Translate(right * forward_speed);
		break;
	case 'D': case 'd':
		t.Translate(right * -forward_speed);
		break;
	default:
		break;
	}
}

void Mouse(int button, int state, int x, int y)
{
	gMouse.Update(button, state, x, y);
}

void PassiveMouse(int x, int y)
{
	static float unit = 2.5;

	glm::vec2 diff = gMouse.Diff();
	if (length(diff) > 0)
	{
		diff = normalize(diff);

		if (gMouse.Button() == GLUT_LEFT_BUTTON)
			gScene.GetCamera().GetTransform().Rotate(glm::vec3(diff.y, diff.x, 0.0) * unit);
		gMouse.Update(x, y);
	}
}

void RenderSponza(const GLShaderProgram & program, const GLScene & scene)
{
#define MATERIAL_NUM_PROPS 4
#define POS_MAT_AMBIENT 0
#define POS_MAT_DIFFUSE 1
#define POS_MAT_SPECULAR 2
#define POS_MAT_SHININESS 3

	static const char *material_loc_str[] =
	{
		"ambient",
		"diffuse",
		"specular",
		"shininess"
	};

	const GLLight & directionalLight = scene.GetGLLight(0);
	glm::vec3 pos(directionalLight.GetParms().position[0],
		directionalLight.GetParms().position[1],
		directionalLight.GetParms().position[2]);

	GLfloat near_plane = 1.0f, far_plane = 7.5f;
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(pos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	glCullFace(GL_FRONT);
	RenderShadow(gScene, lightSpaceMatrix);
	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, wndWidth, wndHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLCamera & camera = scene.GetCamera();

	scene.GetSkybox().Render(camera.GetViewMatrix());	

	program.Use();
	glUniformMatrix4fv(program.GetLocation("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform1i(program.GetLocation("depthmap"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gShadowFrameBuffer.GetTexture(0));

	GLuint uLocMVP = program.GetLocation("mvp");
	GLuint uLocM = program.GetLocation("M");
	GLuint uLocV = program.GetLocation("V");
	GLuint uLocP = program.GetLocation("P");
	GLuint uLocN = program.GetLocation("N");
	GLuint uLocE = program.GetLocation("E");
	GLuint iLocMaterial[MATERIAL_NUM_PROPS];
	
	char str[256];
	for (int i = 0; i < MATERIAL_NUM_PROPS; i++)
	{
		sprintf(str, "Material.%s", material_loc_str[i]);
		iLocMaterial[i] = program.GetLocation(str);
	}

	const std::vector<GLObject *> & objects = scene.GetObjects();
	
	auto lights = scene.GetLights();
	for (int i = 0; i < lights.size(); i++)
	{
		SetLightSource(program, "LightSource", i, scene.GetGLLight(i));
	}

	for (int i = 0; i < objects.size(); i++)
	{
		GLTangentMeshObject * model = static_cast<GLTangentMeshObject *>((objects[i]));

		glm::mat4 &m = model->GetTransform().GetTransformMatrix();
		glm::mat4 v = camera.GetViewMatrix();
		glm::mat4 p = camera.GetProjectionMatrix();
		glm::mat4 &vp = camera.GetViewProjectionMatrix();
		glm::mat4 n = transpose(inverse(v * m));

		glm::mat4 mvp = vp * m * glm::mat4(1.0);
		glUniformMatrix4fv(uLocMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(uLocM, 1, GL_FALSE, glm::value_ptr(m));
		glUniformMatrix4fv(uLocV, 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(uLocP, 1, GL_FALSE, glm::value_ptr(p));
		glUniformMatrix4fv(uLocN, 1, GL_FALSE, glm::value_ptr(n));
		glUniform3fv(uLocE, 1, glm::value_ptr(camera.GetTransform().Position()));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		const GLMesh & mesh = model->Mesh();
		const std::vector<tiny_gl::GLMesh::group_t> & groups = mesh.Groups();
		for (int j = 0; j < groups.size(); j++)
		{
			const tiny_gl::GLMesh::group_t & group = groups[j];

			glBindVertexArray(group.vaoid);
			for (int k = 0; k < group.indexGroups.size(); k++)
			{
				GLint matid = group.indexGroups[k].matid;
				GLuint ibo = group.indexGroups[k].ibo;
				tinyobj::material_t mat = mesh.Materials()[matid];

				glUniform1i(program.GetLocation("sampler"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mesh.Textures()[matid].diffuseid);

				glUniform1i(program.GetLocation("normalmap"), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, mesh.Textures()[matid].bumpid);

				glUniform4fv(iLocMaterial[POS_MAT_AMBIENT], 1, mat.ambient);
				glUniform4fv(iLocMaterial[POS_MAT_DIFFUSE], 1, mat.diffuse);
				glUniform4fv(iLocMaterial[POS_MAT_SPECULAR], 1, mat.specular);
				glUniform1f(iLocMaterial[POS_MAT_SHININESS], mat.shininess);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glDrawElements(GL_TRIANGLES, group.indexGroups[k].numIndices, GL_UNSIGNED_INT, 0);
			}
		}
	}
}

void RenderShadow(const GLScene & scene, glm::mat4 & lightSpaceMatrix)
{
	gShadowShaderProg.Use();

	glUniformMatrix4fv(gShadowShaderProg.GetLocation("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, wndWidth, wndHeight);
	gShadowFrameBuffer.Bind();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const std::vector<GLObject *> & objects = scene.GetObjects();
	for (int i = 0; i < objects.size(); i++)
	{
		GLTangentMeshObject * model = static_cast<GLTangentMeshObject *>((objects[i]));

		glm::mat4 &m = model->GetTransform().GetTransformMatrix();
		glUniformMatrix4fv(gShadowShaderProg.GetLocation("model"), 1, GL_FALSE, glm::value_ptr(m));

		glEnableVertexAttribArray(0);

		const GLMesh & mesh = model->Mesh();
		const std::vector<tiny_gl::GLMesh::group_t> & groups = mesh.Groups();
		for (int j = 0; j < groups.size(); j++)
		{
			const tiny_gl::GLMesh::group_t & group = groups[j];

			glBindVertexArray(group.vaoid);
			for (int k = 0; k < group.indexGroups.size(); k++)
			{
				GLuint ibo = group.indexGroups[k].ibo;

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glDrawElements(GL_TRIANGLES, group.indexGroups[k].numIndices, GL_UNSIGNED_INT, 0);
			}
		}
	}

	gShadowFrameBuffer.Unbind();
}


void RenderFBO(const GLShaderProgram & program, const GLFrameBufferObject & fbo)
{
	std::vector<GLuint> textures = fbo.GetTextures();
	for (int i = 0; i < textures.size(); i++)
	{
		char buff[64];
		sprintf(buff, "tex%d", i);
		glActiveTexture(GL_TEXTURE0 + i);
		glUniform1i(program.GetLocation(buff), i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
	}
	
	fbo.BindVAO();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void SetupShaderProgram()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gSceneShaderProg = GLShaderProgram::LoadWithSeries(SCENE, "scene");
	gSceneShaderProg.Use();
	gSceneShaderProg.SetRenderFunction(RenderSponza);

	gShadowShaderProg = GLShaderProgram::LoadWithSeries(SCENE, "shadow");

	gShadowFrameBuffer.Init(GLFrameBuffer::DEPTH, 1, wndWidth, wndHeight);
	
}

void SetupScene()
{
	gScene.CreatePerspectiveCamera(60.0f, 0.3f, 3000.0f, (800 / 800));
	gScene.CreateTangentMeshObject("sponza.obj");

	gScene.CreateDirectionalLight(
		glm::vec3(1.0, 1.0, 1.0), 
		glm::vec3(255.0 / 255.0, 205.0 / 255.0, 148.0 / 255.0),
		glm::vec3(255.0 / 255.0, 205.0 / 255.0, 148.0 / 255.0),
		glm::vec3(1.0, 1.0, 1.0));

	gScene.CreatePointLight(
		glm::vec3(0.0, 500.0, 0.0),
		glm::vec3(0.03, 0.005, 0.001),
		glm::vec3(255.0 / 255.0, 205.0 / 255.0, 148.0 / 255.0),
		glm::vec3(1.0, 1.0, 1.0),
		0.0f,
		0.001f,
		0.0f);

	gScene.GetCamera().GetTransform().TranslateTo(glm::vec3(0, 600, 0));

	const char *skybox_filenames[6] = {
		"mountaincube_02.jpg",
		"mountaincube_01.jpg",
		"mountaincube_04.jpg",
		"mountaincube_03.jpg",
		"mountaincube_05.jpg",
		"mountaincube_06.jpg"
	};

	gScene.CreateSkybox("mountaincube.png");
}