#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "lina.h"

#define DIM 1

//Define Functions f for Right side of ODE/IVP
void f1 (double t, struct Vector* vec, struct Vector* res) {
	scale_Vector(-t,vec,res);
}

void f2 (double t, struct Vector* vec, struct Vector* res) {
	res->values[0] = 1/(1-t);
}


//Define Euler, ModEuler and Heun Single Step Schemes
void euler (void (*f)(double,struct Vector*,struct Vector*), double t0, struct Vector* y0, double h, double* t, struct Vector** y, int steps) {	
	t[0] = t0;
	y[0] = y0;
	struct Vector* tmp1;
	struct Vector* tmp2;
	for (int j=1; j<steps; j++) {
		t[j] = t0+j*h;
		tmp1 = new_Vector(DIM);
		tmp2 = new_Vector(DIM);
		f(t[j-1],y[j-1],tmp1);
		scale_Vector(h,tmp1,tmp2);
		add_Vectors(y[j-1],tmp2,y[j]);
		delete_Vector(tmp1);
		delete_Vector(tmp2);
	}
}


void modEuler (void (*f)(double,struct Vector*,struct Vector*), double t0, struct Vector* y0, double h, double* t, struct Vector** y, int steps) {
	t[0] = t0;
	y[0] = y0;
	struct Vector* tmp1;
	struct Vector* tmp2;
	for (int j=1; j<steps; j++) {
		t[j] = t0+j*h;
		tmp1 = new_Vector(DIM);
		tmp2 = new_Vector(DIM);
		f(t[j-1],y[j-1],tmp1);
		scale_Vector(h/2.0,tmp1,tmp2);
		add_Vectors(y[j-1],tmp2,tmp1);
		f(t[j-1]+h/2.0,tmp1,tmp2);
		scale_Vector(h,tmp2,tmp1);
		add_Vectors(y[j-1],tmp1,y[j]);
		delete_Vector(tmp1);
		delete_Vector(tmp2);
	}
}

void heun (void (*f)(double,struct Vector*,struct Vector*), double t0, struct Vector* y0, double h, double* t, struct Vector** y, int steps) {
	t[0] = t0;
	y[0] = y0;
	struct Vector* tmp1;
	struct Vector* tmp2;
	struct Vector* tmp3;
	for (int j=1; j<steps; j++) {
		t[j] = t0+j*h;
		tmp1 = new_Vector(DIM);
		tmp2 = new_Vector(DIM);
		tmp3 = new_Vector(DIM);
		f(t[j-1],y[j-1],tmp1);
		scale_Vector(h,tmp1,tmp2);
		add_Vectors(y[j-1],tmp2,tmp1);
		f(t[j],tmp1,tmp2);
		f(t[j-1],y[j-1],tmp1);
		add_Vectors(tmp1,tmp2,tmp3);
		scale_Vector(h/2.0,tmp3,tmp1);
		add_Vectors(y[j-1],tmp1,y[j]);
		delete_Vector(tmp1);
		delete_Vector(tmp2);
		delete_Vector(tmp3);
	}
}

