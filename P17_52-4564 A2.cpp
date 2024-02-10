#include <iostream>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <glut.h>
#include <cmath>
#include <SDL.h>
#include <SDL_mixer.h>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
#define COLOR_CHANGE_INTERVAL 3000  // Change color every 3 seconds

int gameTime = 55; // Initial game time in seconds
bool isGameOver = false;
time_t lastTime = time(nullptr);
float humanPosX = 0.45;
float humanPosZ = 1.5;
bool isGameRunning = true;
bool isFootballCollected = false;
bool isGameWon = false;
float scaleFactor = 0.0;
bool isGrowing = true;
float angle = 0.0;
bool isTreeAnimationActive = false;
float trashBinScaleFactor = 0.0;  // Initial scale factor for the trash bin
bool isTrashBinGrowing = true;
bool isTrashBinAnimationActive = false;
float BenchScaleFactor = 0.0;  // Initial scale factor for the bench 
bool isBenchGrowing = true;
bool isBenchAnimationActive = false;
float LampScaleFactor = 0.0;  // Initial scale factor for the lamp 
bool isLampGrowing = true;
bool isLampAnimationActive = false;
float BirdScaleFactor = 0.0;  // Initial scale factor for the bird 
bool isBirdGrowing = true;
bool isBirdAnimationActive = false;
float footballBounceHeight = 0.03;
unsigned int lastColorChangeTime = 0;
float red = 0.0;
float blue = 0.0;
float green = 0.0;
float headingAngle = 0.0f;

Mix_Chunk* animationSound;
int animationSoundChannel = -1;
Mix_Chunk* footballSound;
Mix_Chunk* collisionSound;
Mix_Chunk* winSound;
Mix_Chunk* loseSound;




class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);

	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}

};

Camera camera;

Uint32 ResumeBackgroundSound(Uint32 interval, void* param) {
	// Resume the background sound
	Mix_Resume(-1);
	return 0;  // Timer only runs once
}

Uint32 ResumeFootballSound(Uint32 interval, void* param) {
	// Resume the background sound
	Mix_Resume(-1);
	return 0;  // Timer only runs once
}

void changeWallColors() {
	// Change colors based on some criteria (you can modify this logic)
	glColor3f(red, green, blue);
}

void drawBoundaryWalls() {

	// Disable depth testing for the second shape to avoid flickering
	glDisable(GL_DEPTH_TEST);

	// Draw the left wall
	glPushMatrix();
	glTranslated(0.25, 0.5, 1.0);

	// First primitive shape (solid cube)
	glPushMatrix();
	glRotated(90.0, 0, 0, 1);
	glScaled(1.0, 0.02, 1.0);
	changeWallColors();
	glutSolidCube(1);
	glPopMatrix();

	//glEnable(GL_DEPTH_TEST);

	// Second primitive shape (black lines cube)
	glPushMatrix();
	glTranslated(0.0, 0.0, 0.0); // No offset for the second shape
	glRotated(90.0, 0, 0, 1);
	glScaled(0.8, 0.02, 0.8); // Adjust scaling to make the second shape more visible
	glColor3f(0.0, 0.0, 0.0);
	glutWireCube(1);
	glPopMatrix();

	glPopMatrix();

	// Draw the middle wall (between the left and right walls)
	glPushMatrix();
	glTranslated(0.75, 0.5, 0.5);

	// First primitive shape (solid cube)
	glPushMatrix();
	glRotated(90.0, 1, 0, 0);
	glScaled(1.0, 0.02, 1.0);
	changeWallColors();
	glutSolidCube(1);
	glPopMatrix();

	// Re-enable depth testing for subsequent shapes
	//glEnable(GL_DEPTH_TEST);

	// Second primitive shape (black lines cube)
	glPushMatrix();
	glTranslated(0.0, 0.0, 0.0); // No offset for the second shape
	glRotated(90.0, 1, 0, 0);
	glScaled(0.8, 0.02, 0.8); // Adjust scaling to make the second shape more visible
	glColor3f(0.0, 0.0, 0.0);
	glutWireCube(1);
	glPopMatrix();

	glPopMatrix();

	// Draw the right wall
	glPushMatrix();
	glTranslated(1.25, 0.5, 1.0);

	// First primitive shape (solid cube)
	glPushMatrix();
	glRotated(90.0, 0, 0, 1);
	glScaled(1.0, 0.02, 1.0);
	changeWallColors();
	glutSolidCube(1);
	glPopMatrix();

	// Re-enable depth testing for subsequent shapes
	glEnable(GL_DEPTH_TEST);

	// Second primitive shape (black lines cube)
	glPushMatrix();
	glTranslated(0.0, 0.0, 0.0); // No offset for the second shape
	glRotated(90.0, 0, 0, 1);
	glScaled(0.8, 0.02, 0.8); // Adjust scaling to make the second shape more visible
	glColor3f(0.0, 0.0, 0.0);
	glutWireCube(1);
	glPopMatrix();

	glPopMatrix();
}


