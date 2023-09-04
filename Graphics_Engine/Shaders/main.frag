#version 460

in vec2 uv;
in vec3 vertexOut;
layout(location = 0) out vec4 fragCol;
layout (std430, binding = 2) buffer spatialization_data
{
	float sphere_coords[3];
};

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
	bool isAudio;
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
	bool hitAudio;
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
float seed;
Sphere lightSphere;
Sphere detectionSphere;

Sphere[3] lights;


// AUDIO
bool sampledAudio = false;

vec3 outDir = vec3(0.);
Sphere audioSphere;
float elevation;
float azimuth;
float minDistToAudioSource = 100;

float totalPathLength = 0.0;

// DEFINES

#define PI 3.14159265359
#define INFINITY 100000000.0
#define EPS 0.001
#define hash21(p) fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453)
#define hash33(p) fract(sin( (p) * mat3( 127.1,311.7,74.7 , 269.5,183.3,246.1 , 113.5,271.9,124.6) ) *43758.5453123)

#define SAMPLES 12
#define PATHDEPTH 8

#define LIGHT vec3(10.)

#define WHITE vec3(0.8)
#define GREEN vec3(0., 0.8, 0.1)
#define RED vec3(0.8, 0., 0.1)

// direct light sampling
#define DLS

// direct audio sampling
#define DAS

vec2 hash2( inout float s ) {
	return fract(sin(vec2(s+=0.1,s+=0.1))*vec2(43758.5453123,22578.1459123));
}

float rand(){
	return hash21(hash2(seed));
}

vec3 randomSphereDirection(inout float s){
	 vec2 h = hash2(s) * vec2(2.,6.28318530718)-vec2(1,0);
    float phi = h.y;
	return vec3(sqrt(1.-h.x*h.x)*vec2(sin(phi),cos(phi)),h.x);
}


// https://www.shadertoy.com/view/lsX3DH
vec3 cosWeightedRandomHemisphereDirection2( const vec3 n ) {
	vec2 seed_vec2 = hash2(seed);
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

vec3 at(Ray r, float t){
	return r.origin + t*r.direction;
}

vec2 cartesianToSpherical(vec3 p){
	int elev = 0;
	int az = 0;

	float r = sqrt( p.x*p.x + p.y*p.y + p.z*p.z);
	float incl = p.z / r;

	int sgn = 0;

	if(p.y > 0.){
		sgn = 1;
	}else if(p.y < 0.){
		sgn = -1;
	}

	float a = sgn * acos( p.x / ( sqrt(p.x*p.x + p.y*p.y) ) );

	elev = int(incl);
	az = int(a);
	return vec2(p.x, p.y);
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
		hit.hitAudio = sphere.isAudio;

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
		hit.hitAudio = sphere.isAudio;

		return true;
	}

	return false;
}

bool boxHit(Ray ray, inout Hit hit, vec3 closestPoint, vec3 farthestPoint, vec3 color){
	
	bool swappedX = false;
	bool swappedY = false;
	bool swappedZ = false;


	vec3 o = ray.origin;
	vec3 dir = ray.direction;


	float txmin = (closestPoint.x - o.x) / dir.x;
	float txmax = (farthestPoint.x - o.x) / dir.x;

	float tymin = (closestPoint.y - o.y) / dir.y;
	float tymax = (farthestPoint.y - o.y) / dir.y;

	if(txmax < txmin){
		float tmp = txmax;
		txmax = txmin;
		txmin = tmp;
		swappedX = true;
	}
	if(tymax < tymin){
		float tmp = tymax;
		tymax = tymin;
		tymin = tmp;
		swappedY = true;
	}

	if( (txmin > tymax) || (tymin > txmax) ){
		return false;
	}

	float tmin = max(txmin, tymin);
	float tmax = min(txmax, tymax);

	float tzmin = (closestPoint.z - o.z) / dir.z;
	float tzmax = (farthestPoint.z - o.z) / dir.z;
	if(tzmax < tzmin){
		float tmp = tzmax;
		tzmax = tzmin;
		tzmin = tmp;
		swappedZ = true;
	}

	if( (tmin > tzmax) || (tzmin > tmax) ){
		return false;
	}

	hit.t = min(tmin, tzmin);

	vec3 normal;
	if(hit.t == tymin){
		normal = vec3(0,1,0);
		if(swappedY){
			normal = vec3(0,-1,0);

		}
	}
	else if(hit.t == txmin){
		normal = vec3(1,0,0);
		if(swappedX){
			normal = vec3(-1,0,0);

		}
	}
	else if(hit.t == tzmin){
		normal = vec3(0,0,1);
		if(swappedZ){
			normal = vec3(0,0,-1);

		}
	}

	hit.emission = color;
	hit.position = at(ray, hit.t);
	set_face_normal(ray, normal, hit);
	hit.hitLight = false;
	hit.hitAudio = false;


	return true;
}


