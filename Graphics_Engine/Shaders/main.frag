#version 460

in vec3 col;
in vec2 uv;
in vec3 vertexOut;
layout(location = 0) out vec4 fragCol;


uniform sampler2D textureImg;
uniform vec2 resolution;

//PATHTRACING UNIFORMS
uniform float vh;
uniform float vw;
//bottom left corner
uniform float fl;


//STRUCTS
struct Sphere{
	vec3 center;
	float radius;
	vec3 color;
	bool isLight;
};

struct Rect{
	vec2 bottomLeft;
	vec2 topRight;
	float size;
	vec3 color;
	bool isLight;
	int type; // 0 = xy, 1 = xz, 2 = yz
};


struct Ray{
	vec3 origin;
	vec3 direction;
};

struct Hit{
	vec3 position;
	vec3 normal;
	vec3 emission;
	float t;
	bool front;
	bool hitLight;
};

struct Camera{
	vec3 up;
	vec3 right;
	vec3 forward;
	vec3 position;
	float fov;
	float focalDist;
	float aperture;
};

uniform Camera camera;
uniform vec2 seedVector;
uniform float time;
vec2 seed;

#define PI 3.14159265359
#define INFINITY 100000000.0
#define EPS = 0.001
#define hash21(p) fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453)
#define hash33(p) fract(sin( (p) * mat3( 127.1,311.7,74.7 , 269.5,183.3,246.1 , 113.5,271.9,124.6) ) *43758.5453123)

float rand(){
	seed -= vec2(seedVector.x * seedVector.y);
	return hash21(seed);
}

vec2 hash2( const float n ) {
	return fract(sin(vec2(n,n+1.))*vec2(43758.5453123));
}

vec3 randomVec3(){
	seed -= vec2(seedVector.x * seedVector.y);
	return vec3(hash21(seed));
}


// https://www.shadertoy.com/view/lsX3DH
vec3 cosWeightedRandomHemisphereDirection2( const vec3 n ) {
	vec2 seed_vec2 = vec2(randomVec3().xy);
	vec3  uu = normalize( cross( n, vec3(0.0,1.0,1.0) ) );
	vec3  vv = cross( uu, n );
	
	float ra = sqrt(seed_vec2.y);
	float rx = ra*cos(6.2831*seed_vec2.x); 
	float ry = ra*sin(6.2831*seed_vec2.x);
	float rz = sqrt( 1.0-seed_vec2.y );
	vec3  rr = vec3( rx*uu + ry*vv + rz*n );

    return normalize( rr );
}

void set_face_normal(inout Ray ray, vec3 normal, inout Hit hit){
	hit.front = dot(ray.direction, normal) < 0.;
	hit.normal = hit.front ? normal : -normal;
}

vec3 at(inout Ray r, float t){
	return r.origin + t*r.direction;
}

vec3 getBackgroundColor( const vec3 ro, const vec3 rd ) {	
	return 1.4*mix(vec3(.5),vec3(.7,.9,1), .5+.5*rd.y);
}

bool xy_rect_hit(Rect rect, Ray ray, inout Hit hit){
	float t = (rect.size - ray.origin.z / ray.direction.z);

	if(t < 0.001 || t > 10000.){
		return false;
	}

	float x = ray.origin.x + t * ray.direction.x;
	float y = ray.origin.y + t * ray.direction.y;
	vec2 p = vec2(x,y);
	if(p.x < rect.bottomLeft.x || p.x > rect.topRight.x || p.y < rect.bottomLeft.y || p.y > rect.topRight.y){
		return false;
	}
	vec3 normal = vec3(0.,0.,1.);
	hit.t = t;
	set_face_normal(ray, normal, hit);
	hit.emission = rect.color;
	hit.position = at(ray, t);
	hit.hitLight = rect.isLight;
	return true;
}