void drawGround() {
	// Draw the ground (grass)
	glPushMatrix();
	glTranslated(0.75, 0.0, 1.0);  // Adjust the translation for the ground
	glScaled(1.0, 0.02, 1.0);       // Adjust the scaling for the ground
	glColor3f(0.1, 0.5, 0);
	glutSolidCube(1);
	glPopMatrix();
}

void drawTree() {
	if (isTreeAnimationActive) {
		// Draw the trunk
		glPushMatrix();
		glTranslated(0.4, 0.12, 0.8);  // Adjust the position for the trunk (left corner)
		glScaled(0.08, 0.3, 0.08);      // Adjust the size of the trunk (taller and thinner)
		glColor3f(0.5, 0.3, 0.0);      // Brown color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the leaves (two cones as the crown of the tree)
		glPushMatrix();
		glTranslated(0.4, 0.28, 0.8);  // Adjust the position for the leaves (higher and left)

		// Draw the first cone
		glPushMatrix();
		glRotated(-90, 1, 0, 0);       // Rotate the cone to face upward
		glScaled(0.5 * scaleFactor, 0.5 * scaleFactor, 0.5 * scaleFactor);    // Adjust the size of the cone (smaller)
		glColor3f(0.0, 0.5, 0.0);      // Green color
		glutSolidCone(0.2, 0.2, 20, 20);
		glPopMatrix();

		// Draw the second cone
		glPushMatrix();
		glTranslated(0.0, 0.11, 0.0);  // Move up for the second cone
		glRotated(-90, 1, 0, 0);       // Rotate the cone to face upward
		glScaled(0.4 * scaleFactor, 0.4 * scaleFactor, 0.4 * scaleFactor);     // Adjust the size of the cone (smaller)
		glColor3f(0.0, 0.5, 0.0);      // Green color
		glutSolidCone(0.2, 0.2, 20, 20);
		glPopMatrix();

		glPopMatrix();
	}
	else {
		glPushMatrix();
		glTranslated(0.4, 0.12, 0.8);  // Adjust the position for the trunk (left corner)
		glScaled(0.08, 0.3, 0.08);      // Adjust the size of the trunk (taller and thinner)
		glColor3f(0.5, 0.3, 0.0);      // Brown color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the leaves (two cones as the crown of the tree)
		glPushMatrix();
		glTranslated(0.4, 0.28, 0.8);  // Adjust the position for the leaves (higher and left)

		// Draw the first cone
		glPushMatrix();
		glRotated(-90, 1, 0, 0);       // Rotate the cone to face upward
		glScaled(0.5, 0.5, 0.5);    // Adjust the size of the cone (smaller)
		glColor3f(0.0, 0.5, 0.0);      // Green color
		glutSolidCone(0.2, 0.2, 20, 20);
		glPopMatrix();

		// Draw the second cone
		glPushMatrix();
		glTranslated(0.0, 0.11, 0.0);  // Move up for the second cone
		glRotated(-90, 1, 0, 0);       // Rotate the cone to face upward
		glScaled(0.4, 0.4, 0.4);     // Adjust the size of the cone (smaller)
		glColor3f(0.0, 0.5, 0.0);      // Green color
		glutSolidCone(0.2, 0.2, 20, 20);
		glPopMatrix();

		glPopMatrix();
	}
}

void drawTrashBin() {
	if (isTrashBinAnimationActive) {
		// Draw the body of the trash bin
		glPushMatrix();
		glTranslated(1.10, 0.07, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.1 * trashBinScaleFactor, 0.15 * trashBinScaleFactor, 0.1 * trashBinScaleFactor);  // Adjust the size of the body
		glColor3f(0.5, 0.5, 0.5);       // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the cover of the trash bin
		glPushMatrix();
		glTranslated(1.10, 0.1, 0.7);   // Adjusted to the right corner of the ground
		glScaled(0.1 * trashBinScaleFactor, 0.02 * trashBinScaleFactor, 0.1 * trashBinScaleFactor);  // Adjust the size of the cover
		glColor3f(0.5, 0.5, 0.5);      // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the opening on the cover
		glPushMatrix();
		glTranslated(1.10, 0.12, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.06 * trashBinScaleFactor, 0.01 * trashBinScaleFactor, 0.06 * trashBinScaleFactor);  // Adjust the size of the opening
		glColor3f(0.0, 0.0, 0.0);      // Black color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the left side
		glPushMatrix();
		glTranslated(1.05, 0.1, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.01 * trashBinScaleFactor, 0.13 * trashBinScaleFactor, 0.1 * trashBinScaleFactor);  // Adjust the size of the left side
		glColor3f(0.5, 0.5, 0.5);      // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the right side
		glPushMatrix();
		glTranslated(1.15, 0.1, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.01 * trashBinScaleFactor, 0.13 * trashBinScaleFactor, 0.1 * trashBinScaleFactor);  // Adjust the size of the right side
		glColor3f(0.5, 0.5, 0.5);      // Gray color
		glutSolidCube(1);
		glPopMatrix();
	}
	else {
		// Draw the body of the trash bin
		glPushMatrix();
		glTranslated(1.10, 0.07, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.1, 0.15, 0.1);       // Adjust the size of the body
		glColor3f(0.5, 0.5, 0.5);       // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the cover of the trash bin
		glPushMatrix();
		glTranslated(1.10, 0.1, 0.7);   // Adjusted to the right corner of the ground
		glScaled(0.1, 0.02, 0.1);      // Adjust the size of the cover
		glColor3f(0.5, 0.5, 0.5);      // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the opening on the cover
		glPushMatrix();
		glTranslated(1.10, 0.12, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.06, 0.01, 0.06);    // Adjust the size of the opening
		glColor3f(0.0, 0.0, 0.0);      // Black color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the left side
		glPushMatrix();
		glTranslated(1.05, 0.1, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.01, 0.13, 0.1);     // Adjust the size of the left side
		glColor3f(0.5, 0.5, 0.5);      // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the right side
		glPushMatrix();
		glTranslated(1.15, 0.1, 0.7);  // Adjusted to the right corner of the ground
		glScaled(0.01, 0.13, 0.1);     // Adjust the size of the right side
		glColor3f(0.5, 0.5, 0.5);      // Gray color
		glutSolidCube(1);
		glPopMatrix();
	}
}


