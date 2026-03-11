#version 450

precision highp float;
precision highp int;

layout(location = 0) out vec4 out_color;

layout (location = 0) in highp vec4 in_vertex_color_0;
layout (location = 1) in highp vec3 in_vertex_normal;
layout (location = 2) in highp vec4 in_vertex_position;
layout (location = 3) in highp vec4 in_vertex_tangent;
layout (location = 4) in highp vec2 in_vertex_texture_coord_0;

layout(std140, set = 0, binding = 19) uniform per_frame_uniform
{
	float   delta_time;
	uint    nodes_count;
	uint    animations_count;
	uint    render_mode;
} in_per_frame_uniform;

void set_render_mode()
{
	vec4 out_color_copy = out_color;

	if (!(in_per_frame_uniform.render_mode == 0))
	{
		out_color = vec4(1.0, 0.0, 1.0, 1.0);
	}

	if (in_per_frame_uniform.render_mode == 1)
	{
		out_color = vec4(in_vertex_texture_coord_0.x, in_vertex_texture_coord_0.y, 0.0, 1.0);
	}
	// Texture coordinates
	else if (in_per_frame_uniform.render_mode == 3)
	{
		out_color = vec4(in_vertex_texture_coord_0.x, in_vertex_texture_coord_0.y, 0.0, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 4)
	{
		//out_color = vec4(in_vertex_texture_coord_1.x, in_vertex_texture_coord_1.y, 0.0, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 5)
	{
		//out_color = vec4(in_vertex_texture_coord_2.x, in_vertex_texture_coord_2.y, 0.0, 1.0);
	}
	// Geometric Data
	else if (in_per_frame_uniform.render_mode == 7)
	{
		out_color     = vec4(normalize(in_vertex_normal), 1.0);
		out_color.xyz = pow(out_color.xyz, vec3(2.2));        // Gamma corrected
	}
	else if (in_per_frame_uniform.render_mode == 8)        // Scaled to * .5 and + .5 to make it look nice on display
	{
		out_color     = vec4(normalize(in_vertex_normal) * .5 + .5, 1.0);
		out_color.xyz = pow(out_color.xyz, vec3(2.2));        // Gamma corrected
	}
	else if (in_per_frame_uniform.render_mode == 9)
	{
		//out_color = get_tangent();
	}
	else if (in_per_frame_uniform.render_mode == 10)
	{
		//out_color = vec4(get_bitangent(in_vertex_normal, get_tangent()), 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 11)
	{
		out_color = in_vertex_color_0;
	}
	else if (in_per_frame_uniform.render_mode == 12)
	{
		//out_color = in_vertex_color_1;
	}
	// Texture Maps
	else if (in_per_frame_uniform.render_mode == 14)
	{
		out_color = vec4(normalize(in_vertex_normal) * .5 + .5, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 15)
	{
		//float occlusion = texture(occlusion_sampler, get_occlusion_uvs()).r;
		//out_color       = vec4(occlusion, occlusion, occlusion, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 16)
	{
		//float roughness = texture(roughness_sampler, get_roughness_uvs()).g;
		//out_color       = vec4(roughness, roughness, roughness, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 17)
	{
		//float metallic = texture(metallic_sampler, get_metallic_uvs()).b;
		//out_color      = vec4(metallic, metallic, metallic, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 18)
	{
		//vec3 N    = texture(normal_sampler, get_normal_uvs()).xyz;
		//out_color = vec4(N, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 19)
	{
		//vec3 N    = texture(normal_sampler, get_normal_uvs()).xyz * 2.0 - vec3(1.0);
		//out_color = vec4(N, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 20)
	{
		//float height = texture(height_sampler, get_height_uvs()).a;
		//out_color    = vec4(height, height, height, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 21)
	{
		//out_color = texture(emissive_sampler, get_emissive_uvs());
	}
	// Shading Values
	else if (in_per_frame_uniform.render_mode == 23)
	{
		out_color = vec4(1.0);
	}
	else if (in_per_frame_uniform.render_mode == 24)
	{
		//out_color = vec4(get_normal(), 1.0);        // Scaled from -1 to 1 and multiplied with factor
	}
	else if (in_per_frame_uniform.render_mode == 25)
	{
		out_color = vec4(1.0);
	}
	else if (in_per_frame_uniform.render_mode == 26)
	{
		//float of  = in_material_factors.occlusion_factor;
		//out_color = vec4(of, of, of, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 27)
	{
		float rf  = 0.0;
		out_color = vec4(rf, rf, rf, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 28)
	{
		float mf  = 0.0;
		out_color = vec4(mf, mf, mf, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 29)
	{
		//float nf  = in_material_factors.normal_factor;
		//out_color = vec4(nf, nf, 1.0, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 30)
	{
		//out_color = in_material_factors.emissive_factor;
	}
	else if (in_per_frame_uniform.render_mode == 31)        // its clamped roughness * factor
	{
		float pr  = 0.0;
		out_color = vec4(pr, pr, pr, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 33)        // its material roughness squared
	{
		float fr  = 0.0;
		out_color = vec4(fr, fr, fr, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 34)        // Clamped metallic same as fragment metallic
	{
		float mm  = 0.0;
		out_color = vec4(mm, mm, mm, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 32)        // Same as material occlusion
	{
		float mo  = 0.0;
		out_color = vec4(mo, mo, mo, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 35)
	{
		out_color = vec4(1.0);
	}
	// Other Values
	else if (in_per_frame_uniform.render_mode == 37)
	{
		vec3 pc   = vec3(1.0);
		vec3 v    = normalize(clamp(pc, 0.0, 1.0));
		out_color = vec4(v, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 38)
	{
		out_color = vec4(1.0);
	}
	else if (in_per_frame_uniform.render_mode == 39)
	{
		out_color.xyz = vec3(0.0);
	}
	else if (in_per_frame_uniform.render_mode == 40)
	{
		out_color.xyz = vec3(1.0);
	}
	else if (in_per_frame_uniform.render_mode == 41)
	{
		vec3  N   = vec3(0.0);
		vec3  V   = vec3(0.0);;
		float NdV = clamp(dot(N, V), 0.0, 1.0);
		out_color = vec4(NdV, NdV, NdV, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 42)
	{
		float NoL   = clamp(dot(0.0, 1.0), 0.0, 1.0);
		out_color   = vec4(NoL, NoL, NoL, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 43)
	{
		vec3  H     = normalize(vec3(1.0));
		out_color   = vec4(H, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 44)
	{
		vec3  H     = normalize(vec3(1.0));
		float NoH   = clamp(0.0, 0.0, 1.0);
		out_color   = vec4(NoH, NoH, NoH, 1.0);
	}
	else if (in_per_frame_uniform.render_mode == 45)
	{
		vec3  H     = normalize(vec3(1.0));
		float VoH   = clamp(dot(H, H), 0.0, 1.0);
		out_color   = vec4(VoH, VoH, VoH, 1.0);
	}
}

void main()
{
	set_render_mode();
}