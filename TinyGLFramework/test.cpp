#include "tiny_gl.h"
#include "tiny_ctrl.h"

using namespace tiny_gl;
using namespace tiny_ctrl;

static GLubyte timer_cnt = 0;
static bool timer_enabled = true;
static unsigned int timer_speed = 30;
static int wndWidth, wndHeight;

static GLShaderProgram gShaderProg;
static GLScene gScene;
static MouseControl gMouse;

static void Display();
static void Reshape(int w, int h);
static void Timer(int val);
static void Keyboard(unsigned char key, int x, int y);
static void Mouse(int button, int state, int x, int y);
static void PassiveMouse(int x, int y);

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
	glutCreateWindow("tiny_gl"); 
	glewInit();
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	dumpInfo();

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
	gShaderProg.Render(gScene);
	glutSwapBuffers();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	wndWidth = w;
	wndHeight = h;
}

void Timer(int val)
{
	float time = glutGet(GLUT_ELAPSED_TIME);

	float r = 500.0f;
	float scale = 0.001f;

	timer_cnt++;
	glutPostRedisplay();
	if (timer_enabled)
	{
		gScene.Update((float)timer_cnt / 255.0);
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

void SetupShaderProgram()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	gShaderProg.Init();
	
	GLShader vShader;
	vShader.Load("vertex.vs.glsl", GL_VERTEX_SHADER);

	GLShader fShader;
	fShader.Load("fragment.fs.glsl", GL_FRAGMENT_SHADER);

	gShaderProg.AttachShader(vShader);
	gShaderProg.AttachShader(fShader);

	gShaderProg.Link();
	gShaderProg.Use();
}

void SetupScene()
{
	gScene.CreatePerspectiveCamera(60.0f, 0.3f, 3000.0f, (800 / 800));
	gScene.CreateMeshObject("sponza.obj");
	gScene.CreateAnimateMeshObject("zombie_walk.FBX");

	gScene.GetCamera().GetTransform().TranslateTo(glm::vec3(0, 0, 50));
}