void drawParkBench() {
	if (isBenchAnimationActive) {
		// Draw the seat of the bench
		glPushMatrix();
		glTranslated(0.75, 0.15, 0.6);  // Adjust the position for the seat
		glScaled(0.4 * BenchScaleFactor, 0.02 * BenchScaleFactor, 0.15 * BenchScaleFactor);      // Adjust the size of the seat (smaller)
		glColor3f(0.4, 0.2, 0.0);      // Brown color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the backrest of the bench
		glPushMatrix();
		glTranslated(0.75, 0.15, 0.6);   // Adjust the position for the backrest
		glScaled(0.4 * BenchScaleFactor, 0.08 * BenchScaleFactor, 0.02 * BenchScaleFactor);      // Adjust the size of the backrest (smaller)
		glColor3f(0.4, 0.2, 0.0);      // Brown color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the legs of the bench
		for (float i = 0.07; i <= 0.2; i += 0.4) {
			glPushMatrix();
			glTranslated(0.75, i, 0.58);  // Adjust the position for the legs
			glScaled(0.03 * BenchScaleFactor, 0.08 * BenchScaleFactor, 0.03 * BenchScaleFactor);   // Adjust the size of the legs (smaller)
			glColor3f(0.4, 0.2, 0.0);    // Brown color
			glutSolidCube(1);
			glPopMatrix();
		}
	}
	else {
		// Draw the seat of the bench
		glPushMatrix();
		glTranslated(0.75, 0.15, 0.6);  // Adjust the position for the seat
		glScaled(0.4, 0.02, 0.15);      // Adjust the size of the seat (smaller)
		glColor3f(0.4, 0.2, 0.0);      // Brown color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the backrest of the bench
		glPushMatrix();
		glTranslated(0.75, 0.15, 0.6);   // Adjust the position for the backrest
		glScaled(0.4, 0.08, 0.02);      // Adjust the size of the backrest (smaller)
		glColor3f(0.4, 0.2, 0.0);      // Brown color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the legs of the bench
		for (float i = 0.07; i <= 0.2; i += 0.4) {
			glPushMatrix();
			glTranslated(0.75, i, 0.58);  // Adjust the position for the legs
			glScaled(0.03, 0.08, 0.03);   // Adjust the size of the legs (smaller)
			glColor3f(0.4, 0.2, 0.0);    // Brown color
			glutSolidCube(1);
			glPopMatrix();
		}
	}

}

