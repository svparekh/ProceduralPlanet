#version 330

const int MAX_POINTS = 128;

// from glState
uniform ivec2 iResolution;
uniform vec3 cameraPosition;
uniform mat3 camTBNMat;
uniform vec2 planetRotationAngleRadians;
uniform float noiseOffset;
uniform int fbmIterations;
uniform bool performanceMode;
uniform int numUserAddedPoints;
uniform bool hardShadowsEnable;
uniform vec3 pArray[MAX_POINTS];
uniform float rArray[MAX_POINTS];
uniform float hArray[MAX_POINTS];

smooth in vec3 fragNorm;    // Interpolated model-space normal
out vec3 outCol;    // Final pixel color

/* Material */
struct Material {
    vec3 color;
    float diffuse_str;
    float specular_str;
    float specular_exp;
    float reflection_str; // keep low - in a black sky env, reflections just darken the color
};
/* Intersection */
struct ItersectionDetails {
    int type;   /* 0: Sky, 1: Sun, 2: Water, 3: Terrain/Planet/Land */
    vec3 pos;
    vec3 normal;
    Material material;
};


/*
 * Global Variables
 * */
float planetBaseSize = 1.0;
vec3  light        = vec3(100.0f, 100.0f, -100.0f);
vec3  light_color  = vec3(1.0);
float ambient      = 0.5;

// Materials 
Material mat_water  = Material(vec3(6.0f, 66.0f, 115.0f)   / 255.0f,   0.7,   0.5,    50.0,   0.25);
Material mat_snow   = Material(vec3(253.0f ,253.0f ,255.0f) / 255.0f,   0.85,  0.6,    75.0,   0.05);
Material mat_sand   = Material(vec3(194.0f, 178.0f, 128.0f) / 255.0f,   0.7,   0.3,    10.0,   0.05);
Material mat_dirt   = Material(vec3(118.0f, 85.0f, 43.0f)   / 255.0f,   0.7,   0.2,    1.0,    0.05);
Material mat_grass  = Material(vec3(96.0f, 145.0f, 32.0f)   / 255.0f,   0.7,   0.2,    2.0,    0.05);
Material mat_stone  = Material(vec3(136.0f, 140.0f, 141.0f) / 255.0f,   0.7,   0.4,    3.0,    0.05);
Material mat_sun    = Material(vec3(255.0f, 211.0f, 92.0f)  / 255.0f,   1.0,   0.0,    0.0,    0.0);


// Intersection types
const int NON_INTERSECT = 0;
const int SUN_INTERSECT = 1;
const int WATER_INTERSECT = 2;
const int PLANET_INTERSECT = 3;


/* SRC: https://www.shadertoy.com/view/3sj3Rt */
const float GOLDEN_RATIO = 1.61803398875;
float SMALL_SPHERES_RADIUS = 25.5;
const float M_PI = 3.14159265359f;
float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio   
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two
const float SEED = M_PI;
const int N_POINTS = 100; // Change this number to increase/reduce number of points

// Ray marching properties
const int MAX_STEPS = 64; // 256
const float MAX_DIST = 50.0; // 500
const float EPSILON = 0.001; // 0.00001

