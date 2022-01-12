/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3.0 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "clouds.h"
#include "noise.h"
#include "constants.h"
#include "debug.h"
#include "profiler.h"
#include "settings.h"


// Menu clouds are created later
class Clouds;
Clouds *g_menuclouds = NULL;
irr::scene::ISceneManager *g_menucloudsmgr = NULL;

static void cloud_3d_setting_changed(const std::string &settingname, void *data)
{
	// TODO: only re-read cloud settings, not height or radius
	((Clouds *)data)->readSettings();
}

static const std::vector<u16> quad_indices = []() {
	int quad_count = 0x10000 / 4; // max number of quads that can be drawn with 16-bit indices
	std::vector<u16> indices;
	indices.reserve(quad_count * 6);
	for (int k = 0; k < quad_count; k++) {
		indices.push_back(4 * k + 0);
		indices.push_back(4 * k + 1);
		indices.push_back(4 * k + 2);
		indices.push_back(4 * k + 2);
		indices.push_back(4 * k + 3);
		indices.push_back(4 * k + 0);
	}
	return indices;
}();

Clouds::Clouds(
		scene::ISceneNode* parent,
		scene::ISceneManager* mgr,
		s32 id,
		u32 seed,
		s16 cloudheight
):
	scene::ISceneNode(parent, mgr, id),
	m_seed(seed),
	m_camera_pos(0.0f, 0.0f),
	m_origin(0.0f, 0.0f),
	m_camera_offset(0.0f, 0.0f, 0.0f),
	m_color(1.0f, 1.0f, 1.0f, 1.0f)
{
	m_material.setFlag(video::EMF_LIGHTING, false);
	//m_material.setFlag(video::EMF_BACK_FACE_CULLING, false);
	m_material.setFlag(video::EMF_BACK_FACE_CULLING, true);
	m_material.setFlag(video::EMF_BILINEAR_FILTER, false);
	m_material.setFlag(video::EMF_FOG_ENABLE, true);
	m_material.setFlag(video::EMF_ANTI_ALIASING, true);
	//m_material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
	m_material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;

	m_params.density       = 0.4f;
	m_params.thickness     = 16.0f;
	m_params.color_bright  = video::SColor(229, 240, 240, 255);
	m_params.color_ambient = video::SColor(255, 0, 0, 0);
	m_params.speed         = v2f(0.0f, -2.0f);

	m_passed_cloud_y = cloudheight;
	readSettings();
	g_settings->registerChangedCallback("enable_3d_clouds",
		&cloud_3d_setting_changed, this);

	updateBox();
}

Clouds::~Clouds()
{
	g_settings->deregisterChangedCallback("enable_3d_clouds",
		&cloud_3d_setting_changed, this);
}

void Clouds::OnRegisterSceneNode()
{
	if(IsVisible)
	{
		SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
		//SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);
	}

	ISceneNode::OnRegisterSceneNode();
}