void drawLampPost() {
	if (isLampAnimationActive) {
		// Draw the post
		glPushMatrix();
		glTranslated(1.0, 0.24, 0.7);    // Adjust the position for the post (beside the bench)
		glScaled(0.03 * LampScaleFactor, 0.5 * LampScaleFactor, 0.03 * LampScaleFactor);      // Adjust the size of the post (thinner)
		glColor3f(0.4, 0.4, 0.4);       // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the lamp head
		glPushMatrix();
		glTranslated(1.0, 0.5, 0.7);    // Adjust the position for the lamp head (raised slightly)
		glScaled(0.15 * LampScaleFactor, 0.05 * LampScaleFactor, 0.15 * LampScaleFactor);     // Adjust the size of the lamp head (thinner)
		glColor3f(0.6, 0.6, 0.0);       // Yellow color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the light bulb on the lamp head
		glPushMatrix();
		glTranslated(1.0, 0.5, 0.7);    // Adjust the position for the light bulb
		glutSolidSphere(0.04, 20, 20);   // Adjust the size of the light bulb (thinner)
		glColor3f(1.0, 1.0, 0.8);        // Light yellow color
		glPopMatrix();
	}
	else {
		// Draw the post
		glPushMatrix();
		glTranslated(1.0, 0.24, 0.7);    // Adjust the position for the post (beside the bench)
		glScaled(0.03, 0.5, 0.03);      // Adjust the size of the post (thinner)
		glColor3f(0.4, 0.4, 0.4);       // Gray color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the lamp head
		glPushMatrix();
		glTranslated(1.0, 0.5, 0.7);    // Adjust the position for the lamp head (raised slightly)
		glScaled(0.15, 0.05, 0.15);     // Adjust the size of the lamp head (thinner)
		glColor3f(0.6, 0.6, 0.0);       // Yellow color
		glutSolidCube(1);
		glPopMatrix();

		// Draw the light bulb on the lamp head
		glPushMatrix();
		glTranslated(1.0, 0.5, 0.7);    // Adjust the position for the light bulb
		glutSolidSphere(0.04, 20, 20);   // Adjust the size of the light bulb (thinner)
		glColor3f(1.0, 1.0, 0.8);        // Light yellow color
		glPopMatrix();
	}

}

void drawFootball() {
	// Check if the football has been collected
	if (!isFootballCollected) {
		float footballBounce = footballBounceHeight * sin(angle);
		// Draw the black and white pattern on the football
		glColor3f(0.0, 0.0, 0.0); // Black
		glPushMatrix();
		glTranslated(0.75, 0.07 + footballBounce, 1.0);  // Adjust the position for the football (center of the ground)
		glutSolidSphere(0.04, 20, 20);  // Adjust the size of the football
		glPopMatrix();

		glColor3f(1.0, 1.0, 1.0); // White
		glPushMatrix();
		glTranslated(0.75, 0.07 + footballBounce, 1.0); // Adjust the position for the football (center of the ground)
		glRotated(90.0, 1, 0, 0); // Rotate the next sphere to cover the sides
		glutSolidSphere(0.04, 20, 20);  // Adjust the size of the football
		glPopMatrix();

		glColor3f(0.0, 0.0, 0.0); // Black
		glPushMatrix();
		glTranslated(0.75, 0.07 + footballBounce, 1.0);  // Adjust the position for the football (center of the ground)
		glRotated(-90.0, 1, 0, 0); // Rotate the next sphere to cover the top and bottom
		glutSolidSphere(0.04, 20, 20);  // Adjust the size of the football
		glPopMatrix();
	}
	Mix_PlayChannel(-1, footballSound, -1);
}

void drawBird() {
	if (isBirdAnimationActive) {
		glPushMatrix();
		glTranslatef(0.70, 0.3, 0.8);  // Adjust the position for the bird
		glScaled(0.02 * BirdScaleFactor, 0.02 * BirdScaleFactor, 0.02 * BirdScaleFactor);     // Adjust the size of the bird
		glColor3f(0.8, 0.4, 0.1);

		glBegin(GL_LINE_STRIP);
		glVertex2f(-5, 25);  // 1
		glVertex2f(0, 30);   // 2
		glVertex2f(5, 25);   // 3
		glVertex2f(10, 30);  // 4
		glVertex2f(15, 25);  // 5
		glVertex2f(10, 28);  // 6
		glVertex2f(5, 23);   // 7
		glVertex2f(0, 28);   // 8
		glVertex2f(-5, 25);  // 9
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex3f(0, 30, 0);   // 2
		glVertex3f(0, 28, 0);   // 8
		glVertex3f(-5, 25, 0);  // 1
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex3f(10, 30, 0);  // 4
		glVertex3f(10, 28, 0);  // 6
		glVertex3f(15, 25, 0);  // 5
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(0, 30);   // 2
		glVertex2f(0, 28);   // 8
		glVertex2f(5, 23);   // 7
		glVertex2f(5, 25);   // 3
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(5, 25);   // 3
		glVertex2f(10, 30);  // 4
		glVertex2f(10, 28);  // 6
		glVertex2f(5, 23);   // 7
		glEnd();

		glPopMatrix();
	}
	else {
		glPushMatrix();
		glTranslatef(0.70, 0.3, 0.8);  // Adjust the position for the bird
		glScaled(0.02, 0.02, 0.02);     // Adjust the size of the bird
		glColor3f(0.8, 0.4, 0.1);

		glBegin(GL_LINE_STRIP);
		glVertex2f(-5, 25);  // 1
		glVertex2f(0, 30);   // 2
		glVertex2f(5, 25);   // 3
		glVertex2f(10, 30);  // 4
		glVertex2f(15, 25);  // 5
		glVertex2f(10, 28);  // 6
		glVertex2f(5, 23);   // 7
		glVertex2f(0, 28);   // 8
		glVertex2f(-5, 25);  // 9
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex3f(0, 30, 0);   // 2
		glVertex3f(0, 28, 0);   // 8
		glVertex3f(-5, 25, 0);  // 1
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex3f(10, 30, 0);  // 4
		glVertex3f(10, 28, 0);  // 6
		glVertex3f(15, 25, 0);  // 5
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(0, 30);   // 2
		glVertex2f(0, 28);   // 8
		glVertex2f(5, 23);   // 7
		glVertex2f(5, 25);   // 3
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(5, 25);   // 3
		glVertex2f(10, 30);  // 4
		glVertex2f(10, 28);  // 6
		glVertex2f(5, 23);   // 7
		glEnd();

		glPopMatrix();
	}

}