float gold_noise(in vec2 coordinate, in float seed){
    return fract(tan(distance(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
}

vec3 lerp(vec3 a, vec3 b, float aa) {
    return a * aa + b * (1.0-aa);
}

float lerp(float a, float b, float aa) {
    return a * aa + b * (1.0-aa);
}

float saturate(float a) {
    return clamp(a, 0.0, 1.0);
}

float sphereSDF(vec3 p, vec3 center, float radius) {
    return length(p - center) - radius;
}

vec3 random_sphere_point(int i) {  // used for shadow calculation (random points used as light sphere)
    // generating uniform points on the sphere: http://corysimon.github.io/articles/uniformdistn-on-sphere/
    float fi = float(i);
    // Note: Use uniform random generator instead of noise in your applications
    float theta = 2.0f * M_PI * gold_noise(vec2(fi * 0.3482f, fi * 2.18622f), SEED);
    float phi = acos(1.0f - 2.0f * gold_noise(vec2(fi * 1.9013, fi * 0.94312), SEED));
    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return vec3(x,y,z);
}

vec3 reflection(vec3 vec_in, vec3 normal) {
    // R = I - 2(N * I)N
    return vec_in - (2 * dot(normal, vec_in) * normal);
}

vec3 sky_color(vec3 ro, vec3 rd) {
    return vec3(0.0) - (rd.y) * 0.2 * vec3(0.05) + 0.05 * 1.0;
    // return vec3(0.05);
}

vec3 customMod(vec3 inc) {
    return inc - floor(inc / 289.0) * 289.0;
}

vec4 customMod(vec4 inc) {
    return inc - floor(inc / 289.0) * 289.0;
}

vec4 permute(vec4 x) {
    return customMod((x * 34.0 + 1.0) * x);
}

vec4 inverseSqrtT(vec4 p) {
    return 1.79284291400159 - p * 0.85373472095314;
}

// Help from: https://www.cs.umd.edu/class/spring2018/cmsc425/Lects/lect13-2d-perlin.pdf
float getNoiseAt(vec3 samplePoint) {
    vec3 sp = samplePoint + noiseOffset;
    const vec2 constant = vec2(1.0 / 6.0,  1.0 / 3.0);

    // generate heights to interpolate btwn
    // four corners
    vec3 init = floor(sp + dot(sp, constant.yyy));
    vec3 c1 = sp - init + dot(init, constant.xxx);
    vec3 a = step(c1.yzx, c1.xyz);
    vec3 b = 1.0 - a;
    vec3 i1 = min(a.xyz, b.zxy);
    vec3 i2 = max(a.xyz, b.zxy);
    vec3 c2 = c1 - i1 + constant.xxx;
    vec3 c3 = c1 - i2 + constant.yyy;
    vec3 c4 = c1 - 0.5;

    
    // permutations
    init = customMod(init);
    vec4 perm = permute(permute(permute(init.z + vec4(0.0, i1.z, i2.z, 1.0)) + init.y + vec4(0.0, i1.y, i2.y, 1.0)) + init.x + vec4(0.0, i1.x, i2.x, 1.0));
    vec4 permAdj = perm - 49.0 * floor(perm / 49.0);
    
    //gradients
    vec4 d1 = floor(permAdj / 7.0);
    vec4 d2 = floor(permAdj - 7.0 * d1);
    vec4 x = (d1 * 2.0 + 0.5) / 7.0 - 1.0;
    vec4 y = (d2 * 2.0 + 0.5) / 7.0 - 1.0;

    vec4 f = vec4(x.xy, y.xy);
    vec4 g = vec4(x.zw, y.zw);
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 i = floor(f) * 2.0 + 1.0;
    vec4 j = floor(g) * 2.0 + 1.0;
    vec4 k = -step(h, vec4(0.0));

    vec4 m = f.xzyw + i.xzyw * k.xxyy;
    vec4 n = g.xzyw + j.xzyw * k.zzww;

    vec3 g1 = vec3(m.xy, h.x);
    vec3 g2 = vec3(m.zw, h.y);
    vec3 g3 = vec3(n.xy, h.z);
    vec3 g4 = vec3(n.zw, h.w);

    // interpolation
    vec4 norm = inverseSqrtT(vec4(dot(g1, g1), dot(g2, g2), dot(g3, g3), dot(g4, g4)));
    g1 *= norm.x;
    g2 *= norm.y;
    g3 *= norm.z;
    g4 *= norm.w;
    
    vec4 maxv = max(0.6 - vec4(dot(c1, c1), dot(c2, c2), dot(c3, c3), dot(c4, c4)), 0.0);
    maxv = maxv * maxv;
    maxv = maxv * maxv;
    
    vec4 p = vec4(dot(c1, g1), dot(c2, g2), dot(c3, g3), dot(c4, g4));

    return 50.0 * dot(maxv, p);
}

float fbm(vec3 samplePoint){
    float sum = 0;
    float amplitude = 1;
    float frequency = 1;
    // increase frequency, decrease amplitude per iteration
    for (int i = 0; i < fbmIterations; i++) {
        sum += getNoiseAt(samplePoint * frequency) * amplitude; 
        frequency *= 2;
        amplitude *= 0.5;
    }

    return sum;
}

vec3 rotateYX(vec3 init, vec2 angle){
    float angleY = angle.x;
    float angleX = angle.y;
    vec3 npt = init;
    npt.x = (init.x * cos(angleY)) + (init.z * sin(angleY));
    npt.z = -(init.x * sin(angleY)) + (init.z * cos(angleY));
    vec3 newPos = npt;
    newPos.y = (npt.y * cos(angleX)) - (npt.z * sin(angleX));
    newPos.z = (npt.y * sin(angleX)) + (npt.z * cos(angleX));
    return newPos;
}

float displace(vec3 p){
    // apply rotation
    vec3 newPos = rotateYX(p, planetRotationAngleRadians);
    if (length(newPos) > EPSILON) {
		p = newPos;
	}

    // displace
    float ret;
    ret = fbm(p); // continents
    // ret = abs(fbm(p)); // rivers on low points
    // ret = 1.0 - abs(fbm(p)); // mountains on high points

    // Generate user terrain
    for (int i = 0; i < numUserAddedPoints; i++) {
        float prox = distance(p, pArray[i]);
        if (prox <= rArray[i]) {
            // ae^(- ((x - b)^2) / (2c^2))
            float x = rArray[i] - prox;
            float c = rArray[i] / 4.0;
            float h = hArray[i] * exp(-(pow(prox, 2.0)) / (2.0 * pow(c, 2.0)));
            ret += h / 0.05;
        }
    }
    
    ret *= 0.05; // normalize
    ret -= 0.0075; 
    return ret;
}

vec2 getRayMarchHit(vec3 p) {
    // sphere
    float sphereDist = sphereSDF(p, vec3(0.0, 0.0, 0.0), planetBaseSize + displace(p));
    float sphereID =PLANET_INTERSECT;
    vec2 sphere = vec2(sphereDist, sphereID);
    // water sphere
    float waterSphereDist = sphereSDF(p, vec3(0.0, 0.0, 0.0), planetBaseSize);
    float waterSphereID = WATER_INTERSECT;
    vec2 waterSphere = vec2(waterSphereDist, waterSphereID);
    // sun sphere
    float sunSphereDist = sphereSDF(p, light / 10.0, 0.35); 
    float sunSphereID = SUN_INTERSECT;
    vec2 sunSphere = vec2(sunSphereDist, sunSphereID);

    vec2 res = (sphere.x < waterSphere.x) ? sphere : waterSphere;
    res = (res.x < sunSphere.x) ? res : sunSphere;
    return res;
}

vec2 rayMarch(vec3 ro, vec3 rd) {
    vec2 hit, item;
    for (int i = 0; i < MAX_STEPS; i++){
        vec3 pos = ro + item.x * rd;
        hit = getRayMarchHit(pos); // get sdf
        item.x += hit.x; // increment by distance
        item.y = hit.y;

        // if hit
        if ((abs(hit.x) < EPSILON) || (item.x > MAX_DIST)) {
            break;
        }
    }
    return item;
}

ItersectionDetails renderScene(vec3 ro, vec3 rd) {
    ItersectionDetails ret;

    ret.type = NON_INTERSECT;
    ret.normal = vec3(0.0);
    ret.material.color = sky_color(ro, rd);

    vec2 dist = rayMarch(ro, rd);
    float t = dist.x;
    if (t < MAX_DIST) {
        if (dist.y == WATER_INTERSECT) { // water
            vec3 pos = ro + t*rd;

            vec3 normal;
            if (performanceMode) {
                normal = normalize(pos);
            }
            else {
                const vec3 small_step = vec3(0.001, 0.0, 0.0);
                float gradient_x = sphereSDF(pos + small_step.xyy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos + small_step.xyy)) - sphereSDF(pos - small_step.xyy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos - small_step.xyy));
                float gradient_y = sphereSDF(pos + small_step.yxy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos + small_step.yxy)) - sphereSDF(pos - small_step.yxy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos - small_step.yxy));
                float gradient_z = sphereSDF(pos + small_step.yyx, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos + small_step.yyx)) - sphereSDF(pos - small_step.yyx, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos - small_step.yyx));
                normal = normalize(vec3(gradient_x, gradient_y, gradient_z));
            }

            ret.type            = WATER_INTERSECT;
            ret.pos             = pos;
            ret.normal          = normal;
            ret.material        = mat_water;
        }
        else if (dist.y == SUN_INTERSECT) { // sun
            vec3 pos = ro + t*rd;
            ret.type            = SUN_INTERSECT;
            ret.pos             = pos;
            ret.normal          = normalize(pos);
            ret.material        = mat_sun;
        }
        else if (dist.y == PLANET_INTERSECT) { // terrain
            vec3 pos = ro + t*rd;

            // calculate normal
            const vec3 small_step = vec3(0.001, 0.0, 0.0);
            float gradient_x = sphereSDF(pos + small_step.xyy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos + small_step.xyy)) - sphereSDF(pos - small_step.xyy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos - small_step.xyy));
            float gradient_y = sphereSDF(pos + small_step.yxy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos + small_step.yxy)) - sphereSDF(pos - small_step.yxy, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos - small_step.yxy));
            float gradient_z = sphereSDF(pos + small_step.yyx, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos + small_step.yyx)) - sphereSDF(pos - small_step.yyx, vec3(0.0, 0.0, 0.0), 1.0 + displace(pos - small_step.yyx));
            vec3 normal = normalize(vec3(gradient_x, gradient_y, gradient_z));

            ret.type    = PLANET_INTERSECT;
            ret.pos     = pos;
            ret.normal  = normal;
            
            float height = length(pos);
            float steepness = 1.0 - dot(normal, normalize(pos));
            steepness = saturate(steepness / 0.1);

            // h > 1.0450 = snow
            // 1.0400 < h < 1.0450 = dirt to snow transition
            // 1.0275 < h < 1.0400 = dirt
            // 1.0225 < h < 1.0275 = grass to dirt transition
            // 1.0075 < h < 1.0225 = grass
            // 1.0025 < h < 1.0075 = sand to grass transition
            // h < 1.0025 = sand
            
            if (height > (planetBaseSize + 0.045)) { // snow
                ret.material = mat_snow;
            }
            else if (height >= (planetBaseSize + 0.04)) { // dirt to snow transition
                float lerp_amount = saturate((height - (planetBaseSize + 0.04)) / 0.005);
                ret.material.color           = lerp(mat_snow.color, mat_dirt.color, lerp_amount);
                ret.material.diffuse_str     = lerp(mat_snow.diffuse_str, mat_dirt.diffuse_str, lerp_amount);
                ret.material.specular_str    = lerp(mat_snow.specular_str, mat_dirt.specular_str, lerp_amount);
                ret.material.specular_exp    = lerp(mat_snow.specular_exp, mat_dirt.specular_exp, lerp_amount);
                ret.material.reflection_str  = mat_dirt.reflection_str;
            }
            else if (height > (planetBaseSize + 0.0275)) { // dirt
                ret.material = mat_dirt;
            }
            else if (height >= (planetBaseSize + 0.0225)) { // grass to dirt transition
                float lerp_amount = saturate((height - (planetBaseSize + 0.0225)) / 0.005);
                ret.material.color           = lerp(mat_dirt.color, mat_grass.color, lerp_amount);
                ret.material.diffuse_str     = lerp(mat_dirt.diffuse_str, mat_grass.diffuse_str, lerp_amount);
                ret.material.specular_str    = lerp(mat_dirt.specular_str, mat_grass.specular_str, lerp_amount);
                ret.material.specular_exp    = lerp(mat_dirt.specular_exp, mat_grass.specular_exp, lerp_amount);
                ret.material.reflection_str  = mat_grass.reflection_str;
            }
            else if (height > (planetBaseSize + 0.0075)) { // grass
                ret.material = mat_grass;
            }
            else if (height >= (planetBaseSize + 0.0025)) { // sand to grass transition
                float lerp_amount = saturate((height - (planetBaseSize + 0.0025)) / 0.005);
                ret.material.color           = lerp(mat_grass.color, mat_sand.color, lerp_amount);
                ret.material.diffuse_str     = lerp(mat_grass.diffuse_str, mat_sand.diffuse_str, lerp_amount);
                ret.material.specular_str    = lerp(mat_grass.specular_str, mat_sand.specular_str, lerp_amount);
                ret.material.specular_exp    = lerp(mat_grass.specular_exp, mat_sand.specular_exp, lerp_amount);
                ret.material.reflection_str  = mat_sand.reflection_str;
            }
            else { // sand
                ret.material = mat_sand;
            }

            // add steepness based color
            if ((height > (planetBaseSize + 0.0075)) && (height < (planetBaseSize + 0.0375))) { 
                if (steepness > 0.333) {
                    // 0.0075 to 0.0125 and 0.0325 to 0.0375, transition stone out
                    float height_multiplier = 0.0;
                    if (height <= (planetBaseSize + 0.0125)) {
                        height_multiplier = 1.0 - saturate((height - (planetBaseSize + 0.0075)) / 0.005);
                    }
                    else if (height >= (planetBaseSize + 0.0325)) {
                        height_multiplier = saturate((height - (planetBaseSize + 0.0325)) / 0.005);
                    }
                    float lerp_amount = saturate((steepness - 0.333) / 1.0) * (1.0 - height_multiplier);
                    ret.material.color           = lerp(mat_dirt.color, ret.material.color, lerp_amount);
                    ret.material.diffuse_str     = lerp(mat_dirt.diffuse_str, ret.material.diffuse_str, lerp_amount);
                    ret.material.specular_str    = lerp(mat_dirt.specular_str, ret.material.specular_str, lerp_amount);
                    ret.material.specular_exp    = lerp(mat_dirt.specular_exp, ret.material.specular_exp, lerp_amount);
                }
                if (steepness > 0.667) {
                    // 0.0075 to 0.0125 and 0.0325 to 0.0375, transition stone out
                    float height_multiplier = 0.0;
                    if (height <= (planetBaseSize + 0.0125)) {
                        height_multiplier = 1.0 - saturate((height - (planetBaseSize + 0.0075)) / 0.005);
                    }
                    else if (height >= (planetBaseSize + 0.0325)) {
                        height_multiplier = saturate((height - (planetBaseSize + 0.0325)) / 0.005);
                    }
                    float lerp_amount = saturate((steepness - 0.667) / 1.0) * (1.0 - height_multiplier);
                    ret.material.color           = lerp(mat_stone.color, ret.material.color, lerp_amount);
                    ret.material.diffuse_str     = lerp(mat_stone.diffuse_str, ret.material.diffuse_str, lerp_amount);
                    ret.material.specular_str    = lerp(mat_stone.specular_str, ret.material.specular_str, lerp_amount);
                    ret.material.specular_exp    = lerp(mat_stone.specular_exp, ret.material.specular_exp, lerp_amount);
                }
            }
        }
    }

    return ret;
}

