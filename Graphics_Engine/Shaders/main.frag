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


// AUDIO
vec3 outDir = vec3(0.);
Sphere audioSphere;
float elevation;
float azimuth;
float minDistToAudioSource;

float totalPathLength = 0.0;

// BDPT

#define LIGHTPATHLEN 2
#define EYEPATHLEN 2

#define AUDIOPATHLEN 1


//#define BDPT
//#define BDST

struct Vertex{
	Hit hit;
};

Vertex eyePath[EYEPATHLEN];
Vertex lightPath[LIGHTPATHLEN];
Vertex audioPath[AUDIOPATHLEN];
// UTILS

#define PI 3.14159265359
#define INFINITY 100000000.0
#define EPS 0.001
#define hash21(p) fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453)
#define hash33(p) fract(sin( (p) * mat3( 127.1,311.7,74.7 , 269.5,183.3,246.1 , 113.5,271.9,124.6) ) *43758.5453123)

#define SAMPLES 12
#define PATHDEPTH 6

#define LIGHT vec3(15.)
#define BRIGHTNESS 0.05

#define WHITE vec3(0.8)
#define GREEN vec3(0., 0.8, 0.1)
#define RED vec3(0.8, 0., 0.1)

// direct light sampling
#define DLS


Sphere[10] scene;
Sphere[8] shadowscene;


//==================================================================================================

// modified version of hash3 from: https://gist.github.com/gonnavis/6e1a58419e5835358122cb1526ece8e5
vec2 hash2( inout float s ) {
	return fract(sin(vec2(s+=0.05,s+=0.05))*vec2(43758.5453123,22578.1459123));
}

// http://corysimon.github.io/articles/uniformdistn-on-sphere/
vec3 randomSphereDirection(){
	vec2 h = hash2(seed) * vec2(2.,6.28318530718)-vec2(1,0);

	float theta = 2. * PI * h.x;
	float phi = acos(1. - 2. * h.y);

	return normalize(vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi)));
}


// pixar algorithm https://graphics.pixar.com/library/OrthonormalB/paper.pdf
vec3 cosWeightedRandomHemisphereDirection( const vec3 n ) {
	vec2 seed_vec2 = hash2(seed);
	float v = seed_vec2.x;
	float u = seed_vec2.y;

    float nsign = (n.z>=0.0)?1.0:-1.0;
    float a = 1.0 / (1.0 + abs(n.z));
    float b = -nsign * n.x * n.y * a;
    vec3 b1 = vec3(1.0 - n.x * n.x * a, nsign*b, -nsign*n.x);
    vec3 b2 = vec3(b, nsign - n.y * n.y * a * nsign, -n.y);
    float randomDist = 6.2831853 * v;
    return sqrt(u)*(cos(randomDist)*b1 + sin(randomDist)*b2) + sqrt(1.0-u)*n;
}


void set_face_normal(inout Ray ray, vec3 normal, inout Hit hit){
	hit.front = dot(ray.direction, normal) < 0.;
	hit.normal = hit.front ? normal : -normal;
}

