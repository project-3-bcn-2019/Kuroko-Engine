#include "PanelAnimationGraph.h"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ModuleTimeManager.h"

#include "ComponentAnimator.h"
#include "ResourceAnimation.h"
#include "ResourceAnimationGraph.h"

PanelAnimationGraph::PanelAnimationGraph(const char * name, bool active) : Panel(name, active)
{
	showGrid = true;
}

PanelAnimationGraph::~PanelAnimationGraph()
{
}

void PanelAnimationGraph::Draw()
{
	ImGui::Begin(name.c_str(), &active, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	clickedLine = false;

	//Get the graph resource of the current selected game object
	bool hasGraph = false;
	if (App->scene->selected_obj.size() > 0)
	{
		bool use_same = true;	
		component = (ComponentAnimator*)App->scene->selected_obj.front()->getComponent(ANIMATOR);
		for (auto it = App->scene->selected_obj.begin(); it != App->scene->selected_obj.end(); ++it)
		{
			if ((ComponentAnimator*)(*it)->getComponent(ANIMATOR) != component)
			{
				use_same = false;
				break;
			}
		}

		if (component != nullptr && use_same)
		{
			graph = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());
			if (graph != nullptr)
				hasGraph = true;
		}
	}
	if (!hasGraph)
		graph = nullptr;

	// Create the child canvas
	if (ImGui::Button("Save Graph") && graph != nullptr)
	{
		graph->saveGraph();
		selected_node = nullptr;
		selected_transition = nullptr;
		hovered_node = nullptr;
		dragging_node = nullptr;
		clickedLine = false;
	}
	ImGui::SameLine();
	ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	ImGui::Checkbox("Show grid", &showGrid);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImU32 bckgColor = IM_COL32(50, 50, 55, 200);
	if (!hasGraph)
		bckgColor = IM_COL32(25, 25, 25, 200);
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, bckgColor);

	float posY = ImGui::GetCursorPosY();

	drawBlackboard();

	ImGui::SetCursorPos({ BLACKBOARD_WIDTH+10, posY });
	ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PushItemWidth(120.0f);

	ImVec2 offset = { ImGui::GetCursorScreenPos().x + scrolling.x, ImGui::GetCursorScreenPos().y + scrolling.y };
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	// Display grid
	if (showGrid)
	{
		ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
		if (!hasGraph)
			GRID_COLOR = IM_COL32(200, 200, 200, 15);
		float GRID_SZ = 64.0f;
		ImVec2 win_pos = ImGui::GetCursorScreenPos();
		ImVec2 canvas_sz = ImGui::GetWindowSize();
		for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
			draw_list->AddLine(ImVec2(x + win_pos.x, win_pos.y), ImVec2(x + win_pos.x, canvas_sz.y + win_pos.y), GRID_COLOR);
		for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
			draw_list->AddLine(ImVec2(win_pos.x, y + win_pos.y), ImVec2(canvas_sz.x + win_pos.x, y + win_pos.y), GRID_COLOR);
	}

	if (graph == nullptr)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		ImGui::End();
		return;
	}

	//Draw content
	draw_list->ChannelsSplit(3); //Split current channel into layers
	//Draw transition window
	if (selected_transition != nullptr)
		drawTransitionMenu();
	else
		hoveringTransitionMenu = false;
	draw_list->ChannelsSetCurrent(0);
	for (std::map<uint, Node*>::iterator it_n = graph->nodes.begin(); it_n != graph->nodes.end(); ++it_n)
	{
		Node* node = (*it_n).second;
		ImGui::PushID(node->UID);
		node->gridPos = { node->pos.x + offset.x,node->pos.y + offset.y };

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		//Node box
		ImU32 borderColor = IM_COL32(100, 100, 100, 255);
		ImU32 backgroundColor = IM_COL32(45, 45, 45, 230);
		float thickness = 1.0f;

		if (hovered_node == node)
		{
			borderColor = IM_COL32(150, 150, 100, 255);
		}
		if (selected_node == node)
		{
			borderColor = IM_COL32(255, 255, 75, 255);
			backgroundColor = IM_COL32(55, 55, 55, 245);
			thickness = 2.0f;
		}
		if (graph->start == node)
		{
			borderColor = IM_COL32(75, 75, 255, 255);
			if (hovered_node == node)
			{
				borderColor = IM_COL32(150, 150, 255, 255);
			}
		}
		if (App->time->getGameState() != GameState::STOPPED && node->UID == component->currentNode)
		{
			borderColor = IM_COL32(75, 255, 255, 255);
			thickness = 3.0f;
		}

		draw_list->AddRectFilled({ node->gridPos.x, node->gridPos.y }, { node->gridPos.x + node->size.x, node->gridPos.y + node->size.y }, backgroundColor, 4.0f);
		draw_list->AddRect({ node->gridPos.x, node->gridPos.y }, { node->gridPos.x + node->size.x, node->gridPos.y + node->size.y }, borderColor, 4.0f, 15, thickness);
		if (graph->start == node)
		{
			draw_list->AddRectFilled({ node->gridPos.x+node->size.x/3, node->gridPos.y -20}, { node->gridPos.x + node->size.x*2/3, node->gridPos.y+1}, backgroundColor, 1.0f);
			draw_list->AddRect({ node->gridPos.x + node->size.x / 3, node->gridPos.y -20}, { node->gridPos.x + node->size.x*2/3, node->gridPos.y+1}, borderColor, 1.0f, 15, thickness);
			ImGui::SetCursorScreenPos({ node->gridPos.x + node->size.x / 3 + 10, node->gridPos.y - 18 });
			ImGui::Text("Start");
		}

		//Node content
		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, { 5,5 });
		ImGui::SetCursorScreenPos({ node->gridPos.x + node->size.x - 70, node->gridPos.y + GRAPH_NODE_WINDOW_PADDING });
		ImGui::Text("Loop:");
		ImGui::SameLine();
		ImGui::Checkbox("##LoopCheck", &node->loop);
		ImGui::PopStyleVar();

		ImGui::SetCursorScreenPos({ node->gridPos.x + GRAPH_NODE_WINDOW_PADDING, node->gridPos.y + GRAPH_NODE_WINDOW_PADDING });
		ImGui::BeginGroup();
		ImGui::Text(node->name.c_str());

		ImGui::SetCursorScreenPos({ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + 5 });

		drawAnimationBox(node);

		ImGui::SetCursorScreenPos({ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + 5 });
		ImGui::Text("Speed:");
		ImGui::SameLine();
		/*ImGui::PushItemWidth(50);
		ImGui::InputFloat("##animSpeed", &node->speed);
		ImGui::PopItemWidth();*/
		
		uint clickedLink = node->drawLinks();
		if (clickedLink != 0)
			linkingNode = clickedLink;
		for (std::list<Transition*>::iterator it_t = node->transitions.begin(); it_t != node->transitions.end(); ++it_t)
		{
			if ((*it_t)->drawLine((*it_t) == selected_transition, (component->doingTransition != nullptr && component->doingTransition == (*it_t))))
			{
				selected_transition = (*it_t);
				clickedLine = true;
			}
		}

		//Node selection & linking
		ImGui::SetCursorScreenPos({ node->gridPos.x, node->gridPos.y });
		ImGui::InvisibleButton("node", { node->size.x, node->size.y });
		if (ImGui::IsItemHovered() && !hoveringTransitionMenu)
		{
			hovered_node = node;

			if (linkingNode != 0 && !ImGui::IsMouseDown(0))
			{
				NodeLink* linking = graph->getLink(linkingNode);
				if (linking != nullptr && linking->nodeUID != node->UID)
				{
					Node* originNode = graph->getNode(linking->nodeUID);
					if (originNode != nullptr && !originNode->checkLink(node))
					{
						NodeLink* linked = node->addLink(INPUT_LINK);
						linking->connect(linked->UID);
						linked->connect(linking->UID);

						originNode->connectLink(linkingNode);
						originNode->transitions.push_back(new Transition(linking->UID, linked->UID, graph->uuid));
					}
				}
				linkingNode = 0;
			}
			if (ImGui::IsMouseClicked(1))
			{
				ImGui::OpenPopup("##node info");
			}
		}
		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, { 5,5 });
		if (ImGui::BeginPopup("##node info"))
		{
			if (graph->start == node && graph->nodes.size() > 1)
				ImGui::Text("Cannot remove starting node");
			else
			{
				if (graph->nodes.size() > 1 && ImGui::Selectable("Set Start"))
					graph->start = node;
				if (ImGui::Selectable("Remove"))
				{
					Node* removed = (*it_n).second;
					bool last = false;
					if (++it_n == graph->nodes.end())
						last = true;
					graph->nodes.erase(removed->UID);
					//TODO remove transition and link pointers
					RELEASE(removed);
					if (last)
					{
						ImGui::EndPopup();
						ImGui::PopStyleVar();
						ImGui::EndGroup();
						ImGui::PopID();
						break;
					}
				}
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		if (linkingNode == 0 && ImGui::IsItemClicked() && !hoveringTransitionMenu)
		{
			selectNode(node);
			selected_transition = nullptr;
			dragging_node = node;
			clickOffset = { ImGui::GetMousePos().x - offset.x - node->pos.x, ImGui::GetMousePos().y - offset.y - node->pos.y };
		}

		ImGui::EndGroup();

		ImGui::PopID();
	}

	//Click to add nodes
	static ImVec2 pos = { 0,0 };
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(1))
	{
		ImGui::OpenPopup("##addNode");
		pos = ImGui::GetMousePos();
	}
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, { 5,5 });
	if (ImGui::BeginPopup("##addNode"))
	{
		if (ImGui::Selectable("Add Node"))
		{
			graph->addNode("Node", { pos.x - offset.x, pos.y - offset.y });
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	draw_list->ChannelsMerge();

	//Drag nodes
	if (!ImGui::IsMouseDown(0))
	{
		dragging_node = nullptr;
		linkingNode = 0;
	}
	if (!ImGui::IsAnyItemHovered())
	{
		hovered_node = nullptr;
		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
			selectNode(nullptr);
	}
	if (!clickedLine && ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
	{
		selected_transition = nullptr;
	}
	if (dragging_node != nullptr && ImGui::IsMouseDragging(0))
	{
		dragging_node->pos = { ImGui::GetMousePos().x - offset.x - clickOffset.x, ImGui::GetMousePos().y - offset.y - clickOffset.y };
	}

	//Drag window
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
		scrolling = { scrolling.x + ImGui::GetIO().MouseDelta.x, scrolling.y + ImGui::GetIO().MouseDelta.y };

	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::End();
}

void PanelAnimationGraph::selectNode(Node * node)
{
	selected_node = node;
}

void PanelAnimationGraph::drawAnimationBox(Node* node) const
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.y += 5.0f;

	draw_list->AddRectFilled(pos, { pos.x + 120, pos.y + 20 }, IM_COL32(25, 25, 25, 255), 1.0f);
	draw_list->AddRect(pos, { pos.x + 120, pos.y + 20 }, IM_COL32(150, 150, 150, 255), 1.0f, 15, 2.0f);

	ImGui::SetCursorScreenPos({ pos.x + GRAPH_NODE_WINDOW_PADDING/2, pos.y + GRAPH_NODE_WINDOW_PADDING / 4 });

	ResourceAnimation* anim = (ResourceAnimation*)App->resources->getResource(node->animationUID);
	ImGui::Text((anim == nullptr) ? "No Animation" : anim->asset.c_str());

	ImGui::SetCursorScreenPos(pos);
	static bool set_animation_menu = false;
	static uint clicked_node = 0;
	if (ImGui::InvisibleButton("animButton", { 120,20 }))
	{
		set_animation_menu = true;
		clicked_node = node->UID;
	}
	if (set_animation_menu && node->UID == clicked_node)
	{
		std::list<resource_deff> anims;
		App->resources->getAnimationResourceList(anims);

		ImGui::Begin("Animation selector", &set_animation_menu);
		for (auto it = anims.begin(); it != anims.end(); it++) {
			resource_deff anim_deff = (*it);
			if (ImGui::MenuItem(anim_deff.asset.c_str())) {
				App->resources->deasignResource(node->animationUID);
				App->resources->assignResource(anim_deff.uuid);
				node->animationUID = anim_deff.uuid;
				set_animation_menu = false;
				clicked_node = 0;
				break;
			}
		}
		ImGui::End();
	}
}

void PanelAnimationGraph::drawBlackboard()
{
	ImGui::BeginChild("Blackboard Panel", { BLACKBOARD_WIDTH,0 }, true);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, { 5,5 });
	ImGui::SetCursorPos({ 5,5 });
	ImGui::Text("Blackboard");

	if (graph != nullptr)
	{
		ImGui::SetCursorPosX(5);
		static bool addVar = false;
		if (ImGui::Button("Add Variable"))
		{
			ImGui::OpenPopup("##choose type");
			addVar = true;
		}

		if (addVar && ImGui::BeginPopup("##choose type", ImGuiWindowFlags_NoMove))
		{
			if (ImGui::Selectable("INT"))
			{
				graph->blackboard.push_back(new Variable(VAR_INT, ""));
				component->setInt(graph->blackboard.back()->uuid, 0);
				addVar = false;
			}
			if (ImGui::Selectable("FLOAT"))
			{
				graph->blackboard.push_back(new Variable(VAR_FLOAT, ""));
				component->setFloat(graph->blackboard.back()->uuid, 0.0f);
				addVar = false;
			}
			if (ImGui::Selectable("STRING"))
			{
				graph->blackboard.push_back(new Variable(VAR_STRING, ""));
				component->setString(graph->blackboard.back()->uuid, "");
				addVar = false;
			}
			if (ImGui::Selectable("BOOL"))
			{
				graph->blackboard.push_back(new Variable(VAR_BOOL, ""));
				component->setBool(graph->blackboard.back()->uuid, false);
				addVar = false;
			}

			ImGui::EndPopup();
		}

		ImGui::BeginChild("variables", { 0,0 }, true);

		static const char* types[LAST_TYPE] = { "INT:", "FLOAT:", "STRING:", "BOOL:" };
		int count = 0;
		for (std::list<Variable*>::iterator it = graph->blackboard.begin(); it != graph->blackboard.end(); ++it)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
			ImGui::BeginChildFrame(count+1, { 0, 45 }, ImGuiWindowFlags_NoScrollbar);
			ImGui::PopStyleVar();
			char name[25] = "";
			strcpy(name, (*it)->name.c_str());
			if (ImGui::InputText("Name", name, sizeof(name)))
			{
				(*it)->name = name;
			}
			ImGui::SameLine();
			if (ImGui::Button("X", { 20,20 }))
			{
				Variable* removed = (*it);
				bool last = false;
				if (++it == graph->blackboard.end())
					last = true;
				graph->blackboard.remove(removed);
				component->removeValue(removed->type, removed->uuid);
				RELEASE(removed);
				if (last)
				{
					ImGui::EndChildFrame();
					break;
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Remove");
			}

			ImGui::Text(types[(*it)->type]);

			ImGui::SameLine(50);
			ImGui::PushItemWidth(125);
			switch ((*it)->type)
			{
			case VAR_INT:
				ImGui::InputInt(("##INT" + std::to_string(count)).c_str(), component->getInt((*it)->uuid));
				break;
			case VAR_FLOAT:
				ImGui::InputFloat(("##FLOAT" + std::to_string(count)).c_str(), component->getFloat((*it)->uuid));
				break;
			case VAR_BOOL:
				ImGui::Checkbox(("##BOOL" + std::to_string(count)).c_str(), component->getBool((*it)->uuid));
				break;
			case VAR_STRING: //Has to be the last to avoid error of text initialization
				char text[25] = "";
				strcpy(text, component->getString((*it)->uuid)->c_str());
				if (ImGui::InputText(("##STRING" + std::to_string(count)).c_str(), text, sizeof(text)))
					component->setString((*it)->uuid, text);
				break;
			}
			ImGui::PopItemWidth();

			count++;
			ImGui::EndChildFrame();
		}

		ImGui::EndChild();
	}

	ImGui::PopStyleVar();
	ImGui::EndChild();
}