void Clouds::render()
{

	if (m_params.density <= 0.0f)
		return; // no need to do anything

	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if(SceneManager->getSceneNodeRenderPass() != scene::ESNRP_TRANSPARENT)
	//if(SceneManager->getSceneNodeRenderPass() != scene::ESNRP_SOLID)
		return;

	ScopeProfiler sp(g_profiler, "Rendering of clouds, avg", SPT_AVG);
	
	int num_faces_to_draw = m_enable_3d ? 6 : 1;
	
	m_material.setFlag(video::EMF_BACK_FACE_CULLING, m_enable_3d);

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	driver->setMaterial(m_material);
	
	/*
		Clouds move from Z+ towards Z-
	*/

	static const float cloud_size = BS * 64.0f;
	
	const float cloud_full_radius = cloud_size * m_cloud_radius_i;
	
	// Position of cloud noise origin from the camera
	v2f cloud_origin_from_camera_f = m_origin - m_camera_pos;
	// The center point of drawing in the noise
	v2f center_of_drawing_in_noise_f = -cloud_origin_from_camera_f;
	// The integer center point of drawing in the noise
	v2s16 center_of_drawing_in_noise_i(
		std::floor(center_of_drawing_in_noise_f.X / cloud_size),
		std::floor(center_of_drawing_in_noise_f.Y / cloud_size)
	);
	// The world position of the integer center point of drawing in the noise
	v2f world_center_of_drawing_in_noise_f = v2f(
		center_of_drawing_in_noise_i.X * cloud_size,
		center_of_drawing_in_noise_i.Y * cloud_size
	) + m_origin;

	/*video::SColor c_top(128,b*240,b*240,b*255);
	video::SColor c_side_1(128,b*230,b*230,b*255);
	video::SColor c_side_2(128,b*220,b*220,b*245);
	video::SColor c_bottom(128,b*205,b*205,b*230);*/
	video::SColorf c_top_f(m_color);
	video::SColorf c_side_1_f(m_color);
	video::SColorf c_side_2_f(m_color);
	video::SColorf c_bottom_f(m_color);
	c_side_1_f.r *= 0.95;
	c_side_1_f.g *= 0.95;
	c_side_1_f.b *= 0.95;
	c_side_2_f.r *= 0.90;
	c_side_2_f.g *= 0.90;
	c_side_2_f.b *= 0.90;
	c_bottom_f.r *= 0.80;
	c_bottom_f.g *= 0.80;
	c_bottom_f.b *= 0.80;
	video::SColor c_top = c_top_f.toSColor();
	video::SColor c_side_1 = c_side_1_f.toSColor();
	video::SColor c_side_2 = c_side_2_f.toSColor();
	video::SColor c_bottom = c_bottom_f.toSColor();

	// Get fog parameters for setting them back later
	video::SColor fog_color(0,0,0,0);
	video::E_FOG_TYPE fog_type = video::EFT_FOG_LINEAR;
	f32 fog_start = 0;
	f32 fog_end = 0;
	f32 fog_density = 0;
	bool fog_pixelfog = false;
	bool fog_rangefog = false;
	driver->getFog(fog_color, fog_type, fog_start, fog_end, fog_density,
			fog_pixelfog, fog_rangefog);
	
	// Set our own fog
	driver->setFog(fog_color, fog_type, cloud_full_radius * 0.5,
			cloud_full_radius*1.2, fog_density, fog_pixelfog, fog_rangefog);

	// Read noise

	const int grid_length = 2 * m_cloud_radius_i;
	std::vector<bool> grid(grid_length * grid_length);
	auto grid_index = [&] (int x, int z) -> int {
		return (z + m_cloud_radius_i) * grid_length + (x + m_cloud_radius_i);
	};
	auto grid_point = [&] (int x, int z) -> bool {
		return grid[grid_index(x, z)];
	};
	float cloud_size_noise = cloud_size / BS / 200;

	for(s16 zi = -m_cloud_radius_i; zi < m_cloud_radius_i; zi++) {
		u32 si = (zi + m_cloud_radius_i) * m_cloud_radius_i * 2 + m_cloud_radius_i;

		for (s16 xi = -m_cloud_radius_i; xi < m_cloud_radius_i; xi++) {
			u32 i = si + xi;

			v2s16 p_in_noise_i(
				xi + center_of_drawing_in_noise_i.X,
				zi + center_of_drawing_in_noise_i.Y
			);

			float noise = noise2d_perlin(
					(float)p_in_noise_i.X * cloud_size_noise,
					(float)p_in_noise_i.Y * cloud_size_noise,
					m_seed, 3, 0.5);
			// normalize to 0..1 (given 3 octaves)
			static const float noise_bound = 1.0f + 0.5f + 0.25f;
			float density = noise / noise_bound * 0.5f + 0.5f;
			grid[i] = (density < m_params.density);
		}
	}

	const float camera_y = m_camera_pos.Y + intToFloat(m_camera_offset, BS).Y;
	const float rel_y = camera_y - m_params.height * BS;
	const bool draw_top = !m_enable_3d || rel_y >= m_params.thickness * BS;
	const bool draw_bottom = rel_y < 0.0f;

	const v3f origin = v3f(world_center_of_drawing_in_noise_f.X, m_params.height * BS, world_center_of_drawing_in_noise_f.Y) - intToFloat(m_camera_offset, BS);
	const f32 rx = cloud_size;
	// if clouds are flat, the top layer should be at the given height
	const f32 ry = m_enable_3d ? m_params.thickness * BS : 0.0f;
	const f32 rz = cloud_size;

	// std::vector is great but it is slow
	// reserve+push/insert is slow because extending needs to check vector size
	// resize+direct access is slow because resize initializes the whole vector
	// so... malloc! it can't overflow as there can't be more than 6 quads per grid cell
	video::S3DVertex *buf = (video::S3DVertex *)malloc(grid.size() * num_faces_to_draw * 4 * sizeof(video::S3DVertex));
	video::S3DVertex *pv = buf;

	const v3f faces[6][4] = {
		{{0, ry, 0}, {0, ry, rz}, {rx, ry, rz}, {rx, ry, 0}}, // top
		{{0, ry, 0}, {rx, ry, 0}, {rx, 0, 0}, {0, 0, 0}}, // back
		{{rx, ry, 0}, {rx, ry, rz}, {rx, 0, rz}, {rx, 0, 0}}, // right
		{{rx, ry, rz}, {0, ry, rz}, {0, 0, rz}, {rx, 0, rz}}, // front
		{{0, ry, rz}, {0, ry, 0}, {0, 0, 0}, {0, 0, rz}}, // left
		{{rx, 0, rz}, {0, 0, rz}, {0, 0, 0}, {rx, 0, 0}}, // bottom
	};
	const v3f normals[6] = {{0, 1, 0}, {0, 0, -1}, {1, 0, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}};
	const video::SColor colors[6] = {c_top, c_side_1, c_side_2, c_side_1, c_side_2, c_bottom};
	const v2f tex_coords[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

	// Draw from back to front for proper transparency
	for (s16 zi0= 1-(int)m_cloud_radius_i; zi0 < m_cloud_radius_i-1; zi0++)
	for (s16 xi0= 1-(int)m_cloud_radius_i; xi0 < m_cloud_radius_i-1; xi0++)
	{
		s16 zi = zi0;
		s16 xi = xi0;
		if (zi >= 0)
			zi = m_cloud_radius_i - zi - 2;
		if (xi >= 0)
			xi = m_cloud_radius_i - xi - 2;

		if (!grid_point(xi, zi))
			continue;

		bool do_draw[6] = {
			draw_top,
			zi > 0 && !grid_point(xi, zi - 1),
			xi < 0 && !grid_point(xi + 1, zi),
			zi < 0 && !grid_point(xi, zi + 1),
			xi > 0 && !grid_point(xi - 1, zi),
			draw_bottom,
		};

		v3f const pos = origin + v3f(xi, 0.0f, zi) * cloud_size;
		for (int i = 0; i < num_faces_to_draw; i++) {
			if (!do_draw[i])
				continue;
			for (int k = 0; k < 4; k++) {
				pv->Pos = pos + faces[i][k];
				pv->Normal = normals[i];
				pv->Color = colors[i];
				pv->TCoords = tex_coords[k];
				pv++;
			}
		}
	}
	int vertex_count = pv - buf;
	int quad_count = vertex_count / 4;
	driver->drawVertexPrimitiveList(buf, vertex_count, quad_indices.data(), 2 * quad_count,
			video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);
	free(buf);

	// Restore fog settings
	driver->setFog(fog_color, fog_type, fog_start, fog_end, fog_density,
			fog_pixelfog, fog_rangefog);
}

void Clouds::step(float dtime)
{
	m_origin = m_origin + dtime * BS * m_params.speed;
}

void Clouds::update(v2f camera_p, video::SColorf color_diffuse)
{
	m_camera_pos = camera_p;
	m_color.r = MYMIN(MYMAX(color_diffuse.r * m_params.color_bright.getRed(),
			m_params.color_ambient.getRed()), 255) / 255.0f;
	m_color.g = MYMIN(MYMAX(color_diffuse.g * m_params.color_bright.getGreen(),
			m_params.color_ambient.getGreen()), 255) / 255.0f;
	m_color.b = MYMIN(MYMAX(color_diffuse.b * m_params.color_bright.getBlue(),
			m_params.color_ambient.getBlue()), 255) / 255.0f;
	m_color.a = m_params.color_bright.getAlpha() / 255.0f;
}

void Clouds::readSettings()
{
	m_params.height = (m_passed_cloud_y ? m_passed_cloud_y :
		g_settings->getS16("cloud_height"));
	m_cloud_radius_i = g_settings->getU16("cloud_radius");
	m_enable_3d = g_settings->getBool("enable_3d_clouds");
}