void adaptive_rk3 (void (*f)(double,struct Vector*,struct Vector*), double t0, struct Vector* y0, double tol, double* t, struct Vector** y, int steps) {
	t[0] = t0;
	y[0] = y0;
	double h;
	double norm;
	struct Vector* k1;
	struct Vector* k2;
	struct Vector* k3;
	struct Vector* tmp1;
	struct Vector* tmp2;
	struct Vector* tmp3;
	for (int j=1; j<steps; j++) {
		bool adapt = true;
		h = 1;
		k1 = new_Vector(DIM);
		k2 = new_Vector(DIM);
		k3 = new_Vector(DIM);
		tmp1 = new_Vector(DIM);
		tmp2 = new_Vector(DIM);
		tmp3 = new_Vector(DIM);
		while(adapt) {
			f(t[j-1],y[j-1],k1);
			scale_Vector(h,k1,tmp1);
			add_Vectors(y[j-1],tmp1,tmp2);
			f(t[j-1]+h,tmp2,k2);
			add_Vectors(k1,k2,tmp1);
			scale_Vector(h/4,tmp1,tmp2);
			add_Vectors(y[j-1],tmp2,tmp1);
			f(t[j-1]+h/2,tmp1,k3);
			scale_Vector(2,k3,tmp1);
			scale_Vector(-1,k2,tmp2);
			add_Vectors(tmp1,tmp2,tmp3);
			scale_Vector(-1,k1,tmp1);
			add_Vectors(tmp1,tmp3,tmp2);
			norm = vectornorm(tmp2);
			if (h/3*norm < tol) {
				adapt = false;
				add_Vectors(k1,k2,tmp1);
				scale_Vector(4,k3,tmp2);
				add_Vectors(tmp1,tmp2,tmp3);
				scale_Vector(h/6,tmp3,tmp1);
				add_Vectors(y[j-1],tmp1,y[j]);
			} else {
				//printf("failed with size: %f\n",h);
				//printf("error estimate: %f\n",h/3*norm);
				if (3/2*tol/norm>0) {
					h = 3/2*tol/norm;
					//printf("new step size: %f\n",h);
				} else {
					h = h/2;
				}
			}
		}
		//printf("used step size: %f\n",h);
		t[j] = t[j-1]+h;

		delete_Vector(tmp1);
		delete_Vector(tmp2);
		delete_Vector(tmp3);
		delete_Vector(k1);
		delete_Vector(k2);
		delete_Vector(k3);
	}
}


//Define Plot Methods
void plotFunction (double (*f)(double), double xmin, double xmax, double ymin, double ymax, double xpos, double ypos, double width, double height, double stepsize) {
	double xdiff = xmax-xmin;
	double xfac = width/xdiff;
	double ydiff = ymax-ymin;
	double yfac = height/ydiff;
	double x = xmin;
	double y;
	glBegin(GL_LINE_STRIP);
		y = f(x);
		glVertex3f(xpos,(y-ymin)*yfac+ypos,0.0f);
		while (x < xmax-stepsize) {
			x += stepsize;
			y = f(x);
			glVertex3f((x-xmin)*xfac+xpos,(y-ymin)*yfac+ypos,0.0f);
		}	
		y = f(xmax);
		glVertex3f(xpos+width,(y-ymin)*yfac+ypos,0.0f);	
	glEnd();
}

void plotArray (double* x, double* y, int size, double xmin, double xmax, double ymin, double ymax, double xpos, double ypos, double width, double height) {
	double xdiff = xmax-xmin;
	double xfac = width/xdiff;
	double ydiff = ymax-ymin;
	double yfac = height/ydiff;
	glBegin(GL_LINE_STRIP);
		for (int j=0; j<size; j++) {
			glVertex3f((x[j]-xmin)*xfac+xpos,(y[j]-ymin)*yfac+ypos,0.0f);
		}	
	glEnd();
}