void PanelAnimationGraph::drawTransitionMenu()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSetCurrent(2);
	//Draw box
	float2 boxSize = { 200,300 };
	ImVec2 posA = { ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - boxSize.x - 5, ImGui::GetWindowPos().y + 5 };
	ImVec2 posB = { ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - 5, ImGui::GetWindowPos().y + boxSize.y + 5 };
	draw_list->AddRectFilled(posA, posB, IM_COL32(55, 55, 60, 255), 5.0f);
	draw_list->AddRect(posA, posB, IM_COL32(150, 150, 150, 255), 5.0f);

	//Draw box content
	ImGui::SetCursorScreenPos({ posA.x, posA.y });
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ChildBg, { 0,0,0,0 });
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, { 5,5 });
	ImGui::BeginChild("transtion menu title", { boxSize.x, boxSize.y }, false, ImGuiWindowFlags_NoScrollbar);
	if (ImGui::IsWindowHovered())
		hoveringTransitionMenu = true;
	else
		hoveringTransitionMenu = false;
	ImGui::SetCursorScreenPos({ posA.x + 5, posA.y + 5 });
	ImGui::BeginGroup();
	ImGui::Text("Transition");

	ImGui::SameLine(ImGui::GetContentRegionMax().x - 30);
	if (ImGui::Button("X", { 20,20 }))
	{
		ImGui::EndGroup();
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		Node* origin = graph->getNode(selected_transition->origin);
		NodeLink* output = graph->getLink(selected_transition->output);
		origin->transitions.remove(selected_transition);
		origin->removeLink(output); //It automatically removes input link and transition
		selected_transition = nullptr;
		return;
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Remove");
	}
	ImGui::PushItemWidth(50);
	
	ResourceAnimation* getAnim = (ResourceAnimation*)App->resources->getResource(selected_transition->destination);

	if (ImGui::InputFloat("Next starts after", &selected_transition->nextStart, 0.0f, 0.0f, "%.2f"))
	{
		if (selected_transition->nextStart > selected_transition->duration)
			selected_transition->nextStart = selected_transition->duration;
	}
	ImGui::SliderFloat("Duration", &selected_transition->duration, 0, 1 + selected_transition->nextStart, "%.2f");
	ImGui::PopItemWidth();

	if (ImGui::Button("Add condition"))
	{
		selected_transition->conditions.push_back(new Condition());
	}
	draw_list->AddLine({ posA.x, posA.y + 95 }, { posB.x, posA.y + 95 }, IM_COL32(150, 150, 150, 255));

	ImGui::SetCursorScreenPos({ posA.x + 5, posA.y + 100 });
	int count = 0;
	for (std::list<Condition*>::iterator it = selected_transition->conditions.begin(); it != selected_transition->conditions.end(); ++it)
	{		
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
		ImGui::BeginChildFrame(count + 1, { boxSize.x-10, 45 }, ImGuiWindowFlags_NoScrollbar);
		ImGui::PopStyleVar();

		Variable* var = graph->getVariable((*it)->variable_uuid);
		
		ImGui::PushItemWidth(80);
		if (ImGui::BeginCombo(("##Vars" + std::to_string(count)).c_str(), (var == nullptr)? (((*it)->type == CONDITION_FINISHED)? "Finished":"") : var->name.c_str()))
		{
			if (ImGui::Selectable(("Finished##" + std::to_string(count)).c_str()))
			{
				(*it)->type = CONDITION_FINISHED;
				(*it)->variable_uuid = 0;
			}
			for (std::list<Variable*>::iterator it_v = graph->blackboard.begin(); it_v != graph->blackboard.end(); ++it_v)
			{
				if (ImGui::Selectable(((*it_v)->name + "##" + std::to_string(count)).c_str()))
				{
					(*it)->variable_uuid = (*it_v)->uuid;
					if ((*it_v)->type == VAR_BOOL)
						(*it)->type = CONDITION_EQUALS;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		if (var != nullptr && (*it)->type != CONDITION_FINISHED)
		{
			ImGui::SameLine();
			static const char* types[6] = { "EQUALS", "DIFERENT", "GREATER", "LESS", "TRUE", "FALSE" };
			ImGui::PushItemWidth(75);
			if (ImGui::BeginCombo(("##Type" + std::to_string(count)).c_str(), (var->type == VAR_BOOL) ? types[(*it)->type + 4] : types[(*it)->type]))
			{
				if (var->type == VAR_BOOL || var->type == VAR_STRING)
				{
					if (ImGui::Selectable(types[4]))
					{
						(*it)->type = CONDITION_EQUALS;
					}
					if (ImGui::Selectable(types[5]))
					{
						(*it)->type = CONDITION_DIFERENT;
					}
				}
				else
				{
					for (int i = 0; i < 4; i++)
					{
						if (ImGui::Selectable(types[i]))
						{
							(*it)->type = (conditionType)i;
						}
					}
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
		}
		ImGui::SameLine(boxSize.x - 30);
		if (ImGui::Button("X", { 20,20 }))
		{
			Condition* removed = (*it);
			bool last = false;
			if (++it == selected_transition->conditions.end())
				last = true;
			selected_transition->conditions.remove(removed);
			RELEASE(removed);
			if (last)
			{
				ImGui::EndChildFrame();
				break;
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Remove");
		}
		if (var != nullptr)
		{
			float* value = &(*it)->conditionant;
			switch (var->type)
			{
			case VAR_INT:
			case VAR_FLOAT:
				ImGui::InputFloat(("Value##NUM" + std::to_string(count)).c_str(), value, 0.0f,0.0f, (var->type == VAR_INT)? "%.0f":"%.3f");
				break;
			case VAR_STRING:
				char text[25] = "";
				strcpy(text, (*it)->string_conditionant.c_str());
				if (ImGui::InputText(("Value##STRING" + std::to_string(count)).c_str(), text, sizeof(text)))
					(*it)->string_conditionant = text;
				break;
			}
		}

		ImGui::EndChildFrame();
		count++;
	}

	ImGui::EndGroup();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}
