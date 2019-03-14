#ifndef _COMPONENTTEXTUI_
#define _COMPONENTTEXTUI_
#include "Component.h"
#include "MathGeoLib/Math/float2.h"
#include "MathGeoLib/Math/float3.h"
#include <vector>


#define MAX_CHARS 32

class ComponentRectTransform;
class Font;

class ComponentTextUI :
	public Component
{

	struct Label {
		Font* font = nullptr;
		float3 color = float3(1, 1, 1);
		float2 textOrigin = float2(0, 0);
		std::string text = std::string("Default Text.");
	};
	class CharPlane {
	public:
		void GenBuffer(float2 size);

		float3* vertex = nullptr;
		int vertexID = -1;
		int textureID = -1;
	};
public:
	ComponentTextUI(GameObject* parent);
	ComponentTextUI(JSON_Object* deff, GameObject* parent);
	~ComponentTextUI();

	bool Update(float dt)override;
	void Draw() const override;
	bool DrawInspector(int id = 0) override;

	void Save(JSON_Object* config) override;

	void ShiftNewLine(float3 & cursor, int & current_line, int i);
	void SetFontScale(int scale);
	void SetFont(const char * font);
	void CleanCharPlanes();
	void SetText(const char * txt);
	void EnframeLabel(float3 * points);
	float3 GetCornerLabelPoint(int corner);
	void AddCharPanel(char character, bool first);
	void FillCharPlanes();

	Label label;
	bool drawCharPanel = false;
	bool drawLabelrect = false;

private:

	ComponentRectTransform * rectTransform = nullptr;

	float2* texCoords = nullptr;
	float3 labelFrame[4];
	float initOffsetX = 0.0f;
	
	int lineSpacing = 30;
	float alpha = 1.0f; //not functional yet


	std::vector<CharPlane*> charPlanes;
	std::vector<float3> offsetPlanes;
};


#endif