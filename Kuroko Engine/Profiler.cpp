#include "Profiler.h"

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