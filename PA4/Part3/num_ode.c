#include <stdbool.h>
#include <stdio.h>

#include "lina.h"
#include "num_ode.h"


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

void symplectic_euler (void (*f)(double,struct Vector*,struct Vector*), double t0, struct Vector* y0, double h, double* t, struct Vector** y, int steps) {
	if (DIM%2 != 0) printf("Warning: Tried to use symplectic Euler method for vector of odd dimension");
	else {	
		t[0] = t0;
		y[0] = y0;
		struct Vector* tmp;
		for (int j=1; j<steps; j++) {
			t[j] = t0+j*h;
			tmp = new_Vector(DIM);
			f(t[j-1],y[j-1],tmp);
			//print_Vector(tmp);
			copy_Vector(y[j-1],y[j]);
			for(int i=DIM/2; i<DIM; i++) {
				y[j]->values[i] = y[j-1]->values[i] + h*tmp->values[i];
			}
			//print_Vector(y[j]);
			f(t[j-1],y[j],tmp);
			//print_Vector(tmp);
			for(int i=0; i<DIM/2; i++) {
				y[j]->values[i] = y[j-1]->values[i] + h*tmp->values[i];
			}
			//print_Vector(y[j]);
			delete_Vector(tmp);
		}
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
				if (3/2*tol/norm>0) {
					h = 3/2*tol/norm;
				} else {
					h = h/2;
				}
			}
		}
		t[j] = t[j-1]+h;

		delete_Vector(tmp1);
		delete_Vector(tmp2);
		delete_Vector(tmp3);
		delete_Vector(k1);
		delete_Vector(k2);
		delete_Vector(k3);
	}
}