/*void drawTrailSign() {
	// Post
	glPushMatrix();
	glTranslated(1.0, 0.0, 0.7);  // Adjust the position for the post
	glScaled(0.02, 0.5, 0.02);     // Adjust the size of the post
	glColor3f(0.6, 0.4, 0.2);      // Brown color
	glutSolidCube(1);
	glPopMatrix();

	// Signboard
	glPushMatrix();
	glTranslated(1.0, 0.25, 0.7);  // Adjust the position for the signboard
	glRotated(45.0, 0, 1, 0);       // Rotate the signboard
	glScaled(0.2, 0.02, 0.2);      // Adjust the size of the signboard
	glColor3f(1.0, 1.0, 1.0);      // White color
	glutSolidCube(1);
	glPopMatrix();

	// Trail symbol (e.g., a tree icon)
	glPushMatrix();
	glTranslated(1.0, 0.25, 0.7);  // Adjust the position for the symbol
	glScaled(0.1, 0.1, 0.1);       // Adjust the size of the symbol
	glColor3f(0.0, 0.5, 0.0);      // Green color (for a tree symbol)
	// Draw your symbol (e.g., a tree) here
	glutSolidCube(1);
	glPopMatrix();
}*/


void drawHuman() {
	// Body
	glPushMatrix();
	glTranslated(humanPosX, 0.2, humanPosZ);
	glRotated(headingAngle, 0, 1, 0);
	glScaled(0.07, 0.15, 0.035);  // Smaller size
	glColor3f(0.0, 0.5, 1.0);  // Blue color
	glutSolidCube(1);
	glPopMatrix();

	// Head
	glPushMatrix();
	glTranslated(humanPosX, 0.325, humanPosZ);  // Adjusted head position
	glutSolidSphere(0.035, 20, 20);  // Smaller size
	glColor3f(1.0, 0.87, 0.68);  // Skin color
	glPopMatrix();

	// Legs
	glPushMatrix();
	glTranslated(humanPosX - 0.015, 0.1, humanPosZ);  // Adjusted left leg position
	glScaled(0.015, 0.15, 0.015);   // Smaller size
	glRotated(headingAngle, 0, 1, 0);
	glColor3f(1.0, 0.0, 0.0);       // Red color
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslated(humanPosX + 0.015, 0.1, humanPosZ);  // Adjusted right leg position
	glScaled(0.015, 0.15, 0.015);   // Smaller size
	glRotated(headingAngle, 0, 1, 0);
	glColor3f(1.0, 0.0, 0.0);       // Red color
	glutSolidCube(1);
	glPopMatrix();

	// Right Arm
	glPushMatrix();
	glTranslated(humanPosX - 0.025, 0.25, humanPosZ);  // Adjusted left arm position
	glRotated(45.0 + headingAngle, 0, 0, 1);
	glScaled(0.01, 0.07, 0.01);  // Smaller size
	glColor3f(1.0, 0.87, 0.68);  // Skin color
	glutSolidCube(1);
	glPopMatrix();

	// Left Arm
	glPushMatrix();
	glTranslated(humanPosX + 0.025, 0.25, humanPosZ);  // Adjusted right arm position
	glRotated(-45.0 + headingAngle, 0, 0, 1);
	glScaled(0.01, 0.07, 0.01);  // Smaller size
	glColor3f(1.0, 0.87, 0.68);  // Skin color
	glutSolidCube(1);
	glPopMatrix();
}



void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}


void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}


void UpdateTime() {
	time_t currentTime = time(nullptr);
	double elapsedTime = difftime(currentTime, lastTime);

	gameTime -= static_cast<int>(elapsedTime);

	lastTime = currentTime;

	if (gameTime <= 0 && !isFootballCollected) {
		gameTime = 0;
		Mix_Pause(-1);

		// Handle goal collision (player has reached the goal)
		isGameOver = true;

		Mix_PlayChannel(1, loseSound, 0);

		SDL_Delay(2000);
		// Resume the background sound after a delay (adjust the delay as needed)
		SDL_AddTimer(9000, ResumeBackgroundSound, NULL);
		SDL_AddTimer(9000, ResumeFootballSound, NULL);


	}

	// Check if the football has been collected and the game is not over
	if (isFootballCollected && !isGameOver) {
		Mix_Pause(-1);

		// Handle goal collision (player has reached the goal)
		isGameWon = true;

		Mix_PlayChannel(1, winSound, 0);
		SDL_Delay(2000);

		// Resume the background sound after a delay (adjust the delay as needed)
		SDL_AddTimer(9000, ResumeBackgroundSound, NULL);
		SDL_AddTimer(9000, ResumeFootballSound, NULL);

	}
}

