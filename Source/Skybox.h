#ifndef _SKYBOX_
#define _SKYBOX_

#include <array>
#include <string>

#include "MathGeoLib\Math\float4x4.h"
#include "Color.h"

struct Texture;
class Mesh;

#define SKYBOX_BASE_DISTANCE 100.0f

enum Direction { LEFT, RIGHT, UP, DOWN, BACK, FRONT};

class Skybox
{
public:
	Skybox(float distance = SKYBOX_BASE_DISTANCE);
	~Skybox();

	void updatePosition(const float3& new_pos) { transform_mat.SetTranslatePart(new_pos); };
	void setAllTextures(std::array<Texture*, 6>& texs);
	void setTexture(Texture* tex, Direction index);
	void removeTexture(Direction index);
	Texture* getTexture(Direction index) { if (index >= 0 && index < 6) return textures[index]; };
	void Draw() const;
	void setDistance(float dist);

private:
	std::array<Texture*, 6> textures;
	std::array<Mesh*, 6> planes;   
	float4x4 transform_mat = float4x4::identity;

public:

	bool active = true;
	bool color_mode = false;
	Color color = White;
	float distance = 0.0f;
};


#endif