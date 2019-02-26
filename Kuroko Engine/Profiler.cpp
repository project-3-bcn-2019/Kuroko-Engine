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
	results.push_back((float)timer.ReadMicroS());
}

//===============================
//PROFILER SYSTEM
//===============================

Profiler::Profiler() : base(ProfileScope("Engine", nullptr)), last_open_scope(&base), draw_curr(&base), frames(0) {}

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

	ImGui::PlotLines("Time Cost", &draw_curr->results[0], draw_curr->results.size());

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