bool xz_rect_hit(Rect rect, Ray ray, inout Hit hit){
	float t = (rect.size - ray.origin.z / ray.direction.z);

	if(t < 0.001 || t > 10000.){
		return false;
	}

	float x = ray.origin.x + t * ray.direction.x;
	float z = ray.origin.z + t * ray.direction.z;
	vec2 p = vec2(x,z);
	if(p.x < rect.bottomLeft.x || p.x > rect.topRight.x || p.y < rect.bottomLeft.y || p.y > rect.topRight.y){
		return false;
	}
	vec3 normal = vec3(0.,1.,0.);
	hit.t = t;
	set_face_normal(ray, normal, hit);
	hit.emission = rect.color;
	hit.position = at(ray, t);
	hit.hitLight = rect.isLight;
	return true;
}

bool yz_rect_hit(Rect rect, Ray ray, inout Hit hit){
	float t = (rect.size - ray.origin.z / ray.direction.z);

	if(t < 0.001 || t > 10000.){
		return false;
	}

	float y = ray.origin.y + t * ray.direction.y;
	float z = ray.origin.z + t * ray.direction.z;
	vec2 p = vec2(y,z);
	if(p.x < rect.bottomLeft.x || p.x > rect.topRight.x || p.y < rect.bottomLeft.y || p.y > rect.topRight.y){
		return false;
	}
	vec3 normal = vec3(1.,0.,0.);
	hit.t = t;
	set_face_normal(ray, normal, hit);
	hit.emission = rect.color;
	hit.position = at(ray, t);
	hit.hitLight = rect.isLight;
	return true;
}



bool sphere_hit(Sphere sphere, Ray ray, inout Hit hit){
	
	vec3 oc = sphere.center - ray.origin;
	float b = dot(oc, ray.direction);
	float d = b * b - dot(oc,oc) + sphere.radius * sphere.radius;
	float epsilon = 0.0001;
	if(d < 0.){
		return false;
	}

	d = sqrt(d);
	float t1 = b - d;

	if(t1 > epsilon){
		hit.emission = sphere.color;
		hit.t = t1;
		hit.position = at(ray, t1);
		vec3 normal = (hit.position - sphere.center) / sphere.radius;
		set_face_normal(ray, normal, hit);
		hit.hitLight = sphere.isLight;

		return true;
	}

	float t2 = b + d;
	if( t2 > epsilon){
		hit.emission = sphere.color;
		hit.t = t2;
		hit.position = at(ray, t2);
		vec3 normal = (hit.position - sphere.center) / sphere.radius;
		set_face_normal(ray, normal, hit);
		hit.hitLight = sphere.isLight;

		return true;
	}

	return false;
}

bool cornell_box_hit(inout Ray ray, inout Hit hit){

	// CORNELL BOX
	vec3 red = vec3(.65, .05, .05);
	vec3 green = vec3(.12, .45, .15);
	vec3 white = vec3(.73);
	vec3 light = vec3(1.);


	Rect r1 = Rect(vec2(0.5,-.5), vec2(1.,-2.), 1., green, false, 2);
	Rect r2 = Rect(vec2(-0.,-1), vec2(1.,-2.), 0, red, false, 2);
	Rect r3 = Rect(vec2(-1.,-0.5), vec2(1.,1.), 1.5, white, false, 1);
	Rect r4 = Rect(vec2(-1.,-0.5), vec2(1.,1.), 1.5, white, false, 1);
	Rect r5 = Rect(vec2(-1.,0), vec2(1.,1.), 2, red, false, 0);
	Rect rectLight = Rect(vec2(-0,-1), vec2(1.1,1.2), .1, light, true, 1);


	Rect[] scene = Rect[](r1, r2, r3);

	float closest_hit = INFINITY;
	bool has_hit = false;

	// 0 = xy, 1 = xz, 2 = yz
	for(int i = 0; i < scene.length(); i++){
		Hit tmp;
		if(scene[i].type == 0){
			if(xy_rect_hit(scene[i],ray, tmp)){
				has_hit = true;
				if(tmp.t < closest_hit){
					closest_hit = tmp.t;
					hit.t = tmp.t;
					hit.normal = tmp.normal;
					hit.position = tmp.position;
					hit.emission = tmp.emission;
					hit.hitLight = tmp.hitLight;
				}
			}
		}else if(scene[i].type == 1){
			if(xz_rect_hit(scene[i],ray, tmp)){
				has_hit = true;
				if(tmp.t < closest_hit){
					closest_hit = tmp.t;
					hit.t = tmp.t;
					hit.normal = tmp.normal;
					hit.position = tmp.position;
					hit.emission = tmp.emission;
					hit.hitLight = tmp.hitLight;
				}
			}		
		}else{
			if(yz_rect_hit(scene[i],ray, tmp)){
				has_hit = true;
				if(tmp.t < closest_hit){
					closest_hit = tmp.t;
					hit.t = tmp.t;
					hit.normal = tmp.normal;
					hit.position = tmp.position;
					hit.emission = tmp.emission;
					hit.hitLight = tmp.hitLight;
				}
			}		
		}
	}
	return has_hit;
}