//Run Program
int main (int argc, char** argv) {
	
	//Read input argument
	int in = 0;
	double h = 0.5;
	double tol = 0.1;
	if (argc > 1) in = atoi(argv[1]);	
	if (argc > 2) h = atof(argv[2]);	
	if (argc > 3) tol = atof(argv[3]);	
	printf("Input arguments: %i %f %f \n", in, h, tol);
	
	//Check if SDL Initialization is successful
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		return -1;
	}

	//Create Window
	SDL_Window *win = SDL_CreateWindow("Adaptive Step Sizes",50,50,640,480,SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!win) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
	}
	
	//Initialize OpenGL
	SDL_GLContext context = SDL_GL_CreateContext(win);
	if (!context) {
		fprintf(stderr, "Could not create OpenGL context: %s\n", SDL_GetError());
	}
	glViewport(0,0,(GLsizei)640,(GLsizei)480);
	glClearColor(1.0f,1.0f,1.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(win);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

	//Plot Functions
	double xpos = -0.9;
	double ypos = -0.9;
	double width = 1.8;
	double height = 1.8;
	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(xpos,ypos,0.0f);
		glVertex3f(xpos+width,ypos,0.0f);
		glVertex3f(xpos+width,ypos+height,0.0f);
		glVertex3f(xpos,ypos+height,0.0f);
	glEnd();
	glBegin(GL_LINES);
		for (int j=1; j<10; j++) {
		glVertex3f(xpos+j*width/10,ypos,0.0f);
		glVertex3f(xpos+j*width/10,ypos+0.02f,0.0f);
		glVertex3f(xpos+j*width/10,ypos+height,0.0f);
		glVertex3f(xpos+j*width/10,ypos+height-0.02f,0.0f);
		glVertex3f(xpos,ypos+j*height/10,0.0f);
		glVertex3f(xpos+0.02f,ypos+j*height/10,0.0f);
		glVertex3f(xpos+width,ypos+j*height/10,0.0f);
		glVertex3f(xpos+width-0.02f,ypos+j*height/10,0.0f);
		}
	glEnd();
	double xmin,xmax,ymin,ymax,t0;
	struct Vector* y0;
	int steps;
	void (*f)(double,struct Vector*,struct Vector*);
	switch(in) {
		case 0:
			xmin = 0.0;
			xmax = 20;
			ymin = 0.0;
			ymax = 1.2;
			t0 = 0;
			y0 = new_Vector(DIM);
			y0->values[0] = 1;
			steps = (int)floor((xmax-xmin)/h+1);
			f = f1;
		break;

		case 1:
			xmin = 0.0;
			xmax = 0.999;
			ymin = 0;
			ymax = 15;
			t0 = 0;
			y0 = new_Vector(DIM);
			y0->values[0] = 0;
			steps = (int)floor((xmax-xmin)/h+1);
			f = f2;
		break;
		
		default:
			printf("Invalid argument: %i\n",in);
		break;	
	}

	glColor4f(0.0f,0.0f,0.0f,1.0f);

	double* t = malloc(sizeof(double)*steps);
	double* values = malloc(sizeof(double)*steps);
	struct Vector** y = malloc(sizeof(struct Vector*)*steps);	
	for (int j=1; j < steps; j++) {
		y[j] = new_Vector(DIM);
	}

	/*	
	euler (f, t0, y0, h, t, y, steps);
	glColor4f(1.0f,0.0f,0.0f,1.0f);
	for (int j=0; j < steps; j++) {
		values[j] = y[j]->values[0];
	}
	plotArray(t, values, steps, xmin, xmax, ymin, ymax, xpos, ypos, width, height);
	*/
	
	/*
	modEuler (f, t0, y0, h, t, y, steps);
	glColor4f(0.0f,0.0f,1.0f,1.0f);
	for (int j=0; j < steps; j++) {
		values[j] = y[j]->values[0];
	}
	plotArray(t, values, steps, xmin, xmax, ymin, ymax, xpos, ypos, width, height);
	*/
			
	heun (f, t0, y0, h, t, y, steps);
	glColor4f(0.0f,1.0f,0.0f,1.0f);
	for (int j=0; j < steps; j++) {
		values[j] = y[j]->values[0];
	}
	plotArray(t, values, steps, xmin, xmax, ymin, ymax, xpos, ypos, width, height);
	
	adaptive_rk3 (f, t0, y0, tol, t, y, steps);
	glColor4f(1.0f,0.0f,0.0f,1.0f);
	for (int j=0; j < steps; j++) {
		values[j] = y[j]->values[0];
	}
	plotArray(t, values, steps, xmin, xmax, ymin, ymax, xpos, ypos, width, height);
	
	//Clean Up Memory					
	free(t);
	for (int j=1; j < steps; j++) {
		delete_Vector(y[j]);
	}
	free(y);
	free(values);
	delete_Vector(y0);

	SDL_GL_SwapWindow(win);

	//Save PLot as BMP
	/*
	unsigned char* pixels = malloc(640*480*4*sizeof(unsigned char));
	glReadPixels(0,0,640,480,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
	SDL_Surface* sur = SDL_CreateRGBSurfaceFrom(pixels,640,480,8*4,640*4,0x000000FF,0x0000FF00,0x00FF0000,0);
	SDL_SaveBMP(sur,"test.bmp");
	SDL_FreeSurface(sur);
	free(pixels);
	*/
	

	
	//Wait for Window Close Event
	SDL_Event event;
	while (SDL_WaitEvent(&event) >= 0) {
		if (event.type == SDL_QUIT) {
			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(win);
			SDL_Quit();
			break;
		}
	}
	return 0;
}
