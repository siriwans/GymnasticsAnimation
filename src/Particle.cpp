//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

void Particle::load()
{
	// Random initialization
	rebirth(0.0f);
}

// all particles born at the origin
void Particle::rebirth(float t)
{
	charge = randFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
	m = 1.0f;
	d = randFloat(0.0f, 0.02f);
	x.x = 0;
	x.y = 0;
	x.z = 0; // change to 0
	v.x = randFloat(-0.3f, 0.3f);
	v.y = randFloat(-0.7f, -0.3f);//making it shoot upward initially
	v.z = randFloat(-0.2f, 0.2f);
	lifespan = randFloat(0.0f, 150.f);

	tEnd = t + lifespan;

	scale = randFloat(0.01f, 0.05f);
	color.r = randFloat(0.9f, 1.0f);
	color.g = randFloat(0.9f, 1.0f);
	color.b = randFloat(0.9f, 1.0f);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const bool *keyToggles)
{
	if (t > tEnd)
	{
		rebirth(t);
	}
	// very simple update
   v.y -= 18.0f * 0.0001;
	x += h * v;
	color.a = (tEnd - t) / lifespan;
   
   lifespan -= 0.5;

   /*if (lifespan <= 5)
   {
      rebirth(10);
   }*/
}
