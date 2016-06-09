#include "tiny_gl.h"
#include "tiny_ctrl.h"

using namespace tiny_gl;
using namespace tiny_ctrl;

static struct {
	int enable_normalmap;
	bool use_lightview;
	bool enable_msaa;
	int shade_normal;
	float dirLight_far;
	int shadow_width;
	int shadow_height;
} gSceneShaderSetting{ 1, false, false, 0, 10000.0f, 8192, 8192 };

static struct
{
	GLuint waterDuDv;
	float waterPlaneHeight;
} gWaterConfig{ 0, 60.0f };

static GLubyte timer_cnt = 0;
static bool timer_enabled = true;
static unsigned int timer_speed = 30;
static int wndWidth, wndHeight;

static GLShaderProgram gSceneShaderProg;
static GLShaderProgram gReflectProg;
static GLShaderProgram gWaterProg;
static GLShaderProgram gGlowProg;
static GLShaderProgram gShadowShaderProg;
static GLShaderProgram gOminiShadowShaderProg;
static GLShaderProgram gShadowVisualizerProg;
static GLFrameBuffer gShadowFrameBuffer;
static GLFrameBuffer gOminiShadowFrameBuffer;
static GLFrameBufferObject gShadowVisualizer;
static GLFrameBufferObject gReflectionFrameBuffer;
static GLFrameBufferObject gGlowFrameBuffer;
static GLFrameBufferObject gWaterFrameBuffer;

static GLScene gScene;
static MouseControl gMouse;

static void Display();
static void Reshape(int w, int h);
static void Timer(int val);
static void Keyboard(unsigned char key, int x, int y);
static void SpecialKeyboard(int key, int x, int y);
static void Mouse(int button, int state, int x, int y);
static void PassiveMouse(int x, int y);

static void RenderSponza(const GLShaderProgram &, const GLScene &, int reflect);
static void RenderShadow(const GLScene &, glm::mat4 & lightSpaceMatrix);
static void RenderFBO(const GLShaderProgram &, const GLFrameBufferObject &);
static void RenderWaterFBO(const GLShaderProgram & program, const GLFrameBufferObject & fbo);
static void RenderReflection(const GLShaderProgram & program, const GLScene & scene);
static void RenderDebugDepthMap();
static glm::mat4 CalculateLightMatrix(const GLLight & directionalLight);
static void SetupWaterTexture();
static void BindScreenFrameBuffer();

static void SetupShaderProgram();
static void SetupScene();
static void SetupAA(bool enable);

int main(int argc, char *argv[])
{
	wndWidth = 1024;
	wndHeight = 768;

	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
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
	glutSpecialFunc(SpecialKeyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(PassiveMouse);
	
	glutMainLoop();

	return 0;
}

void Display()
{
	GLFrameBufferObject screenFbo(wndWidth, wndHeight);

	// Setup AA
	SetupAA(gSceneShaderSetting.enable_msaa);

	// Directional light shadow
	const GLLight & directionalLight = gScene.GetGLLight(0);
	glm::mat4 lightSpaceMatrix = CalculateLightMatrix(directionalLight);

	// Render shadow map
	RenderShadow(gScene, lightSpaceMatrix);

	// Render reflection
	gReflectionFrameBuffer.BindFBO();
	gReflectProg.Render(gScene);

	gWaterFrameBuffer.BindFBO();
	RenderSponza(gSceneShaderProg, gScene, 0);

	gGlowFrameBuffer.BindFBO();
	gWaterProg.Render(gWaterFrameBuffer);

	screenFbo.BindFBO();
	gGlowProg.Render(gGlowFrameBuffer);

	//GLFrameBufferObject screenFbo(wndWidth, wndHeight);
	//screenFbo.BindFBO();
	/*gWaterProg.Render(gWaterFrameBuffer);*/

	////gGlowFrameBuffer.BindFBO();
	//gWaterProg.Render(gWaterFrameBuffer);
	////GLFrameBufferObject screenFbo(wndWidth, wndHeight);
	////screenFbo.BindFBO();
	////gGlowProg.Render(gGlowFrameBuffer);

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
	time *= 0.00008;
	float r = 150.0;
	float scale = 0.01f;

	timer_cnt++;
	glutPostRedisplay();
	if (timer_enabled)
	{
		gScene.Update((float)timer_cnt / 255.0);
		
		GLLight & light = gScene.GetGLLight(0);
		glm::vec3 v;
		v.z = 1000.0f + (cos(time)) * 3000.0f ;
		v.x = 0.001; //sin(time) * r;
		v.y = 3000.0;// +fabs(sin(time)) * 2500.0f;

		light.SetPosition(glm::vec4(v, 0.0));

		glutTimerFunc(timer_speed, Timer, val);
	}
}

void Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);

	float forward_speed = 10.0f;
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
	case 'L': case 'l':
		gSceneShaderSetting.use_lightview = !gSceneShaderSetting.use_lightview;
		break;
	case '0':
		gSceneShaderSetting.enable_normalmap ^= 0x1;
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
	static float unit = 1.25;

	glm::vec2 diff = gMouse.Diff();
	if (length(diff) > 0)
	{
		diff = normalize(diff);

		if (gMouse.Button() == GLUT_LEFT_BUTTON)
			gScene.GetCamera().GetTransform().RotateEuler(glm::vec3(diff.y, 0, 0) * unit);
		if (gMouse.Button() == GLUT_RIGHT_BUTTON)
			gScene.GetCamera().GetTransform().RotateEuler(glm::vec3(diff.y, diff.x, 0) * unit);
		auto v = gScene.GetCamera().GetTransform().EulerAngle();
		printf("%f , %f , %f\n", v.x, v.y, v.z);
		gMouse.Update(x, y);
	}
}

