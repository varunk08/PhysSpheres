#ifndef _OBJECTS_H_
#define _OBJECTS_H_
#include <GL/glut.h>
#include <gmtl/gmtl.h>
#include <vector>

using namespace gmtl;

// A very simple plane class. The plane is defined as a point and normal.
// Many routines depend on geometric primitives from gmtl
class plane
{
public:
	Vec3d N;
    Vec3d p;
	double K;

	plane() {;} // default constructor
	plane( Vec3d normal, Vec3d position, double wallSpring = 1000.0)
	{
		N = normal;
		p = position;
		K = wallSpring;
	}
    // Draw the plane. A plane has infinite extend but we will draw it bounded according to width
    void draw(double width = 1.0)
	{
	    // The hard thing is to create a full local coordinate system so
	    // we can draw the corners. First, pick a vector to cross with, then
	    // make the full 3 ortho vectors.
		Vec3d vec1, vec2;
		vec1[0] = vec1[1] = vec1[2] = 0.0;
		if ( N[2] > 0.8 )
			vec1[1] = 1.0;
		else
			vec1[2] = 1.0;
		cross( vec2, vec1, N );
		gmtl::normalize( vec2 );
	    cross( vec1, vec2, N );

	    Vec3d A, B, C, D, tmp;

	    // Find the corners A,B,C,D
		tmp = p + vec1 * width;
		A = tmp + vec2 * width;
		B = tmp - vec2 * width;
		tmp = p - vec1 * width;
		C = tmp + vec2 * width;
		D = tmp - vec2 * width;

	    // Draw the filled plane. I actually draw the corners counter-clockwise
	    // from what they should be so that they disappear with back face culling on.
	    // This is so you always see the back side of the box, no matter how it is rotated.
		glEnable(GL_POLYGON_OFFSET_FILL); // This is a hack to make the filled box faces not overlap with the drawn edges
		glPolygonOffset(1.0, 1.0);
		glBegin(GL_QUADS);
	    glVertex3d( A[0], A[1], A[2] ); // ahh, really should use the array form of glVertex here
	    glVertex3d( C[0], C[1], C[2] );
	    glVertex3d( D[0], D[1], D[2] );
	    glVertex3d( B[0], B[1], B[2] );
	    glEnd();
		glDisable(GL_POLYGON_OFFSET_FILL);

	    // Draw the plane outlined as well.
	    glLineWidth(5.0);
	    glDisable(GL_LIGHTING);
	    glBegin(GL_LINE_LOOP);
	    glVertex3d( A[0], A[1], A[2] );
	    glVertex3d( B[0], B[1], B[2] );
	    glVertex3d( D[0], D[1], D[2] );
	    glVertex3d( C[0], C[1], C[2] );
	    glEnd();
	    glEnable(GL_LIGHTING);
	    glLineWidth(1.0);

	}
};

// A simple Sphere class - a particle just has a zero radius
// This is really the main object in the world
class sphere
{
public:
    Vec3d p; // position
    Vec3d v; // velocity
	double mass; // mass
	double r; // radius
	double K; // Penalty spring constant
	bool fixed; // if fixed, don't update its position
    Vec3d f; // force to be applied
	float _fixedColor[3];
	float _collisionColor[3];
	bool colliding;
	// A basic constructor
	sphere(double rad = 0.0, double bounceScale = 1000.0)
	{
		v.set(0.0, 0.0, 0.0);
		mass = 0.1;
		r = rad;
		fixed = false;
		K = bounceScale;
		colliding = false;
		_fixedColor[0] = 1; _fixedColor[1] = 0; _fixedColor[2] = 0;
		_collisionColor[0] = 0; _collisionColor[1] = 1; _collisionColor[2] = 1;
	
	}
	// Self-modifies to have a random position
	void makeRandomSphere( double wall = 1.0, double rad = 0.05)
	{
		p[0] = 2.0 * wall * (rand() / (double)RAND_MAX) - wall;
		p[1] = 2.0 * wall * (rand() / (double)RAND_MAX) - wall;
		p[2] = 2.0 * wall * (rand() / (double)RAND_MAX) - wall;
		r = rad;

		v[0] = v[1] = v[2] = 0.0;
		mass = 0.1;
	}

	// Clear out any accumulated forces.
	void clearForce()
	{
		f.set(0.0, 0.0, 0.0);
	}

	// Compute gravitational force = mass * g (g is a vector) and accumulate it in the force vector.
	void accumulateGravity( Vec3d g )
	{
		f += g * mass;
	}