vec3 at(Ray r, float t){
	return r.origin + t*r.direction;
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

void sceneInit(){
	// FIRST ROOM

	Sphere s1 = Sphere(vec3(0.,0., -1.), 0.5, vec3(1., 0.0, 0.3), false);
	Sphere s5 = Sphere(vec3(1.3,0.3, -0.7), 0.3, vec3(0.3, 1.0, 0.3), false);

	Sphere floorSphere = Sphere(vec3(0., -101.4, -1.), 100., WHITE, false);
	Sphere s2 = Sphere(vec3(0., 110.4, -1.), 100., WHITE, false);
	Sphere leftSphere = Sphere(vec3(-103., 0., -1.), 100., GREEN, false);
	Sphere rightSphere = Sphere(vec3(80., 0., 70.), 90., RED, false);
	Sphere backSphere = Sphere(vec3(0., 0., -120.), 100., WHITE, false);
	Sphere frontSphere = Sphere(vec3(0., 0., 110.), 100., WHITE, false);

	// SECOND ROOM
	
	Sphere rightFloor = Sphere(vec3(75., -101, -10), 100., GREEN, false);
	Sphere rightBack = Sphere(vec3(210, 20, -100),100,  WHITE, false);

	lightSphere.center = vec3( 80. ,80., -35.);
	lightSphere.center = vec3(5.5 + sin(time * 2)  * 8, .6 + sin(time), 0. - sin(time * 1) * 8);


	shadowscene = Sphere[](s1, floorSphere, s5, s2, leftSphere, rightSphere, backSphere, frontSphere);
	scene = Sphere[](s1, floorSphere, lightSphere, s5, s2, leftSphere, rightSphere, backSphere, frontSphere ,audioSphere);	
}


bool scene_hit(inout Ray ray, inout Hit hit, bool isShadow = false){

	lightSphere.center = vec3(5.5 + sin(time * 2)  * 8, .6 + sin(time), 0. - sin(time * 1) * 8);


	float closest_hit = INFINITY;
	bool has_hit = false;


	if(isShadow){
		for(int i = 0; i < shadowscene.length(); i++){
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
	}else{
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
	}
	return has_hit;
}

vec3 directLightSampling(Ray ray, inout Hit hit){
	vec3 n = randomSphereDirection() * lightSphere.radius;
	return lightSphere.center + n;
}

bool shadowIntersect(Ray ray, float maxLength){
	Hit h;
	if(scene_hit(ray, h, true)){
		if(h.t < maxLength){
			return true;
		}
	}
	return false;
}

// ======================AUDIO=========================

// UTILS

float toDegrees(float angle){
	return angle * (180/PI);
}

void setAngles(vec3 directionToAudio, float addedLength = 0.0, bool adjust = false){
	vec3 tmp = directionToAudio - camera.forward;

	int elev = int(toDegrees( atan(tmp.y, sqrt(tmp.x * tmp.x + tmp.z * tmp.z) )));
	int az = int(abs(toDegrees( atan( -tmp.x, -tmp.z)) - 90));

	if(az != 180){
		az = az % 180;
	}

	if(!adjust){
		elevation = max(elev, -40);
		azimuth = az;
		minDistToAudioSource = totalPathLength + addedLength;
		return;
	}

	elevation += max(elev, -40) * 0.1;
	azimuth += az * 0.1;
}


// check if audio source can be accessed directly from the camera
void directAudioSampling(){

	// vector between camera pos and audio source
	vec3 audioDir = (audioSphere.center - camera.position);

	float l = length(audioDir);
	if(l >= minDistToAudioSource){
		return;
	}

	Ray audioRay = Ray(camera.position, normalize(audioDir));
	if(!shadowIntersect(audioRay, l) ){
		setAngles(audioDir, l);
	}
}

void buildAudioRays(){
	vec3 randomDirection = randomSphereDirection();
	vec3 direction = cosWeightedRandomHemisphereDirection(randomDirection);

	vec3 origin = audioSphere.center + randomDirection * audioSphere.radius;
	Ray ray = Ray(origin, direction);

	for( int i = 0; i < AUDIOPATHLEN; i++){
		Hit audioPathHit;
		bool hasHit = scene_hit(ray, audioPathHit);

		if(!hasHit){
			audioPathHit.t = -1;
			audioPath[i].hit = audioPathHit;

			break;
		}
		ray.direction = cosWeightedRandomHemisphereDirection(audioPathHit.normal);
		ray.origin = audioPathHit.position;
		audioPathHit.emission = LIGHT;
		audioPath[i].hit = audioPathHit;
	}
}

void buildLightRays(){
	vec3 randomDirection = randomSphereDirection();
	vec3 direction = cosWeightedRandomHemisphereDirection(randomDirection);

	vec3 origin = lightSphere.center + randomDirection * lightSphere.radius;
	Ray ray = Ray(origin, direction);

	for( int i = 0; i < LIGHTPATHLEN; i++){
		Hit lightPathHit;
		bool hasHit = scene_hit(ray, lightPathHit);

		if(!hasHit){
			lightPathHit.t = -1;
			lightPath[i].hit = lightPathHit;

			break;
		}
		ray.direction = cosWeightedRandomHemisphereDirection(lightPathHit.normal);
		ray.origin = lightPathHit.position;
		lightPathHit.emission = LIGHT;
		lightPath[i].hit = lightPathHit;
	}
}

void buildCameraAudioRays(){
	vec3 randomDirection = randomSphereDirection();
	vec3 direction = cosWeightedRandomHemisphereDirection(randomDirection);

	vec3 origin = camera.position + randomDirection;
	Ray ray = Ray(origin, direction);

	for( int i = 0; i < EYEPATHLEN; i++){
		Hit cameraPathHit;
		bool hasHit = scene_hit(ray, cameraPathHit);

		if(!hasHit){
			cameraPathHit.t = -1;
			eyePath[i].hit = cameraPathHit;

			break;
		}
		ray.direction = cosWeightedRandomHemisphereDirection(cameraPathHit.normal);
		ray.origin = cameraPathHit.position;
		cameraPathHit.emission = LIGHT;
		eyePath[i].hit = cameraPathHit;
	}


}

void buildCameraRays(Ray cameraRay){
	
	for(int i = 0; i < EYEPATHLEN; i++){
		Hit eyePathHit;
		bool hasHit = scene_hit(cameraRay, eyePathHit);

		if(!hasHit){
			eyePathHit.t = -1;
			eyePath[i].hit = eyePathHit;

			break;
		}else if(eyePathHit.hitLight){
			eyePath[i].hit = eyePathHit;

			break;
		}

		cameraRay.direction = cosWeightedRandomHemisphereDirection(eyePathHit.normal);
		cameraRay.origin = eyePathHit.position;
		eyePath[i].hit = eyePathHit;
	}
}


void reverberation(){
	vec3 origDir = eyePath[0].hit.position;

	for(int i = 0; i < EYEPATHLEN; i++){
		Hit hit = eyePath[i].hit;
		if(hit.t == -1){
			return;
		}
		totalPathLength += hit.t;
		for(int i = 0; i < AUDIOPATHLEN; i++){
			if(audioPath[i].hit.t == -1){
				break;
			}
			totalPathLength += audioPath[i].hit.t;

			vec3 audioPos = audioPath[i].hit.position;
			vec3 audioDir = audioPos - hit.position;
			vec3 normalAudioDir = normalize(audioDir);
			Ray shadowRay = Ray(hit.position, normalAudioDir);

			if(!shadowIntersect(shadowRay, length(audioDir))){
				setAngles(origDir, length(audioDir), true);
			}
		}
	}
}


vec3 ray_color(Ray ray){

	vec3 col = vec3(0.);
	vec3 pxl = vec3(1.);

	bool isSpecular = true;

	int maxDepth;

#ifdef BDPT
	maxDepth = EYEPATHLEN;
#else
	maxDepth = PATHDEPTH;
#endif
	for(int depth = 0; depth < maxDepth; depth++){

#ifdef BDPT
		Hit hit = eyePath[depth].hit;

		if(hit.t == -1){
			return col;
		}

		ray.origin = hit.position;

		if(hit.hitLight){
			if(isSpecular){
				col += pxl * hit.emission;
			}
			return col;
		}


		isSpecular = false;
		pxl *= hit.emission;

		// check light path intersect
		for(int i = 0; i < LIGHTPATHLEN; i++){
			if(lightPath[i].hit.t == -1){
				break;
			}

			vec3 lightPos = lightPath[i].hit.position;
			vec3 lightDir = lightPos - ray.origin;
			vec3 normalLightDir = normalize(lightDir);
			Ray shadowRay = Ray(ray.origin, normalLightDir);

			if(!shadowIntersect(shadowRay, length(lightDir))){
				// WEIGHTING ALGORITHM

				// 1. angle between lightHit normal and cameraHit normal * angle between lightHit normal and incoming light direction
				// 3. divide by path dist to light source, because 0 is a valid path node, add 2 ( for both light and eye path)

				float weight = clamp(dot(lightPath[i].hit.normal, hit.normal), 0.,1.) * clamp( dot(lightPath[i].hit.normal, -normalLightDir), 0., 1.);

				col += (pxl * lightPath[i].hit.emission * weight) / (LIGHTPATHLEN - i + depth + 2);
				return col;
			}
		}
#else
		Hit hit;

		bool h = scene_hit(ray,hit);

		if(!h){
			return col;
		}
		
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
		ray.direction = cosWeightedRandomHemisphereDirection(hit.normal);
		ray.origin = hit.position;

		pxl *= hit.emission;

#endif

#ifdef DLS
		Hit lightHit;
		vec3 lightDir = directLightSampling(ray, lightHit) - ray.origin;
		vec3 normalLightDir = normalize(lightDir );
		Ray shadowRay = Ray(ray.origin, normalLightDir);
		if(!shadowIntersect(shadowRay, length(lightDir)) ){
			col +=  pxl * LIGHT * ( BRIGHTNESS * clamp( dot(normalLightDir, hit.normal), 0., 1.) );
		}
#ifndef BDPT
		depth++;
#endif
#endif
	}

	return col;

}

void main(){
	
	vec2 q = gl_FragCoord.xy/resolution.xy;
	vec2 p = -1.0 + 2.0 * ( gl_FragCoord.xy) / resolution.xy;
	p.x *= resolution.x/resolution.y;

	// cursor
	vec2 inCenter = step(vec2(-0.01), p) - step(vec2(0.01), p);
	float v = inCenter.y * inCenter.x;
	if(length(p) < 0.005){
		fragCol = vec4(0.8);	
		return;
	}

	// seed calc from: https://www.shadertoy.com/view/lsX3DH
	seed = p.x + p.y * 3.43121412313 + fract(1.12345314312*(time * 0.2));
	lightSphere = Sphere(vec3(.5, .0, -0.2), 0.3, vec3(15.), true);

	// AUDIO
	elevation = sphere_coords[0];
	azimuth = sphere_coords[1];
	minDistToAudioSource = sphere_coords[2];

	audioSphere = Sphere(vec3(0., .5, .5), 0.01, vec3(0.2, 0., 1.), false);

	sceneInit();

	vec3 rayCol = vec3(0.);
	vec3 totalCol = vec3(0);

    vec3 lookAt = vec3(0., 0.,  0.0);
    vec3 ww = normalize( lookAt - camera.position );
    vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
    vec3 vv = normalize( cross(uu,ww));

	// AUDIO
	directAudioSampling();

	#ifdef BDST
		buildAudioRays();
		buildCameraAudioRays();
		reverberation();
	#endif

	for(int i = 0; i<SAMPLES; i++){
		totalPathLength = 0.;
		vec2 rayDirOffset = 2. * (hash2(seed) - vec2(0.5)) / resolution.y;

		vec3 rayDir = normalize((p.x + rayDirOffset.x) * camera.right + (p.y + rayDirOffset.y) * camera.up + camera.forward);

		Ray ray = Ray(camera.position, rayDir);

#ifdef BDPT
		buildLightRays();
		buildCameraRays(ray);
#endif
	
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
}

