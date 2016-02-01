#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <limits>

using namespace std;

// window ID
int wid;

// default (initial) window size
#define VPD_DEFAULT 700

// size of the window
int width = VPD_DEFAULT;
int height = VPD_DEFAULT;

// zoom interface related stuff
bool is_middle_mouse_button_down = false;
// controls the field of view
GLfloat zoom = 1;
// coordinates of last [mouse movement with middle button down] event
GLint mprev[2];

// IDs of noise texture and the GPU program
GLuint noise_texture;
GLuint p;

// time
float t = 0.0;
float max_t = numeric_limits<float>::max();

int lic_number_of_steps;

char* read_file(const char* name) {
	int size=0; {
		ifstream ifs(name);
		assert(ifs);
		do {
			size++;
			ifs.get();
		}
		while(!ifs.eof());
	}
	char *res = new char[size];
	ifstream ifs(name);
	ifs.read(res,size-1);
	res[size-1]=0;
	return res;
}

void printShaderInfoLog(GLuint obj) {
	int infologLength=0;
	int charsWritten=0;
	char* infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if(infologLength > 0) {
		infoLog = new char[infologLength];
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		cout << infoLog << endl;
		delete[] infoLog;
	}
}

void printProgramInfoLog(GLuint obj) {
	int infologLength=0;
	int charsWritten=0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if(infologLength > 0) {
		infoLog = new char[infologLength];
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		cout << infoLog << endl;
		delete[] infoLog;
	}
}

GLvoid draw() {
	glutSetWindow(wid);

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-zoom,zoom,-zoom,zoom,-0.5,0.5);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// use the noise texture
	// since we are doing GPU programming YOU have to look up colors in one of the programs
	// etc. in order for texture to work
	glBindTexture(GL_TEXTURE_2D, noise_texture);
	glEnable(GL_TEXTURE_2D);

	// no need for depth test
	glDisable(GL_DEPTH_TEST);

	// use program p
	glUseProgram(p);

	// bump time step
	t += 0.005;
	if(t >= max_t) {
		t = 0.0;
	}
	glUniform1f(glGetUniformLocation(p, "t"), t);
	glUniform1i(glGetUniformLocation(p, "STEPS"), lic_number_of_steps);

	// just render a single quad with texture coords
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2f(-1,-1);

	glTexCoord2f(1,0);
	glVertex2f(1,-1);

	glTexCoord2f(1,1);
	glVertex2f(1,1);

	glTexCoord2f(0,1);
	glVertex2f(-1,1);

	glEnd();
	glFinish();

	// back to the old-fashioned pipeline
	glUseProgram(0);

	glutSwapBuffers();

	// animate
	glutPostRedisplay();
}

GLvoid mouse_button(GLint btn, GLint state, GLint x, GLint y) {
	if(btn == GLUT_MIDDLE_BUTTON) {
		if(state==GLUT_DOWN) {
			is_middle_mouse_button_down = true;
			mprev[0] = x;
			mprev[1] = y;
		}
		if(state==GLUT_UP) {
			is_middle_mouse_button_down = false;
			zoom += 8*(y-mprev[1])/(float)height;
		}
	}
}

GLvoid button_motion(GLint x, GLint y) {
	if(is_middle_mouse_button_down) {
		// middle button is down - update zoom factor (related to field of view - see draw()
		// also, save the
		zoom *= 1+(y-mprev[1])/200.0;
		mprev[0] = x;
		mprev[1] = y;
	}
	glutPostRedisplay(); // add display event to queue
	return;
}

GLvoid keyboard(GLubyte key, GLint x, GLint y) {
	switch(key) {
		// Esc
		case 27:
			exit(0);
			break;
		default:
			break;
	}
}

GLvoid reshape(GLint vpw, GLint vph) {
	glutSetWindow(wid);
	width = vpw;
	height = vph;
	glViewport(0, 0, width, height);
	glutReshapeWindow(width, height);

	// add display event to queue
	glutPostRedisplay();
}


void printHelp() {
	printf("\nControls\n");
	printf("=========\n");
	printf("middle mouse button:\t Zoom camera.\n");
}

int initShaders(string vertex_shader, string fragment_shader) {
	const GLchar *vsh = read_file(vertex_shader.c_str());
	const GLchar *fsh = read_file(fragment_shader.c_str());

	GLint vshid = glCreateShader(GL_VERTEX_SHADER);
	GLint fshid = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vshid, 1, &vsh, NULL);
	glShaderSource(fshid, 1, &fsh, NULL);
	printShaderInfoLog(vshid);
	printShaderInfoLog(fshid);
	glCompileShader(vshid);
	glCompileShader(fshid);
	printShaderInfoLog(vshid);
	printShaderInfoLog(fshid);
	p = glCreateProgram();
	glAttachShader(p, vshid);
	glAttachShader(p, fshid);
	printProgramInfoLog(p);
	glLinkProgram(p);
	printProgramInfoLog(p);
	return 0;
}

int initLIC(int argc, char * argv[]) {
	// defaults
	string lic_number_of_steps_input = "80";
	string vertex_shader = "./src/vertex-shader";
	string fragment_shader = "./src/fragment-shader";

	if (argc > 4) {
		printf("Usage:\n%s [vertex shader] [fragment shader] [lic number of steps]\n", argv[0]);
		printf("Example usage:\n%s %s %s %s\n", argv[0], vertex_shader.c_str(), fragment_shader.c_str(), lic_number_of_steps_input.c_str());
		return 1;
	}
	if(argc > 1) {
		vertex_shader = argv[1];
	}
	if(argc > 2) {
		fragment_shader = argv[2];
	}
	if(argc > 3) {
		lic_number_of_steps_input = argv[3];
	}

	lic_number_of_steps = atoi(lic_number_of_steps_input.c_str());
	lic_number_of_steps = min(max(0, lic_number_of_steps), 200);

	printf("Inputs\n");
	printf("======\n");

	printf("vertex shader: %s\n", vertex_shader.c_str());
	printf("fragment shader: %s\n", fragment_shader.c_str());
	printf("lic number of steps: %i\n", lic_number_of_steps);

	printHelp();
	return initShaders(vertex_shader, fragment_shader);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(10, 10);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	wid = glutCreateWindow("Line Integral Convolution");

	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse_button);
	glutMotionFunc(button_motion);
	glutDisplayFunc(draw);

	// initialize glew and check for OpenGL 2.0 support
	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")) {
		cout << "Ready for OpenGL 2.0" << endl;
	} else {
		cerr << "OpenGL 2.0 not supported" << endl;
		return 1;
	}

	// create the noise texture
	glGenTextures(1 ,&noise_texture);
	glBindTexture(GL_TEXTURE_2D, noise_texture);
	{
		float *arr = new float[VPD_DEFAULT*VPD_DEFAULT];

		for(int i=0; i<VPD_DEFAULT*VPD_DEFAULT; i++) {
			arr[i] = drand48();
		}
		glTexImage2D(GL_TEXTURE_2D, 0, 1, VPD_DEFAULT, VPD_DEFAULT, 0, GL_LUMINANCE, GL_FLOAT, arr);
		delete[] arr;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int status = initLIC(argc, argv);
	if(status != 0) {
		return status;
	}

	glutMainLoop();

	return 0;
}