void RenderSponza(const GLShaderProgram & program, const GLScene & scene, int reflect)
{
#define MATERIAL_NUM_PROPS 4
#define POS_MAT_AMBIENT 0
#define POS_MAT_DIFFUSE 1
#define POS_MAT_SPECULAR 2
#define POS_MAT_SHININESS 3
#define DIFFUSE_TEX GL_TEXTURE0
#define NORMAL_TEX GL_TEXTURE

	static const char *material_loc_str[] =
	{
		"ambient",
		"diffuse",
		"specular",
		"shininess"
	};

	// Directional light shadow
	const GLLight & directionalLight = gScene.GetGLLight(0);
	glm::mat4 lightSpaceMatrix = CalculateLightMatrix(directionalLight);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//gWaterFrameBuffer.BindFBO();
	glViewport(0, 0, wndWidth, wndHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLCamera & camera = scene.GetCamera();
	
	// Draw skybox
	//scene.GetSkybox().Render(camera.GetTransform().Position(), camera.GetViewMatrix(), camera.GetProjectionMatrix());

	// Render scene
	program.Use();
	
	// Setup shading mode
	glUniform1i(program.GetLocation("Mode"), gSceneShaderSetting.shade_normal);
	glUniformMatrix4fv(program.GetLocation("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform1i(program.GetLocation("isReflect"), reflect);
	glUniform1f(program.GetLocation("waterHeight"), gWaterConfig.waterPlaneHeight);
	
	// Shadow map
	glUniform1i(program.GetLocation("depthmap"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gShadowFrameBuffer.GetTexture(0));

	GLuint uLocMVP = program.GetLocation("mvp");
	GLuint uLocM = program.GetLocation("M");
	GLuint uLocV = program.GetLocation("V");
	GLuint uLocP = program.GetLocation("P");
	GLuint uLocN = program.GetLocation("N");
	GLuint uLocE = program.GetLocation("E");
	GLuint uLocObjVariant = program.GetLocation("objectVariant");
	GLuint uLocGlowIntensity = program.GetLocation("glowIntensity");
	GLuint iLocMaterial[MATERIAL_NUM_PROPS];
	char str[256];
	for (int i = 0; i < MATERIAL_NUM_PROPS; i++)
	{
		sprintf(str, "Material.%s", material_loc_str[i]);
		iLocMaterial[i] = program.GetLocation(str);
	}
	
	// Setup lightsource
	auto lights = scene.GetLights();
	for (int i = 0; i < lights.size(); i++)
	{
		SetLightSource(program, "LightSource", i, scene.GetGLLight(i));
	}

	glm::mat4 v = camera.GetViewMatrix();
	glm::mat4 p = camera.GetProjectionMatrix();
	glUniform3fv(uLocE, 1, glm::value_ptr(camera.GetTransform().Position()));

	// Render skybox
	glUniformMatrix4fv(uLocV, 1, GL_FALSE, glm::value_ptr(v));
	glUniformMatrix4fv(uLocP, 1, GL_FALSE, glm::value_ptr(p));
	glUniform1i(uLocObjVariant, -1);
	GLSkybox & skybox = gScene.GetSkybox();
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE5);
	glUniform1i(program.GetLocation("skybox"), 5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.GetTextureID());
	skybox.BindVAO();
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);

	const std::vector<GLObject *> & objects = scene.GetObjects();
	for (int i = 0; i < objects.size(); i++)
	{
		GLMeshObject * model = static_cast<GLMeshObject *>((objects[i]));

		glm::mat4 &m = model->GetTransform().GetTransformMatrix();
		glm::mat4 mvp = p * v * m * glm::mat4(1.0);
		glUniformMatrix4fv(uLocMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(uLocM, 1, GL_FALSE, glm::value_ptr(m));
		glUniformMatrix4fv(uLocV, 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(uLocP, 1, GL_FALSE, glm::value_ptr(p));
		glUniform1i(uLocObjVariant, model->GetVariant());

		const GLMesh & mesh = model->Mesh();
		const std::vector<tiny_gl::GLMesh::group_t> & groups = mesh.Groups();
		const std::vector<float> & glowCoffs = mesh.GlowCoffs();

		for (int j = 0; j < groups.size(); j++)
		{
			const tiny_gl::GLMesh::group_t & group = groups[j];

			glBindVertexArray(group.vaoid);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);

			// Glow coff
			glUniform1f(uLocGlowIntensity, glowCoffs[j]);

			for (int k = 0; k < group.indexGroups.size(); k++)
			{
				GLint matid = group.indexGroups[k].matid;
				GLuint ibo = group.indexGroups[k].ibo;
				const tinyobj::material_t & mat = mesh.Materials()[matid];

				// Diffuse map
				glUniform1i(program.GetLocation("sampler"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mesh.Textures()[matid].diffuseid);

				// Normal map
				if(mesh.Textures()[matid].bumpid)
				{
					glUniform1i(program.GetLocation("enable_normalmap"), gSceneShaderSetting.enable_normalmap);
					glUniform1i(program.GetLocation("normalmap"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, mesh.Textures()[matid].bumpid);
				}
				else
				{
					glUniform1i(program.GetLocation("enable_normalmap"), 0);
				}

				// Specular map
				if (mesh.Textures()[matid].specularid)
				{
					glUniform1i(program.GetLocation("enable_specularMap"), 0);
					glUniform1i(program.GetLocation("specularmap"), 4);
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, mesh.Textures()[matid].specularid);
				}
				else
				{
					glUniform1i(program.GetLocation("enable_specularMap"), 0);
				}

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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	gShadowShaderProg.Use();

	glUniformMatrix4fv(gShadowShaderProg.GetLocation("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, gSceneShaderSetting.shadow_width, gSceneShaderSetting.shadow_height);

	gShadowFrameBuffer.Bind();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const std::vector<GLObject *> & objects = scene.GetObjects();
	for (int i = 0; i < objects.size(); i++)
	{
		GLMeshObject * model = static_cast<GLMeshObject *>((objects[i]));

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
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
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

void RenderWaterFBO(const GLShaderProgram & program, const GLFrameBufferObject & fbo)
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

	glActiveTexture(GL_TEXTURE3);
	glUniform1i(program.GetLocation("tex3"), 3);
	glBindTexture(GL_TEXTURE_2D, gReflectionFrameBuffer.GetTextures()[0]);

	glActiveTexture(GL_TEXTURE4);
	glUniform1i(program.GetLocation("waterDuDv"), 4);
	glBindTexture(GL_TEXTURE_2D, gWaterConfig.waterDuDv);

	GLuint uLocT = program.GetLocation("time");
	float time = glutGet(GLUT_ELAPSED_TIME);
	glUniform1f(uLocT, time * 0.00003);

	fbo.BindVAO();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void RenderDebugDepthMap()
{
	static const GLfloat window_vertex[] = {
		//vec2 position vec2 texture_coord 
		1.0f,-1.0f,1.0f,0.0f,
		-1.0f,-1.0f,0.0f,0.0f,
		-1.0f,1.0f,0.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f
	};

	gShadowVisualizerProg.Use();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, wndWidth / 4, wndHeight / 4);
	glDisable(GL_DEPTH_TEST);
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertex), window_vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glUniform1i(gShadowVisualizerProg.GetLocation("depthMap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gShadowFrameBuffer.GetTexture(0));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

glm::mat4 CalculateLightMatrix(const GLLight & directionalLight)
{
	glm::vec3 pos(directionalLight.GetParms().position[0],
		directionalLight.GetParms().position[1],
		directionalLight.GetParms().position[2]);
	GLfloat near_plane = 1.0f, far_plane = gSceneShaderSetting.dirLight_far;
	glm::mat4 lightProjection = glm::ortho(-far_plane / 2.0f, far_plane / 2.0f, -far_plane / 2.0f, far_plane / 2.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(pos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
	return lightProjection * lightView;
}

void SetupAA(bool enable)
{
	if (enable)
	{
		glEnable(GL_MULTISAMPLE);
		glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
	}
}

void RenderReflection(const GLShaderProgram & program, const GLScene & scene)
{
	static float gWaterPlaneHeight = 5.0f;

	glEnable(GL_CLIP_DISTANCE0);

	GLCamera & camera = scene.GetCamera();

	float dis = 2 * (camera.GetTransform().Position().y - (gWaterPlaneHeight));

	glm::vec3 rotate = camera.GetTransform().EulerAngle();
	float angle = rotate.x * 57.2957795;

	camera.GetTransform().Translate(glm::vec3(0, -dis, 0));
	camera.GetTransform().RotateEuler(glm::vec3(-2 * angle, 0, 0));

	RenderSponza(program, scene, 1);

	camera.GetTransform().RotateEuler(glm::vec3(2 * angle, 0, 0));
	camera.GetTransform().Translate(glm::vec3(0, dis, 0));

	glDisable(GL_CLIP_DISTANCE0);
}


void SetupShaderProgram()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gSceneShaderProg = GLShaderProgram::LoadWithSeries(SCENE, "scene");
	gSceneShaderProg.Use();

	gShadowShaderProg = GLShaderProgram::LoadWithSeries(SCENE, "shadow");
	gShadowFrameBuffer.Init(GLFrameBuffer::DEPTH, 1, gSceneShaderSetting.shadow_width, gSceneShaderSetting.shadow_height);
	gOminiShadowShaderProg = GLShaderProgram::LoadWithSeries(SCENE, "shadow_omini", BIT_VS | BIT_FS | BIT_GS);
	gOminiShadowFrameBuffer.Init(GLFrameBuffer::DEPTH_CUBE, 1, gSceneShaderSetting.shadow_width, gSceneShaderSetting.shadow_height);
	
	gShadowVisualizer.Init(1, gSceneShaderSetting.shadow_width, gSceneShaderSetting.shadow_height);
	gShadowVisualizerProg = GLShaderProgram::LoadWithSeries(SCENE, "depth", BIT_VS | BIT_FS);

	SetupWaterTexture();
	gReflectProg = GLShaderProgram::LoadWithSeries(SCENE, "scene");
	gReflectProg.SetRenderFunction(RenderReflection);
	gReflectionFrameBuffer.Init(4, wndWidth, wndHeight);

	gWaterProg = GLShaderProgram::LoadWithSeries(FBO, "water");
	gWaterProg.SetFrameBufferRenderFunction(RenderWaterFBO);
	gWaterFrameBuffer.Init(4, wndWidth, wndHeight);

	gGlowProg = GLShaderProgram::LoadWithSeries(FBO, "glow");
	gGlowProg.SetFrameBufferRenderFunction(RenderFBO);
	gGlowFrameBuffer.Init(3, wndWidth, wndHeight);
}

void SetupScene()
{
	gScene.CreatePerspectiveCamera(60.0f, 0.3f, 10000.0f, (wndWidth / wndHeight));
	gScene.CreateTangentMeshObject("sponza.obj");
	//int knight_id = gScene.CreateTangentMeshObject("knight.obj");
	//gScene.GetGLObject(knight_id).GetTransform().ScaleTo(glm::vec3(10.0, 10.0, 10.0));
	//gScene.GetGLObject(knight_id).GetTransform().TranslateTo(glm::vec3(.0, 1500.0, 0.0));

	int pnum = gScene.CreateTangentMeshObject("WaterPlane.obj");
	GLObject & p = gScene.GetGLObject(pnum);
	p.SetVariant(1);
	p.GetTransform().Scale(glm::vec3(100.0, 1.0, 100.0));
	p.GetTransform().Translate(glm::vec3(0.0, gWaterConfig.waterPlaneHeight, 0.0));

	gScene.CreateDirectionalLight(
		glm::vec3(0.01f, 5000.0f, 0.0f), 
		glm::vec3(255.0 / 255.0, 205.0 / 255.0, 148.0 / 255.0),
		colorRGB(255.0, 255.0, 255.0),
		colorRGB(255.0, 255.0, 255.0));

	gScene.CreatePointLight(
		glm::vec3(0.0, 500.0, 0.0),
		glm::vec3(255.0 / 255.0, 205.0 / 255.0, 148.0 / 255.0),
		glm::vec3(255.0 / 255.0, 205.0 / 255.0, 148.0 / 255.0),
		glm::vec3(255.0 / 255.0, 205.0 / 255.0, 148.0 / 255.0),
		0.0f,
		0.002f,
		0.0f);

	gScene.GetCamera().GetTransform().TranslateTo(glm::vec3(-100, 200, 0));
	gScene.GetCamera().GetTransform().RotateEuler(glm::vec3(45, 0, 0));

	const char *skybox_filenames[6] = {
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"back.jpg",
		"front.jpg"
	};

	gScene.CreateSkybox(std::vector<std::string>(skybox_filenames, skybox_filenames + 6));

	GLTangentMeshObject & g = (GLTangentMeshObject&)gScene.GetGLObject(0);
	GLMesh & mesh = g.Mesh();
	for (int i = 11; i < 20; i++) {
		mesh.SetupGlow(i, 0.08);
	}
}

void SpecialKeyboard(int key, int x, int y)
{
	float unit = 0.0001;
	GLLight & light_d = gScene.GetGLLight(0);
	Transform td;
	td.TranslateTo(glm::vec3(light_d.GetParms().position[0], light_d.GetParms().position[1], light_d.GetParms().position[2]));
	
	float newPos[3];

	//Transform & transform = gScene.GetGLObject(1).GetTransform();
	switch (key)
	{
	case GLUT_KEY_LEFT:
		//transform.Rotate(glm::normalize(unit * glm::vec3(0.0, 1.0, 0.0)));
		td.Translate(glm::normalize(unit * glm::vec3(-1.0, 0.0, 0.0)));
		light_d.SetPosition(glm::vec4(td.Position(), 1.0));
		break;
	case GLUT_KEY_RIGHT:
		//transform.Rotate(glm::normalize(unit * glm::vec3(0.0, -1.0, 0.0)));
		td.Translate(glm::normalize(unit * glm::vec3(1.0, 0.0, 0.0)));
		light_d.SetPosition(glm::vec4(td.Position(), 1.0));
		break;
	case GLUT_KEY_UP:
		//transform.Rotate(glm::normalize(unit * glm::vec3(1.0, 0.0, 0.0)));
		td.Translate(glm::normalize(unit * glm::vec3(0.0, 0.0, 1.0)));
		light_d.SetPosition(glm::vec4(td.Position(), 1.0));
		break;
	case GLUT_KEY_DOWN:
		//transform.Rotate(glm::normalize(unit * glm::vec3(-1.0, 0.0, 0.0)));
		td.Translate(glm::normalize(unit * glm::vec3(0.0, 0.0, -1.0)));
		light_d.SetPosition(glm::vec4(td.Position(), 1.0));
		break;
	case GLUT_KEY_CTRL_L:
		scanf("%f%f%f",&newPos[0], &newPos[1], &newPos[2]);
		light_d.SetPosition(glm::vec4(newPos[0], newPos[1], newPos[2], 0.0));
		break;
	case GLUT_KEY_ALT_L:
		scanf("%f", &newPos[0]);
		gSceneShaderSetting.dirLight_far = newPos[0];
		break;
	case GLUT_KEY_F1:
		gSceneShaderSetting.shade_normal ^= 0x1;
		break;
	case GLUT_KEY_F2:
		gSceneShaderSetting.enable_msaa = !gSceneShaderSetting.enable_msaa;
		break;
	default:
		break;
	}
}

void SetupWaterTexture() {

	ILuint ilDuDvMap;
	ilGenImages(1, &ilDuDvMap);
	ilBindImage(ilDuDvMap);
	if (ilLoadImage("waterDUDV.png") && ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
		unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 4];
		ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGBA, IL_UNSIGNED_BYTE, data);

		glGenTextures(1, &gWaterConfig.waterDuDv);
		glBindTexture(GL_TEXTURE_2D, gWaterConfig.waterDuDv);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		delete[] data;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	else {
		printf("GO HELL !");
	}

	ilDeleteImages(1, &ilDuDvMap);

}

void BindScreenFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, wndWidth, wndHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