void Timer(int value) {
	UpdateTime();
	red = (rand() % 100) / 100.0;
	green = (rand() % 100) / 100.0;
	blue = (rand() % 100) / 100.0;
	glutPostRedisplay(); // Redraw the scene
	glutTimerFunc(1000, Timer, 0);


}

void drawText(const std::string& text, int x, int y, void* font) {
	glRasterPos2i(x, y);

	for (char c : text) {
		glutBitmapCharacter(font, c);
	}
}

void drawTextCentered(const std::string& text, int centerX, int centerY, void* font) {
	int width = glutBitmapLength(font, (const unsigned char*)text.c_str());
	int height = 18; // Adjust based on the chosen font size (e.g., GLUT_BITMAP_HELVETICA_18)

	int x = centerX - width / 2;
	int y = centerY - height / 2;

	drawText(text, x, y, font);
}

void displayTime() {
	// Switch to 2D projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

	// Switch to modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(0.0, 0.0, 0.0);

	// Check if the game is over or won
	if (isGameOver || isGameWon) {
		glColor3f(0.0, 0.0, 0.0); // Black background
		glColor3f(0.0, 0.0, 0.0); // White color

		// Choose the appropriate message based on the game state
		std::stringstream gameOverText;
		gameOverText << (isGameWon ? "Game Win!" : "Game Over!");

		// Display the message in the center of the screen with a larger font
		drawTextCentered(gameOverText.str(), 300, 250, GLUT_BITMAP_HELVETICA_18);
	}
	// Check if the football is collected
	else if (!isFootballCollected && abs(humanPosX - 0.75) < 0.05 && abs(humanPosZ - 1.0) < 0.05) {
		isFootballCollected = true;
	}
	else {
		// Display the remaining time with a larger font
		std::stringstream timeText;
		timeText << "Time: " << gameTime << "s";

		// Display the time in the top-left corner with a larger font
		drawText(timeText.str(), 10, glutGet(GLUT_WINDOW_HEIGHT) - 30, GLUT_BITMAP_HELVETICA_18);
	}
}


void animate() {
	angle += 0.01;  // Adjust the rotation speed
	unsigned int currentTime = glutGet(GLUT_ELAPSED_TIME);

	if (currentTime - lastColorChangeTime > COLOR_CHANGE_INTERVAL) {
		// Update the last color change time
		lastColorChangeTime = currentTime;

		// Change colors for the bounding walls
		changeWallColors();
	}

	if (isTreeAnimationActive) {
		if (isGrowing) {
			scaleFactor += 0.001;  // Increase the scale factor for growing
			if (scaleFactor > 1.5) {
				isGrowing = false;  // Change direction when the tree reaches a certain size
			}
		}
		else {
			scaleFactor -= 0.001;  // Decrease the scale factor for shrinking
			if (scaleFactor < 0.5) {
				isGrowing = true;  // Change direction when the tree reaches a certain size
			}
		}

	}
	if (isTrashBinAnimationActive) {
		if (isTrashBinGrowing) {
			trashBinScaleFactor += 0.001;  // Increase the scale factor for growing
			if (trashBinScaleFactor > 1.5) {
				isTrashBinGrowing = false;  // Change direction when the trash bin reaches a certain size
			}
		}
		else {
			trashBinScaleFactor -= 0.001;  // Decrease the scale factor for shrinking
			if (trashBinScaleFactor < 0.5) {
				isTrashBinGrowing = true;  // Change direction when the trash bin reaches a certain size
			}
		}
	}
	if (isBenchAnimationActive) {
		if (isBenchGrowing) {
			BenchScaleFactor += 0.001;  // Increase the scale factor for growing
			if (BenchScaleFactor > 1.5) {
				isBenchGrowing = false;  // Change direction when the trash bin reaches a certain size
			}
		}
		else {
			BenchScaleFactor -= 0.001;  // Decrease the scale factor for shrinking
			if (BenchScaleFactor < 0.5) {
				isBenchGrowing = true;  // Change direction when the trash bin reaches a certain size
			}
		}
	}
	if (isLampAnimationActive) {
		if (isLampGrowing) {
			LampScaleFactor += 0.001;  // Increase the scale factor for growing
			if (LampScaleFactor > 1.5) {
				isLampGrowing = false;  // Change direction when the trash bin reaches a certain size
			}
		}
		else {
			LampScaleFactor -= 0.001;  // Decrease the scale factor for shrinking
			if (LampScaleFactor < 0.5) {
				isLampGrowing = true;  // Change direction when the trash bin reaches a certain size
			}
		}
	}
	if (isBirdAnimationActive) {
		if (isBirdGrowing) {
			BirdScaleFactor += 0.001;  // Increase the scale factor for growing
			if (BirdScaleFactor > 1.1) {
				isBirdGrowing = false;  // Change direction when the trash bin reaches a certain size
			}
		}
		else {
			BirdScaleFactor -= 0.001;  // Decrease the scale factor for shrinking
			if (BirdScaleFactor < 0.5) {
				isBirdGrowing = true;  // Change direction when the trash bin reaches a certain size
			}
		}
	}

	glutPostRedisplay();
}


