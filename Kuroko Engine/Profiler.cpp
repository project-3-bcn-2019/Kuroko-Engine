#include "Profiler.h"

//===============================
//PROFILE SCOPE
//===============================

ProfileScope::ProfileScope() : name("") { timer.Start(); }
ProfileScope::ProfileScope(const char* str, ProfileScope* par) : name(str), parent(par) { timer.Start(); }

ProfileScope::~ProfileScope()
{
	for (int i = 0; i < scopes.size(); ++i)
		delete scopes[i];

	name.clear();
}

ProfileScope* ProfileScope::Find(const char* name)
{
	for (int i = 0; i < scopes.size(); ++i)
		if (scopes[i]->name.compare(name) == 0) return scopes[i];

	return nullptr;
}

void ProfileScope::Close()
{
	results.push_back(((float)timer.ReadMicroS()) / 1000.f);
}

//===============================
//PROFILER SYSTEM
//===============================

Profiler::Profiler() : base(ProfileScope("Engine", nullptr)), last_open_scope(&base), draw_curr(&base), frames(0), paused(false), starting_frame(0) {}

void Profiler::Clear()
{

}

void Profiler::AddLog(const char file[], int line, LogType lt, const char* format, ...) IM_FMTARGS(2)
{
	KLog push;
	va_list vargs;
	va_start(vargs, format);
	vsprintf_s(push.buf, sizeof(push.buf), format, vargs);
	va_end(vargs);

	sprintf_s(push.file, sizeof(push.file), file);

	push.buf[sizeof(push.buf) - 1] = '\n';
	push.file[sizeof(push.file) - 1] = '\n';
	push.line = line;
	push.lt = lt;

	logs.push_back(push);
}

void Profiler::LogDraw(const char* title, bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(title, p_open);
	if (ImGui::Button("Clear")) Clear();
	ImGui::SameLine();
	//if (ImGui::Button("Copy")) ImGui::LogToClipboard();
	//ImGui::SameLine();
	Filter.Draw("Filter", -100.0f);
	ImGui::Separator();
	ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (Filter.IsActive()) {
		for (int i = 0; i < logs.size(); ++i)
			if (Filter.PassFilter(logs[i].buf))
				ImGui::TextUnformatted(logs[i].buf);
	}
	else
	{
		for (int i = 0; i < logs.size(); ++i)
			ImGui::TextUnformatted(logs[i].buf);
	}

	if (ScrollToBottom)
		ImGui::SetScrollHere(1.0f);
	ScrollToBottom = false;
	ImGui::EndChild();
	ImGui::End();
}

// Profiling----------------------------------------------------------------------------------------------

void Profiler::AskScope(const char* name)
{
	ProfileScope* get = last_open_scope->Find(name);
	if (get != nullptr)
	{
		get->timer.Start();
	}
	else
	{
		last_open_scope->scopes.push_back(new ProfileScope(name, last_open_scope));
		get = last_open_scope->scopes[last_open_scope->scopes.size() - 1];
	}

	last_open_scope = get;

	bool caca;
}

void Profiler::CloseLastScope()
{
	last_open_scope->Close();
	if (last_open_scope->parent != nullptr)
		last_open_scope = last_open_scope->parent;
	else
		bool caca = true;
		//Report Frame
}

void Profiler::StartFrame()
{
	base.timer.Start();
}

void Profiler::CloseFrame()
{
	base.Close();
	++frames;
}


void Profiler::DrawScope(const char* title, bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(title, p_open);

	if (!paused)
	{
		if (draw_curr->results.size() > 240)
			ImGui::PlotLines("##TimeCost", &draw_curr->results[draw_curr->results.size() - 241], 240);
		else
			ImGui::PlotLines("##TimeCost", &draw_curr->results[0], draw_curr->results.size());
	}
	else
	{
		if (draw_curr->results.size() > 240)
			ImGui::PlotLines("##TimeCost", &draw_curr->results[starting_frame], ending_frame - starting_frame);
		else
			ImGui::PlotLines("##TimeCost", &draw_curr->results[starting_frame], ending_frame - starting_frame);
	}
	ImGui::SameLine();
	ImGui::Text("%.fsec", SDL_GetTicks() / 1000.f);
	ImGui::SameLine();
	// Controls
	// Pause Histogram
	if (ImGui::Button("||")) {
		if (paused)
			paused = false;
		else
		{
			paused = true;
			ending_frame = draw_curr->results.size() - 1;
			stopped_sec = SDL_GetTicks() / 1000.f;
			starting_frame = 0;
			if (draw_curr->results.size() > 240)
				starting_frame = ending_frame - 240;
		}
	}
	// Select Seconds to look at
	if (paused)
	{
		ImGui::SameLine();
		ImGui::Text("Stopped at frame:%d (%dsec)", ending_frame, stopped_sec);

		ImGui::SliderInt("Start", &starting_frame, 0, ending_frame);
		/*ImGui::SameLine();
		ImGui::DragInt("End", &ending_frame, starting_frame, draw_curr->results.size() - 1);*/
	}
	
	if (draw_curr->parent != nullptr && ImGui::Button("<- Back"))
		draw_curr = draw_curr->parent;

	for (int i = 0; i < draw_curr->scopes.size(); ++i)
	{
		std::string ButtonName(draw_curr->scopes[i]->name + "##" + std::to_string(i));
		if (ImGui::Button(ButtonName.c_str()))
			draw_curr = draw_curr->scopes[i];
	}
	// Selecte partition to look at
	/*if (ImGui::Button("Clear")) Clear();
	ImGui::SameLine();
	//if (ImGui::Button("Copy")) ImGui::LogToClipboard();
	//ImGui::SameLine();
	Filter.Draw("Filter", -100.0f);
	ImGui::Separator();
	ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (Filter.IsActive()) {
		for (int i = 0; i < logs.size(); ++i)
			if (Filter.PassFilter(logs[i].buf))
				ImGui::TextUnformatted(logs[i].buf);
	}
	else
	{
		for (int i = 0; i < logs.size(); ++i)
			ImGui::TextUnformatted(logs[i].buf);
	}

	if (ScrollToBottom)
		ImGui::SetScrollHere(1.0f);
	ScrollToBottom = false;
	ImGui::EndChild();*/
	ImGui::End();
}