bool scene_hit(inout Ray ray, inout Hit hit){
	Sphere s1 = Sphere(vec3(0.,0., -1.), 0.5, vec3(1., 0.0, 0.3), false);
	Sphere s2 = Sphere(vec3(0., -100.4, -1.), 100., vec3(0.7), false);
	Sphere s3 = Sphere(vec3(0., .5, -.2), 0.1, vec3(5.), true);

	Sphere[] scene = Sphere[](s1, s2, s3);

	//Rect light = Rect( vec2(-0.5.,0.5), vec2(1.,1.), 0.5, vec3(2., 2., 2.), true);
	//Rect light = Rect(vec2(-2., -.1), vec2(3, 10), 0.5, vec3(1.,1.,1.), true);


	float closest_hit = INFINITY;
	bool has_hit = false;


	for(int i = 0; i < scene.length(); i++){
		Hit tmp;
		if(sphere_hit(scene[i],ray, tmp)){
			has_hit = true;
			if(tmp.t < closest_hit){
				closest_hit = tmp.t;
				hit.t = tmp.t;
				hit.normal = tmp.normal;
				hit.position = tmp.position;
				hit.emission = tmp.emission;
				hit.hitLight = tmp.hitLight;
			}
		}
	}
	return has_hit;
	
}

vec3 ray_color(Ray ray){

	Hit hit;
	int maxDepth = 50;
	vec3 col = vec3(0.);
	vec3 pxl = vec3(1.);
	bool bg_hit = false;



	for(int depth = 0; depth < maxDepth; depth++){
		bool h = scene_hit(ray,hit);
		//bool h = cornell_box_hit(ray,hit);

		if(hit.hitLight){
			pxl *= hit.emission;
			bg_hit = true;
			break;
		}
		if(!h){
			// background
			pxl *= getBackgroundColor(ray.origin, ray.direction);
			//pxl *= vec3(0.);
			bg_hit = true;
			break;
		}else{
			pxl *= hit.emission;
			ray.direction = cosWeightedRandomHemisphereDirection2(hit.normal);
			ray.origin = hit.position + 0.0001 * ray.direction;
		}
	}
	pxl = sqrt(clamp(pxl, 0., 1.));
	if(bg_hit){
		col += pxl;
	}
	return col;

}

void main(){
	// seed calc for random unit hemisphere from: https://www.shadertoy.com/view/lsX3DH
	vec2 q = gl_FragCoord.xy/resolution.xy;
	vec2 p = -1.0+2.0*q;
	p.x *= resolution.x/resolution.y;
	float s = time+(p.x+resolution.x*p.y)*1.51269341231;
	seed = hash2(24.4316544311+s);


	// add 'jitter' to emulate supersampling. Write output to tex and read from tex to add to existing pic
	float r1 = 2. * rand();
	float r2 = 2. * rand();

	vec2 jitter;
	jitter.x = r1 < 1. ? sqrt(r1) - 1. : 1. - sqrt(2. - r1);
	jitter.y = r2 < 1. ? sqrt(r2) - 1. : 1. - sqrt(2. - r2);
	jitter /= (resolution * 0.5);

	vec2 d = (2.0 * uv - 1.) + jitter;
	d.x *= resolution.x / resolution.y * tan(camera.fov / 2.0);
	d.y *= tan(camera.fov / 2.0);

	vec3 rayDir = normalize(d.x * camera.right + d.y * camera.up + camera.forward);

	Ray ray = Ray(camera.position, rayDir);

	vec3 rayCol = ray_color(ray);

	fragCol = vec4(rayCol, 1.0);

}
