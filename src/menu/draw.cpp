#include "menu.h"

namespace m {
static bool init_once = false;
static std::string process_name;
static std::string service_name;
static int tab = 0;
static bool popup_about = false;
static bool popup_settings = false;

static std::vector<std::string_view> tabs = {"Processes", "Services"};
static std::vector<std::string_view> process_info = {"Process Name", "PID"};
static std::vector<std::string_view> service_info = {"Service Name", "PID"};
static std::vector<std::string_view> dll_info = {"Dll Name", "Size (KB)"};

// settings
static std::vector<const char*> injection_types = {"Load Library", "Manual Map",
                                                   "Manual Map w. Shellcode"};

}  // namespace m
void process_column();
void dll_column();
void settings();
void about();

void menu::draw() {
  if (!m::init_once) {
    mem::iterate_processes();
    mem::iterate_services();
    mem::iterate_handles();
    m::init_once = true;
  }

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);

  ImGuiStyle& style = ImGui::GetStyle();

  ImGui::Begin("##main_menu", 0,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  const float win_sizex = ImGui::GetContentRegionAvail().x;
  const float win_sizey = ImGui::GetContentRegionAvail().y;

  ImGui::SeparatorText("Info");
  ImGui::InputTextWithHint(
      "##info", "No Process / Service Selected", &vars::process_selected.first,
      ImGuiInputTextFlags_ReadOnly);  // you can select the text :)

  ImGui::SameLine();
  if (ImGui::Button("Clear", ImVec2(-FLT_MIN, 0.f))) {
    vars::process_selected = {};
  }

  ImGui::BeginChild("##list_region", ImVec2(0.f, win_sizey - 185.f), 0,
                    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::MenuItem("Processes")) {
      m::tab = 0;
    }

    if (ImGui::MenuItem("Services")) {
      m::tab = 1;
    }

    ImGui::EndMenuBar();
  }

  process_column();
  ImGui::EndChild();

  ImGui::BeginChild("##dll_list_region",
                    ImVec2(0.f, ImGui::GetContentRegionAvail().y - 30.f), 0,
                    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse);

  if (ImGui::BeginMenuBar()) {
    if (ImGui::MenuItem("Load Dll")) {
      inj::open_file_dialog();
    }

    if (ImGui::MenuItem("Clear List")) {
      inj::empty_dll_list();
    }

    ImGui::EndMenuBar();
  }

  dll_column();
  ImGui::EndChild();

  if (ImGui::Button("About", ImVec2(80.f, -FLT_MIN))) {
    m::popup_about = true;
  }
  ImGui::SameLine();

  if (ImGui::Button("Settings", ImVec2(80.f, -FLT_MIN))) {
    m::popup_settings = true;
  }
  ImGui::SameLine();

  if (ImGui::Button("Inject", ImVec2(-FLT_MIN, -FLT_MIN))) {
    inj::inject();
  }

  ImGui::End();

  if (m::popup_about) {
    ImGui::OpenPopup("About");
    m::popup_about = false;
  }

  if (m::popup_settings) {
    ImGui::OpenPopup("Settings");
  }

  about();
  settings();
}

void process_column() {
  switch (m::tab) {
    case 0: {
      ImGui::InputTextWithHint("##filter01", "Process Name", &m::process_name);
      ImGui::SameLine();
      if (ImGui::Button("Refresh", ImVec2(-FLT_MIN, 0.f)))
        mem::iterate_processes();

      if (ImGui::BeginTable("process_table", (int)m::process_info.size(),
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn(m::process_info[0].data(),
                                ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(m::process_info[1].data(),
                                ImGuiTableColumnFlags_WidthFixed, 50.f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        static int selected_idx = -1;
        int idx = 0;
        for (const auto& i : mem::get_process_list()) {
          if (!utils::name_matches(i.ProcessName, m::process_name)) continue;
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::PushID(idx);
          if (ImGui::Selectable(i.ProcessName.c_str(), selected_idx == idx,
                                ImGuiSelectableFlags_SpanAllColumns)) {
            selected_idx = idx;
            vars::process_selected.first = i.ProcessName;
            vars::process_selected.second = i.PID;
          }
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%lu", i.PID);
          ImGui::PopID();
          ++idx;
        }

        ImGui::EndTable();
      }
    } break;
    case 1: {
      ImGui::InputTextWithHint("##filter02", "Service Name", &m::service_name);
      ImGui::SameLine();
      if (ImGui::Button("Refresh", ImVec2(-FLT_MIN, 0.f)))
        mem::iterate_services();
      if (ImGui::BeginTable("service_table", (int)m::service_info.size(),
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn(m::service_info[0].data(),
                                ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(m::service_info[1].data(),
                                ImGuiTableColumnFlags_WidthFixed, 50.f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        static int selected_idx = -1;
        int idx = 0;
        for (const auto& i : mem::get_service_list()) {
          if (!utils::name_matches(i.ServiceName, m::service_name)) continue;
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::PushID(idx);
          if (ImGui::Selectable(i.ServiceName.c_str(), selected_idx == idx,
                                ImGuiSelectableFlags_SpanAllColumns)) {
            selected_idx = idx;
            vars::process_selected.first = i.ServiceName;
            vars::process_selected.second = i.PID;
          }
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%lu", i.PID);
          ImGui::PopID();
          ++idx;
        }

        ImGui::EndTable();
      }

    } break;
  }
}

void dll_column() {
  if (ImGui::BeginTable("Dll List", m::dll_info.size(),
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollY)) {
    ImGui::TableSetupColumn(m::dll_info[0].data(),
                            ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(m::dll_info[1].data(),
                            ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();

    for (const auto& i : inj::get_dll_list()) {
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::Text(i.FileName.c_str());

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%lu", i.FileSize / 1024);
    }

    ImGui::EndTable();
  }
}

void settings() {
  ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
  if (ImGui::BeginPopupModal(
          "Settings", &m::popup_settings,
          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::SeparatorText("Syscall Settings");
    ImGui::Checkbox("Indirect Syscall", &vars::syscall_indirect);
    ImGui::Checkbox("Proxy Syscall", &vars::proxy_call);

    ImGui::SeparatorText("Injection Settings");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::Combo("##inject_type", &vars::injection_type,
                 m::injection_types.data(), m::injection_types.size());

    ImGui::Separator();

    ImGui::Checkbox("Handle Hijack", &vars::hijack_handle);
    ImGui::SameLine();
    ImGui::Checkbox("Thread Hijack", &vars::hijack_thread);
    ImGui::PopStyleVar();
    ImGui::EndPopup();
  }
}

void about() {
  ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2,
                                 ImGui::GetIO().DisplaySize.y / 2),
                          ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopup("About")) {
    ImGui::TextUnformatted("made with love and pastes");
    ImGui::TextUnformatted("by koling");
    ImGui::EndPopup();
  }
}