bool plane_hit(Ray ray, inout Hit hit, vec4 plane, vec3 normal, vec3 color){
	
	float t = (-plane.w - dot(plane.xyz, ray.origin) / dot(plane.xyz, ray.direction));

	if(t <= EPS || t > INFINITY){
		return false;
	}

	hit.emission = color;
	hit.t = t;
	hit.position = at(ray,t);
	set_face_normal(ray, normal, hit);
	hit.hitLight = false;
	hit.hitAudio = false;

	return true;
}

bool shadow_hit(Ray ray, inout Hit hit){

	Sphere s1 = Sphere(vec3(0.,0., -1.), 0.5, vec3(1., 0.0, 0.3), false, false);
	Sphere s5 = Sphere(vec3(1.3,0.3, -0.7), 0.3, vec3(0.3, 1.0, 0.3), false, false);
	Sphere[] scene = Sphere[](s1, s5);


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
				hit.hitAudio = tmp.hitAudio;
			}
		}
	}
	return has_hit;

}


bool scene_hit(inout Ray ray, inout Hit hit){

	Sphere s1 = Sphere(vec3(0.,0., -1.), 0.5, vec3(1., 0.0, 0.3), false, false);
	Sphere s5 = Sphere(vec3(1.3,0.3, -0.7), 0.3, vec3(0.3, 1.0, 0.3), false, false);

	Sphere floorSphere = Sphere(vec3(0., -101.4, -1.), 100., WHITE, false, false);
	Sphere s2 = Sphere(vec3(0., 104.4, -1.), 100., WHITE, false, false);
	Sphere leftSphere = Sphere(vec3(-103., 0., -1.), 100., GREEN, false, false);
	Sphere rightSphere = Sphere(vec3(104., 0., -1.), 100., RED, false, false);

	lightSphere.center = vec3(  .5 + sin(time * 2), .6 + sin(time), -0.2 - sin(time * 1));
	audioSphere.center = vec3(.5 - sin(time * 2), .6 + cos(time), -0.2 - sin(time * 1));

	Sphere[] scene = Sphere[](s1, floorSphere, lightSphere, s5, s2, leftSphere, rightSphere, audioSphere);


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
				hit.hitAudio = tmp.hitAudio;
			}
		}
	}
	return has_hit;
	
}

vec3 directLightSampling(Ray ray, inout Hit hit){
	vec3 n = randomSphereDirection(seed) * 0.3;
	return lightSphere.center + n;
}

bool shadowIntersect(Ray ray, float maxLength){
	Hit h;
	if(shadow_hit(ray, h)){
		if(h.t < maxLength){
			return true;
		}
	}

	return false;

}

// ======================AUDIO=========================

vec3 directAudioSampling(){
	vec3 n = randomSphereDirection(seed) * 0.3;
	return audioSphere.center + n;
}

bool detectionSphereHit(Ray ray, inout Hit hit){
	return sphere_hit(detectionSphere, ray, hit);
}


// if audio source can be accessed directly from the camera, no need to fully path trace
void audioCheck(Ray ray){

	Hit audioHit;
	vec3 audioDir = (audioSphere.center - ray.origin) + audioSphere.radius;
	vec3 normalAudioDir = normalize(audioDir);
	Ray shadowRay = Ray(ray.origin, normalAudioDir);
	if(!shadowIntersect(shadowRay, length(audioDir))){
		float l = totalPathLength + length(audioDir);
		if(l < minDistToAudioSource){
			Ray unitRay = Ray(ray.origin, normalAudioDir);
			vec3 p = at(unitRay, .1);
			vec2 res = cartesianToSpherical(p);
			elevation = int(res.x);
			azimuth = int(res.y);
			minDistToAudioSource = l;
		}
	}
}