	// Compute viscous air resistance and accumulate it in the force vector.
	void accumulateDrag( double b )
	{
		f += -b * v;
	}

    // Compute the signed distance between a sphere and plane. The plane normal determines
    // which side is 'inside'. When they are colliding a negative distance is returned.
    double distSpherePlane( const plane &P)
	{
		Vec3d vec = p - P.p;
	    return dot( vec, P.N ) - r;
	}

    // compute the signed distance between two spheres.
    double distSphereSphere( const sphere &s )
	{
		Vec3d separation = p - s.p;
		// Can't do squared distance optimization here because we want to use a negative distance as spring
	    return length(separation) - r - s.r;
	}

	// Compute forces for penetrating into a wall.
	void accumulatePlaneContact( const plane & wall )
	{
		double dist = distSpherePlane(wall);

		if ( dist <  0.0 )
		{
			double fMag = -dist * wall.K; // force is -Kx
			f += wall.N * fMag; // force is in the wall normal direction - add it to the existing force
		}
	}

    // add to current force the penalty of colliding with another sphere.
	void accumulateSphereContact( const sphere & s )
	{
		double dist = distSphereSphere(s);

		if ( dist <  0.0 )
		{
			// force is the magnitude of the force
			double force = dist * s.K; // force is the -Kx
			// N is the direction of the force
			Vec3d N;
			N = p - s.p; // the vector between sphere centers is the force direction
			normalize( N );
			N *= -force; // Scale the direction by the magnitude
			f += N; // We are accumulating forces so add it into the existing forces found
		}
	}

	// Compute all the forces between a sphere and the walls and other spheres and gravity and drag.
	void computeForces(Vec3d gravity, double dragConstant, plane walls[6], std::vector< sphere > & spheres)
	{
		clearForce();
		accumulateGravity( gravity );
		accumulateDrag( dragConstant );
		// Now check for collisions with the box walls
		for ( int j = 0; j < 6; j++ )
			accumulatePlaneContact(walls[j]);
		//Check for collisions with external spheres in the environment

		// This needs to be toggled to use a list of spheres for the grid

		for ( unsigned int j = 0; j < spheres.size(); j++ )
			if ( this != &spheres[j] ) // Don't collide with yourself
				accumulateSphereContact( spheres[j] );
	}
	// Compute all the forces between a sphere and the walls and other spheres and gravity and drag.
	void computeForcesWithNeighbors(Vec3d gravity, double dragConstant, plane walls[6], std::vector< sphere* > & spheres)
	{
		clearForce();
		accumulateGravity(gravity);
		accumulateDrag(dragConstant);
		// Now check for collisions with the box walls
		for (int j = 0; j < 6; j++)
			accumulatePlaneContact(walls[j]);
		//Check for collisions with external spheres in the environment

		// This needs to be toggled to use a list of spheres for the grid

		for (unsigned int j = 0; j < spheres.size(); j++){
			if (this != spheres[j]) // Don't collide with yourself
				accumulateSphereContact(*spheres[j]);
			if (this->fixed){
				spheres[j]->colliding = true;
			}
		}
	}
	// Perform Euler-Cromer integration using the accumulated force stored in the sphere.
	void EulerCromer( double deltat)
	{
		if ( !fixed ) // fixed spheres do not move
		{
			v = v + f/mass * deltat;
			p = p + v * deltat;
		}
	}
	void Euler(double deltat)
	{
		if (!fixed) // fixed spheres do not move
		{
			p = p + v * deltat;
			v = v + f / mass * deltat;
		
		}
	}

    // Draw the particle as a small sphere using glutSolidSphere. Particles with zero radius are given a small size.
    void draw()
	{
	    glPushMatrix();
		float currentColor[4];
		glGetFloatv(GL_CURRENT_COLOR, currentColor);
		if (colliding) glColor4f(_collisionColor[0], _collisionColor[1], _collisionColor[2], 1);
		if (fixed) glColor4f(_fixedColor[0], _fixedColor[1], _fixedColor[2], 1);
		
	    glTranslated(p[0],p[1],p[2]);
		if ( r < 0.0001)
		    glutSolidSphere( 0.01, 5, 5 );
		else
			glutSolidSphere( r, 12, 12 );
		glColor4f(currentColor[0], currentColor[1], currentColor[2], 1);
	    glPopMatrix();
		colliding = false;
	}
};

#endif _OBJECTS_H_