void Display() {
	setupCamera();
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawGround();
	drawBoundaryWalls();
	drawTree();
	drawTrashBin();
	drawParkBench();
	drawLampPost();
	drawFootball();
	drawBird();
	drawHuman();

	displayTime();


	glFlush();


	if (isGameOver || isGameWon) {
		isGameRunning = false;
	}
}

bool checkCollision(float x, float z) {
	// Check collision with tree
	if (x > 0.32 && x < 0.68 && z > 0.6 && z < 0.9) {
		Mix_PlayChannel(1, collisionSound, 0);
		return true;
	}

	// Check collision with bench
	if (x > 0.55 && x < 0.95 && z > 0.45 && z < 0.75) {
		Mix_PlayChannel(1, collisionSound, 0);
		return true;
	}

	// Check collision with trash bin
	if (x > 1.0 && x < 1.2 && z > 0.6 && z < 0.8) {
		Mix_PlayChannel(1, collisionSound, 0);
		return true;
	}

	// Check collision with lamp post
	if (x > 0.9 && x < 1.1 && z > 0.45 && z < 0.75) {
		Mix_PlayChannel(1, collisionSound, 0);
		return true;
	}

	if (!isFootballCollected && x > 0.72 && x < 0.78 && z > 0.98 && z < 1.02) {
		isFootballCollected = true;
		//gameTime = 0;  // Stop the game timer
		return true;
	}

	return false;
}


void Keyboard(unsigned char key, int x, int y) {
	float d = 0.01;
	float m = 0.01;

	if (!isGameRunning || isFootballCollected) {
		return; // If the game is not running, ignore keyboard input
	}

	if (key == '5') {
		isTreeAnimationActive = !isTreeAnimationActive;
		if (isTreeAnimationActive) {
			// Start or resume animation sound
			animationSoundChannel = Mix_PlayChannel(1, animationSound, -1);
		}
		else {
			// Stop animation sound
			Mix_HaltChannel(animationSoundChannel);
		}
	}
	if (key == '6') {
		isTrashBinAnimationActive = !isTrashBinAnimationActive;
		if (isTrashBinAnimationActive) {
			// Start or resume animation sound
			animationSoundChannel = Mix_PlayChannel(1, animationSound, -1);
		}
		else {
			// Stop animation sound
			Mix_HaltChannel(animationSoundChannel);
		}
	}
	if (key == '7') {
		isBenchAnimationActive = !isBenchAnimationActive;
		if (isBenchAnimationActive) {
			// Start or resume animation sound
			animationSoundChannel = Mix_PlayChannel(1, animationSound, -1);
		}
		else {
			// Stop animation sound
			Mix_HaltChannel(animationSoundChannel);
		}
	}
	if (key == '8') {
		isLampAnimationActive = !isLampAnimationActive;
		if (isLampAnimationActive) {
			// Start or resume animation sound
			animationSoundChannel = Mix_PlayChannel(1, animationSound, -1);
		}
		else {
			// Stop animation sound
			Mix_HaltChannel(animationSoundChannel);
		}
	}
	if (key == '9') {
		isBirdAnimationActive = !isBirdAnimationActive;
		if (isBirdAnimationActive) {
			// Start or resume animation sound
			animationSoundChannel = Mix_PlayChannel(1, animationSound, -1);
		}
		else {
			// Stop animation sound
			Mix_HaltChannel(animationSoundChannel);
		}
	}

	switch (key) {
		// Human movement
	case 'w':
		headingAngle = 0.0f;
		// Check if the next position is within the boundaries and no collision
		if (humanPosZ + m < 1.5 && !checkCollision(humanPosX, humanPosZ + m)) {
			humanPosZ += m;
		}
		break;
	case 's':
		headingAngle = 180.0f;
		// Check if the next position is within the boundaries and no collision
		if (humanPosZ - m > 0.0 && !checkCollision(humanPosX, humanPosZ - m)) {
			humanPosZ -= m;
		}
		break;
	case 'a':
		headingAngle = 90.0f;
		// Check if the next position is within the boundaries and no collision
		if (humanPosX - m > 0.32 && !checkCollision(humanPosX - m, humanPosZ)) {
			humanPosX -= m;
		}
		break;
	case 'd':
		headingAngle = -90.0f;
		// Check if the next position is within the boundaries and no collision
		if (humanPosX + m < 1.18 && !checkCollision(humanPosX + m, humanPosZ)) {
			humanPosX += m;
		}
		break;

		// Camera movement
	case 'i':
		camera.moveY(d);
		break;
	case 'k':
		camera.moveY(-d);
		break;
	case 'j':
		camera.moveX(d);
		break;
	case 'l':
		camera.moveX(-d);
		break;
	case 'u':
		camera.moveZ(d);
		break;
	case 'o':
		camera.moveZ(-d);
		break;
	case '1': //front view
		camera.eye = Vector3f(0.75, 1.0, 5.0);
		camera.center = Vector3f(0.75, 0.0, 1.0);
		camera.up = Vector3f(0.0, 1.0, 0.0);
		break;
	case '2': // top view
		camera.eye = Vector3f(0.75, 2.5, 1.0);
		camera.center = Vector3f(0.75, 0.0, 1.0);
		camera.up = Vector3f(0.0, 0.0, -1.0);
		break;
	case '3': // Side view (right wall)
		camera.eye = Vector3f(1.3, 3.0, 1.0);   // Adjusted eye position
		camera.center = Vector3f(0.75, 0.0, 1.0);  // Adjusted center position
		camera.up = Vector3f(0.0, 1.0, 0.0);        // Adjusted up vector
		break;
	case '4': // Side view (left wall)
		camera.eye = Vector3f(0.0, 1.0, 1.0);   // Adjusted eye position
		camera.center = Vector3f(0.75, 0.0, 1.0);  // Adjusted center position
		camera.up = Vector3f(0.0, 1.0, 0.0);        // Adjusted up vector
		break;

	}

	// Ensure that the camera stays within boundaries
	camera.eye.x = std::min(std::max(camera.eye.x, 0.0f), 1.5f);
	camera.eye.y = std::min(std::max(camera.eye.y, 0.0f), 1.5f);
	camera.eye.z = std::min(std::max(camera.eye.z, 0.0f), 5.0f);

	// Ensure that the human stays within boundaries
	humanPosX = std::min(std::max(humanPosX, 0.32f), 1.18f);
	humanPosZ = std::min(std::max(humanPosZ, 0.0f), 1.5f);
	if (key == 'w' || key == 's' || key == 'a' || key == 'd') {
		if (checkCollision(humanPosX, humanPosZ)) {
			Mix_PlayChannel(0, collisionSound, 0); // Play collision sound
		}
	}


	glutPostRedisplay();
}