vec3 ray_color(Ray ray){

	Hit hit;
	vec3 col = vec3(0.);
	vec3 pxl = vec3(1.);

	bool isSpecular = true;

	for(int depth = 0; depth < PATHDEPTH; depth++){
		bool h = scene_hit(ray,hit);

		if(!h){
			return col;
		}

		totalPathLength += hit.t;
		// AUDIO
//			if(hit.hitAudio && totalPathLength < minDistToAudioSource){
//				Ray unitRay = Ray(vec3(0.), outDir);
//				vec3 p = at(unitRay, 1.);
//				vec2 res = cartesianToSpherical(p);
//				elevation = res.x;
//				azimuth = res.y;
//				minDistToAudioSource = totalPathLength;
//			}else{
//#ifdef DAS
//				Hit audioHit;
//				vec3 audioDir = directAudioSampling() - ray.origin;
//				vec3 normalAudioDir = normalize(audioDir);
//				Ray shadowRay = Ray(ray.origin, normalAudioDir);
//				if(!shadowIntersect(shadowRay, length(audioDir))){
//					float l = totalPathLength + length(audioDir);
//					if(l < minDistToAudioSource){
//						Ray unitRay = Ray(vec3(0.), outDir);
//						vec3 p = at(unitRay, 1.);
//						vec2 res = cartesianToSpherical(p);
//						elevation = int(res.x);
//						azimuth = int(res.y);
//						minDistToAudioSource = l;
//					}
//				}
//				depth++;
//			}
//#endif
		


		// VISUAL
		if(hit.hitLight){
#ifdef DLS
			if(isSpecular){
				col += pxl * hit.emission;
			}
#else
			col += pxl * hit.emission;
#endif
			return col;
		}

		isSpecular = false;
		ray.direction = cosWeightedRandomHemisphereDirection2(hit.normal);
		ray.origin = hit.position;

		pxl *= hit.emission;

// DLS
#ifdef DLS
		Hit lightHit;
		vec3 lightDir = directLightSampling(ray, lightHit) - ray.origin;
		vec3 normalLightDir = normalize(lightDir );
		Ray shadowRay = Ray(ray.origin, normalLightDir);
		if(!shadowIntersect(shadowRay, length(lightDir)) ){
			float cos_a_max = sqrt(1. - clamp(0.3 * 0.3 / dot(lightSphere.center - ray.origin, lightSphere.center - ray.origin), 0., 1.));
			float weight = 2. * (1. - cos_a_max);
			col += ( pxl * LIGHT) * (weight * clamp( dot(normalLightDir, hit.normal), 0., 1.) );
		}
		depth++;
#endif
	}
	
	return col;

}

void main(){
	// seed calc for random unit hemisphere from: https://www.shadertoy.com/view/lsX3DH
	vec2 q = gl_FragCoord.xy/resolution.xy;
	vec2 p = -1.0 + 2.0 * ( gl_FragCoord.xy) / resolution.xy;
	p.x *= resolution.x/resolution.y;

	seed = p.x + p.y * 3.43121412313 + fract(1.12345314312*(time * 0.2));
	lightSphere = Sphere(vec3(.5, .0, -0.2), 0.3, vec3(15.), true, false);
	Sphere lightSphere2 = Sphere(vec3(.5, .0, -0.2), 0.3, vec3(15.), true, false);
	Sphere lightSphere3 = Sphere(vec3(.5, .0, -0.2), 0.3, vec3(15.), true, false);


	lights[0] = lightSphere;
	lights[1] = lightSphere2;
	lights[2] = lightSphere3;

	// AUDIO

	elevation = sphere_coords[0];
	azimuth = sphere_coords[1];
	minDistToAudioSource = sphere_coords[2];

	audioSphere = Sphere(vec3(0.3, 0.6, -1.), 0.2, vec3(0.2, 0., 1.), false, true);


	vec3 rayCol = vec3(0.);
	vec3 totalCol = vec3(0);

    vec3 lookAt = vec3(0., 0.,  0.0);
    vec3 ww = normalize( lookAt - camera.position );
    vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
    vec3 vv = normalize( cross(uu,ww));

	detectionSphere = Sphere(camera.position, 1., vec3(1.), false, false);
	for(int i = 0; i<SAMPLES; i++){
		vec2 rayDirOffset = 2. * (hash2(seed) - vec2(0.5)) / resolution.y;

		vec3 rayDir = normalize((p.x + rayDirOffset.x) * camera.right + (p.y + rayDirOffset.y) * camera.up + camera.forward);

		// save first ray direction for audio direction
		outDir = rayDir;
		Ray ray = Ray(camera.position, rayDir);
		audioCheck(ray);
		rayCol = ray_color(ray);
		totalCol += rayCol;

		seed = mod( seed*1.1234567893490423, 13. );
	}

	totalCol /= float(SAMPLES);
	// clamp between 0 and 1 + gamma adjustment
	totalCol = pow(clamp(totalCol, 0.0, 1.0), vec3(0.45));

	sphere_coords[0] = elevation;
	sphere_coords[1] = azimuth;
	sphere_coords[2] = minDistToAudioSource;


	fragCol = vec4(totalCol, 1.0);
	//fragCol = vec4(uv, 0., 1.);
}


// TODO: save full path length to audio source
//		 compare path length to minDistToAudioSource
//		 if smaller - save that length + sphere coords to SSBO
