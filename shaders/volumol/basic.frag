#version 330

in vec3 vertColor;
in vec2 texCoord;
in vec3 normal;
in vec3 vertPos;

out vec4 FragColor;

uniform vec3 sun_direction;
uniform vec3 sun_color;
uniform vec3 ambient_color;
uniform vec3 camera_pos;
uniform sampler2DArray shadow_map;
uniform int levels;
uniform mat4[8] light_matrices;
uniform float[8] layer_depths;
uniform vec4 camera_dir;

#define PI 3.141592653589

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float sampleShadowMap(int level, vec2 light_pos, float d) {
	float depth = texture(shadow_map, vec3(light_pos, float(level))).r;
	return depth > d ? 1.0 : 0.0;
}

float smoothShadow(int level, vec3 pos) {
	if (level < 0) return 1.0;

	vec4 light_pos = light_matrices[level] * vec4(pos, 1.0);

	light_pos.xyz /= light_pos.w;
	light_pos = light_pos * 0.5 + 0.5;

	vec2 texel_size = 1.0 / vec2(textureSize(shadow_map, 0));
	vec2 f = fract(light_pos.xy / texel_size);
	vec2 sample_pos = light_pos.xy - f * texel_size;

	float s00 = sampleShadowMap(level, sample_pos + vec2(-1.0, -1.0) * texel_size, light_pos.z);
	float s10 = sampleShadowMap(level, sample_pos + vec2( 0.0, -1.0) * texel_size, light_pos.z);
	float s20 = sampleShadowMap(level, sample_pos + vec2( 1.0, -1.0) * texel_size, light_pos.z);
	float s01 = sampleShadowMap(level, sample_pos + vec2(-1.0,  0.0) * texel_size, light_pos.z);
	float s11 = sampleShadowMap(level, sample_pos                                , light_pos.z);
	float s21 = sampleShadowMap(level, sample_pos + vec2( 1.0,  0.0) * texel_size, light_pos.z);
	float s02 = sampleShadowMap(level, sample_pos + vec2(-1.0,  1.0) * texel_size, light_pos.z);
	float s12 = sampleShadowMap(level, sample_pos + vec2( 0.0,  1.0) * texel_size, light_pos.z);
	float s22 = sampleShadowMap(level, sample_pos + vec2( 1.0,  1.0) * texel_size, light_pos.z);

	float t00 = mix(mix(s00, s01, f.y), mix(s10, s11, f.y), f.x);
	float t01 = mix(mix(s01, s02, f.y), mix(s11, s12, f.y), f.x);
	float t10 = mix(mix(s10, s11, f.y), mix(s20, s21, f.y), f.x);
	float t11 = mix(mix(s11, s12, f.y), mix(s21, s22, f.y), f.x);

	return pow(0.25 * (t00 + t01 + t10 + t11), 2.0);
}

void main() {
	vec3 fNormal = normalize(normal);
	
	vec3 view_vector = mix(normalize(camera_pos - vertPos), -camera_dir.xyz, camera_dir.w);

	vec3 baseColor = pow(vertColor, vec3(2.2));

	float roughness = texCoord.x;
	float metallicity = texCoord.y;

	vec3 F0 = mix(vec3(0.04), baseColor, metallicity);

	vec3 half_vector = normalize(sun_direction + view_vector);

	float NDF = DistributionGGX(fNormal, half_vector, roughness);
	float G   = GeometrySmith(fNormal, view_vector, sun_direction, roughness);
	vec3 F    = fresnelSchlick(max(dot(half_vector, view_vector), 0.0), F0);
	vec3 kD = vec3(1.0) - F;
	kD *= 1.0 - metallicity;
	
	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(fNormal, view_vector), 0.0) * max(dot(fNormal, sun_direction), 0.0) + 0.0001;
	vec3 specular     = numerator / denominator;
	    
	float NdotL = max(dot(fNormal, sun_direction), 0.0);
	vec3 Lo = (kD * baseColor / PI + specular) * sun_color * NdotL;

	float fresnel_factor = max(0.0, dot(fNormal, view_vector));

	int level = -1;
	for (int i = 0; i < levels; i++) {
		if (gl_FragCoord.z / gl_FragCoord.w < layer_depths[i + 1]) {
			level = i;
			break;
		}
	}

	float shadow = smoothShadow(level, vertPos);

	vec3 color = baseColor * ambient_color + Lo * shadow;

	color += 0.3 * baseColor * normalize(baseColor) * metallicity * pow(max(0.0, 1.0 - fresnel_factor * fresnel_factor), 4.0);

	FragColor = vec4(color, 1.);
}
