#ifndef _PANEL_ANIMATIONGRAPH
#define _PANEL_ANIMATIONGRAPH

#include "Panel.h"
#include <map>

struct Node;
struct Transition;
class ResourceAnimationGraph;
class ComponentAnimator;

class PanelAnimationGraph : public Panel
{
public:
	PanelAnimationGraph(const char * name, bool active);
	~PanelAnimationGraph();

	void Draw();
	void drawAnimationBox(Node* node) const;

	void selectNode(Node* node);

public:

	ImVec2 scrolling = { 0.0f,0.0f };
	bool showGrid = true;
	uint linkingNode = 0;

private:

	ComponentAnimator* component = nullptr;
	ResourceAnimationGraph* graph = nullptr;

	Node* selected_node = nullptr;
	Node* hovered_node = nullptr;
	Node* dragging_node = nullptr;
	ImVec2 clickOffset = { 0.0f,0.0f };

	Transition* selected_transition = nullptr;
	bool clickedLine = false;
};

#endif // !_PANEL_ANIMATIONGRAPH