/*
 * Shading the scene
 **/
vec3 shading(vec3 ro, vec3 rd, ItersectionDetails intersect) {
    vec3 ret    = vec3(0.0f);

    int   type              = intersect.type;
    vec3  pos               = intersect.pos;
    vec3  normal            = intersect.normal;
    vec3  color             = intersect.material.color;
    float diffuse_str       = intersect.material.diffuse_str;
    float specular_str      = intersect.material.specular_str;
    float specular_exp      = intersect.material.specular_exp;
    float reflection_str    = intersect.material.reflection_str;

    vec3 sky    = sky_color(ro, rd);

    if (type == NON_INTERSECT) {
        return sky;
    }
    if (type == SUN_INTERSECT) {
        return mat_sun.color;
    }

    ret = normal;
    vec3 light_color = lerp(sky, light_color, 0.5);

    // ro = point on surface
    // rd = to light
    vec3 lightDir = -normalize(light);
    vec3 reflected = reflection(-lightDir, normal);
    // Blinn-Phong: use halfway between light direction and view direction instead of reflected
    //vec3 halfway = normalize(lightDir + rd);

    float diffuse = max(0.0, dot(normal, -lightDir)) * diffuse_str;
    float specular = pow(max(0.0, dot(rd, reflected)), specular_exp) * specular_str;
    float shadow = 0.0;
    // REFLECTIONS AND SOFT SHADOWS ARE TURNED OFF FOR PERFORMANCE
    // ONLY HARD SHADOW ENABLED BY DEFAULT
    if (performanceMode){
        // hard shadows
        if (hardShadowsEnable) {
            ItersectionDetails shadow_ret = renderScene(pos + 0.01 * ((light) / length(light)), (light) / length(light));
            if (shadow_ret.type != NON_INTERSECT){
                if (shadow_ret.type != SUN_INTERSECT) {
                    shadow = 0.0;
                }
                else {
                    shadow = 1.0;
                }
            }
            else {
                shadow = 1.0;
            }
        }
        else {
            shadow = 1.0;
        }
        
    }
    else {
        // reflections
        ItersectionDetails reflectionIntersection = renderScene(pos + 0.01 * reflection(rd, normal), reflection(rd, normal));
        light_color = lerp(light_color, reflectionIntersection.material.color, 1.0 - reflection_str);
        // soft shadows
        for (int i = 0; i < N_POINTS; i++){
            vec3 p = random_sphere_point(i) * SMALL_SPHERES_RADIUS;

            ItersectionDetails shadow_ret = renderScene(pos + 0.01 * ((p + light) / length(p + light)), (p + light) / length(p + light));
            if (shadow_ret.type != NON_INTERSECT){
                shadow += 0.0;
            }
            else {
                shadow += 1.0;
            }
        }
        shadow = shadow / N_POINTS;
        shadow = saturate(shadow);
    }

    ret = (ambient + (diffuse + specular) * shadow) * color * light_color;
    return ret;
}

void main() {
    // pixel offset
    vec2 p  = vec2((2.0 * gl_FragCoord.x - iResolution.x) / iResolution.x, (2.0 * gl_FragCoord.y - iResolution.y) / iResolution.y);
    
    // ro = camera
    // rd = direction to center offset by frag coord
    vec3 ro = -2.0 * cameraPosition;
    vec3 rd = camTBNMat * normalize( vec3(p, -2.0) );

    ItersectionDetails intersect = renderScene(ro, rd);
    outCol = shading(ro, rd, intersect);
}