void Special(int key, int x, int y) {
	float a = 1.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}

int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL initialization failed: %s\n", SDL_GetError());
		return 1; // Exit the program with an error code
	}

	// Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer initialization failed: %s\n", Mix_GetError());
		return 1; // Exit the program with an error code
	}

	// Load the background sound effect
	Mix_Chunk* backgroundSound = Mix_LoadWAV("C:\\Users\\user\\Downloads\\background.wav");
	if (backgroundSound == NULL) {
		printf("Failed to load background sound effect: %s\n", Mix_GetError());
		return 1; // Exit the program with an error code
	}
	// Play the background sound effect
	Mix_PlayChannel(0, backgroundSound, -1);

	animationSound = Mix_LoadWAV("C:\\Users\\user\\Downloads\\animation.wav");
	if (animationSound == NULL) {
		printf("Failed to load animation sound effect: %s\n", Mix_GetError());
		return 1; // Exit the program with an error code
	}

	footballSound = Mix_LoadWAV("C:\\Users\\user\\Downloads\\football.wav");
	if (animationSound == NULL) {
		printf("Failed to load football sound effect: %s\n", Mix_GetError());
		return 1; // Exit the program with an error code
	}

	collisionSound = Mix_LoadWAV("C:\\Users\\user\\Downloads\\collision.wav");
	if (collisionSound == NULL) {
		printf("Failed to load collision sound effect: %s\n", Mix_GetError());
		return 1; // Exit the program with an error code
	}

	winSound = Mix_LoadWAV("C:\\Users\\user\\Downloads\\win.wav");
	if (winSound == NULL) {
		printf("Failed to load win sound effect: %s\n", Mix_GetError());
		return 1; // Exit the program with an error code
	}

	loseSound = Mix_LoadWAV("C:\\Users\\user\\Downloads\\lose.wav");
	if (loseSound == NULL) {
		printf("Failed to load lose sound effect: %s\n", Mix_GetError());
		return 1; // Exit the program with an error code
	}

	glutInit(&argc, argv);

	glutInitWindowSize(800, 600);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("Lab 6");
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);
	glutIdleFunc(animate);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);



	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);


	glShadeModel(GL_SMOOTH);

	glutTimerFunc(1000, Timer, 0);

	glutMainLoop();


	Mix_FreeChunk(backgroundSound);
	Mix_FreeChunk(collisionSound);
	Mix_FreeChunk(footballSound);
	Mix_FreeChunk(animationSound);
	Mix_FreeChunk(winSound);
	Mix_FreeChunk(loseSound);
	Mix_CloseAudio();
	SDL_Quit();

	return 0;
}