#ifndef _MATERIAL
#define _MATERIAL
#include "Globals.h"
#include <string>

enum Mat_Wrap { W_UNKNOWN, CLAMP, CLAMP_TO_BORDER, REPEAT, MIRRORED_REPEAT };
enum Mat_MinMagFilter { M_UNKNOWN, NEAREST, LINEAR, MIPMAP_NEAREST, MIPMAP_LINEAR};
enum TextureType { DIFFUSE, AMBIENT, NORMALS, LIGHTMAP};

#define SIZE_CHECKERED 64

struct Texture
{
public:
	Texture();
	Texture(uint GL_id, const char* texture_name, bool compress = true);
	~Texture();

	uint getGLid() const { return gl_id; };
	uint getId() const { return id; };
	void getSize(int& x, int& y) const { x = size_x; y = size_y; };
	const char* getName() { return texture_name.c_str(); };

	void setParameters(Mat_Wrap wrap, Mat_MinMagFilter min_filter, Mat_MinMagFilter mag_filter);
	void LoadCheckered();

public:
	uint gl_id		= 0;

	Mat_Wrap wrap_mode				= W_UNKNOWN;
	Mat_MinMagFilter min_filter_mode = M_UNKNOWN;
	Mat_MinMagFilter mag_filter_mode = M_UNKNOWN;

private:

	std::string texture_name;
	const uint id	= 0;
	int size_x = 0;
	int size_y = 0;
};

class Material
{
public:

	Material();
	~Material();

	uint getTextureResource(TextureType tex_type) const;
	void setTextureResource(TextureType tex_type, uint textureResource); 
	void setCheckeredTexture(TextureType tex_type = DIFFUSE); // TODO: Should get a special resource from resource manager
	uint getId() const { return id; };

private:
	uint id = 0;
	uint diffuse_resource = 0;
	uint ambient_resource = 0;
	uint normals_resource = 0;
	uint lightmap_resource = 0;

};
#endif