#ifndef _RESOURCE_ANIMATIONGRAPH
#define _RESOURCE_ANIMATIONGRAPH

#include "Resource.h"
#include <map>
#include <list>
#include "MathGeoLib\MathGeoLib.h"
#include "Random.h"

struct Node;
struct NodeLink;

enum variableType
{
	VAR_INT,
	VAR_FLOAT,
	VAR_STRING,
	VAR_BOOL,

	LAST_TYPE
};

struct Variable
{
	Variable(variableType type, const char* name): type(type), name(name), uuid(random32bits()) {}

	uint uuid = 0;
	variableType type = VAR_INT;
	std::string name = "";
};

enum linkType
{
	INPUT_LINK,
	OUTPUT_LINK
};

struct Transition
{
	Transition(NodeLink* output, NodeLink* input, uint graphUID);

	bool drawLine(bool selected, float2 offset);

	uint graphUID = 0;

	Node* origin = nullptr;
	Node* destination = nullptr;

	NodeLink* output = nullptr;
	NodeLink* input = nullptr;

	std::string usingLetter = "None";
	int sdlKeyValue = 0;
};

struct NodeLink
{
	NodeLink(linkType type, uint nodeUID, uint resourceUID);
	~NodeLink();

	void connect(uint nodeLinkUID) { connectedNodeLink = nodeLinkUID; }

	float2 nodePos = { 0,0 };

	uint UID = 0;
	uint nodeUID = 0;
	uint resourceUID = 0;
	uint connectedNodeLink = 0;

	linkType type = INPUT_LINK;

	bool linking = false;
};

struct Node
{
	Node(const char* name, uint graphUID, float2 pos, float2 size = { 150, 80 });
	Node(const char* name, uint graphUID, float2 pos, uint forced_UID);

	~Node();

	NodeLink* addLink(linkType type, bool addToList = true, uint forced_uid = 0);
	void removeLink(NodeLink* link);
	//Return true if any node is clicked
	uint drawLinks() const;

	bool checkLink(Node* node);
	void connectLink(uint linkUID);

	uint UID = 0;
	std::string name;
	float2 pos, gridPos, size = { 0.0f,0.0f };

	uint graphUID = 0;
	uint animationUID = 0;

	std::list<NodeLink*> links;
	int inputCount = 0;
	int outputCount = 0;

	std::list<Transition*> transitions;
};

class ResourceAnimationGraph : public Resource
{
public:
	ResourceAnimationGraph(resource_deff deff);
	~ResourceAnimationGraph();

	void LoadToMemory();
	bool LoadGraph();
	void UnloadFromMemory();

	bool saveGraph() const;

	Node* addNode(const char* name, float2 pos = { 20,20 });
	void pushLink(NodeLink* link);

	Node* getNode(uint UID);
	NodeLink* getLink(uint UID);

public:

	std::string Parent3dObject;

	std::map<uint, Node*> nodes;
	std::map<uint, NodeLink*> links;

	Node* start = nullptr;

	std::list<Variable*> blackboard;
};

#endif // !_RESOURCE_ANIMATIONGRAPH