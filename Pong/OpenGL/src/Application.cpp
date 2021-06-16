//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
#include "Graphics2D.h"

#include <iostream>

using namespace std;

Graphics2D engine = Graphics2D(960, 540, "Pong", false);

class Paddle {
	private:
		float xPos;
		float yPos;
		float width;
		float height;
		float maxY;
		float minY;

	public:
		Paddle(float xPos, float yPos, float width, float height, float minY, float maxY) {
			this->xPos = xPos;
			this->yPos = yPos;
			this->width = width;
			this->height = height;
			this->minY = minY;
			this->maxY = maxY;
		}


		void movePaddle(float yDir) {
			yPos += yDir;
			if (yPos < minY) {
				yPos = minY;
			}
			else if (yPos + height > maxY) {
				yPos = maxY - height;
			}
		}

		void render() {
			engine.rect(xPos, yPos, width, height);
		}

		float getXPos() {return xPos;}
		float getYPos() {return yPos;}
		float getWidth() {return width;}
		float getHeight() {return height;}
};

class Ball {
	private:
		float coords[4]; //used for paddle collision test

		float startX;
		float startY;
		float xPos;
		float yPos;
		float radius;
		float xVel;
		float yVel;
		float startSpeed;
		float speed;
		float maxSpeed;
		float acc;
		float minX; //screen bounds
		float maxX;
		float minY;
		float maxY;
		int coolDown;

	public:
		Ball(float startX, float startY, float radius, float speed, float minX, float maxX, float minY, float maxY, float acc, float maxSpeed) {
			this->startX = startX;
			this->startY = startY;
			this->xPos = startX;
			this->yPos = startY;
			this->radius = radius;
			this->xVel = speed;
			this->yVel = 0;
			this->startSpeed = speed;
			this->maxSpeed = maxSpeed;
			this->speed = speed;
			this->minX = minX;
			this->maxX = maxX;
			this->minY = minY;
			this->maxY = maxY;
			this->acc = acc;
			this->coolDown = 0;
		}

		void collisionResponse(Paddle *paddle) { //respond accordingly, dir is refers to x axis
			int dir = 0;
			if (xPos < paddle->getXPos()) {
				dir = -1;
			}
			else if (xPos > paddle->getXPos() + paddle->getWidth()) {
				dir = 1;
			}

			if (dir != 0) {
				float maxDiff = ((float)paddle->getHeight() / 2) + radius;
				float diff = yPos - (((float)paddle->getHeight() / 2) + paddle->getYPos());
				float ratio = (float)(diff * 0.99) / maxDiff;
				xVel = speed * dir * sqrt(1 - (ratio * ratio));
				yVel = speed * ratio;
			}
			else { //bounce off the sides of the paddle
				yVel = -yVel;
			}
			coolDown = 5;
		}

		bool collisionTest(Paddle *paddle) { //determine if colliding
			coords[0] = paddle->getXPos();
			coords[1] = paddle->getYPos();
			coords[2] = coords[0] + paddle->getWidth();
			coords[3] = coords[1] + paddle->getHeight(); //convert coords to {min, max, min, max}
			if (xPos - radius > coords[2] || xPos + radius < coords[0] || yPos - radius > coords[3] || yPos + radius < coords[1]) { //bounding box test
				return false;
			}
			if (xPos >= coords[0] && xPos <= coords[2] && yPos >= coords[1] && yPos <= coords[3]) { //does circle center lie in rectangle
				return true;
			}
			float xDiff;
			float yDiff;
			for (int i = 0; i <= 2; i+=2) { //do any rectangle vertices lie inside the circle
				for (int j = 1; j <= 3; j+=2) {
					xDiff = coords[i] - xPos;
					yDiff = coords[j] - yPos;
					if (xDiff * xDiff + yDiff * yDiff <= radius * radius) {
						return true;
					}
				}
			}
			//potential chords formed by indexes 02 at minX, 13 at minY, 02 at maxX, 13 at maxY
			if (coords[0] <= xPos && coords[2] >= xPos) { //if chord lies on either side of center and overlaps circle
				if (coords[1] >= yPos - radius && coords[1] <= yPos + radius) {
					return true;
				}
				if (coords[3] >= yPos - radius && coords[3] <= yPos + radius) {
					return true;
				}
			}
			if (coords[1] <= yPos && coords[3] >= yPos) {
				if (coords[0] >= xPos - radius && coords[0] <= xPos + radius) {
					return true;
				}
				if (coords[2] >= xPos - radius && coords[2] <= xPos + radius) {
					return true;
				}
			}
			return false;
		}

		void handleCollision(Paddle* paddle) {
			if (coolDown == 0) {
				if (collisionTest(paddle) == true) {
					collisionResponse(paddle);
				}
			}
		}

		int update() { //determines if the ball has moved off screen
			if (coolDown != 0) {
				coolDown -= 1;
			}
			xPos += xVel;
			yPos += yVel;
			if (speed != maxSpeed) {
				speed += acc;
				if (speed > maxSpeed) {
					speed = maxSpeed;
				}
			}
			if (yPos + radius > maxY || yPos - radius < minY) {
				yVel = -yVel;
				yPos += yVel;
			}
			if (xPos + radius > maxX) {
				return 1;
			}
			if (xPos - radius < minX) {
				return -1;
			}
			return 0;
		}

		void reset() {
			xPos = startX;
			yPos = startY;
			speed = startSpeed;
			xVel = speed;
			yVel = 0;
		}

		void render() {
			engine.circle(xPos, yPos, radius);
		}
};

int player1Score = 0;
int player2Score = 0;
Paddle* paddle1 = new Paddle(-engine.getAspectRatio() + 0.1, -0.2, 0.1, 0.4, -1, 1);
Paddle* paddle2 = new Paddle(engine.getAspectRatio() - 0.2, -0.2, 0.1, 0.4, -1, 1);
Ball* ball = new Ball(-1, 0, 0.05, 0.01, -engine.getAspectRatio() - 0.1, engine.getAspectRatio() + 0.1, -1, 1, 0.000001, 0.03);

void input() {
	if (engine.keyPress(GLFW_KEY_W) == true) {
		paddle1->movePaddle(0.01);
	}
	if (engine.keyPress(GLFW_KEY_S) == true) {
		paddle1->movePaddle(-0.01);
	}
	if (engine.keyPress(GLFW_KEY_UP) == true) {
		paddle2->movePaddle(0.01);
	}
	if (engine.keyPress(GLFW_KEY_DOWN) == true) {
		paddle2->movePaddle(-0.01);
	}
}

void renderAll() {
	paddle1->render();
	paddle2->render();
	ball->render();
	engine.renderString(-engine.getAspectRatio() + 0.5, 0.7, to_string(player1Score));
	engine.renderString(engine.getAspectRatio() - 0.7, 0.7, to_string(player2Score));
}

void updateBall() {
	ball->handleCollision(paddle1);
	ball->handleCollision(paddle2);
	int status = ball->update();
	if (status == 1) {
		player1Score += 1;
		ball->reset();
	}
	else if (status == -1) {
		player2Score += 1;
		ball->reset();
	}
}

int main() { //for now the coordinates system goes from (-aspectRatio, -1) to (aspectRatio, 1)
	engine.resizeText(0.1);
	while (engine.shouldClose() == false) {
		input();
		updateBall();
		renderAll();
		engine.clear();
		if (engine.keyPress(GLFW_KEY_ESCAPE) == true) {
			engine.closeWindow();
		}
	}
}