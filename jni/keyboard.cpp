#include "main.h"

#include "gui/gui.h"
#include "game/game.h"
#include "keyboard.h"

extern CGUI *pGUI;

CKeyBoard::CKeyBoard()
{
	Log("Initalizing KeyBoard..");

	ImGuiIO& io = ImGui::GetIO();
	m_Size = ImVec2(io.DisplaySize.x, io.DisplaySize.y * 0.65);
	m_Pos = ImVec2(0, io.DisplaySize.y * (1-0.65));
	m_fFontSize = pGUI->ScaleY(70.0f);
	m_fKeySizeY = m_Size.y / 6;

	Log("Size: %f, %f. Pos: %f, %f", m_Size.x, m_Size.y, m_Pos.x, m_Pos.y);
	Log("font size: %f. Key's height: %f", m_fFontSize, m_fKeySizeY);

	m_bEnable = false;
	m_iLayout = LAYOUT_ENG;
	m_iTemp = LAYOUT_ENG;
	m_iCase = LOWER_CASE;
	m_iPushedKey = -1;

	m_utf8Input[0] = '\0';
	m_iInputOffset = 0;

	InitENG();
	InitTH();
	InitNUM();
}

CKeyBoard::~CKeyBoard()
{
}

void CKeyBoard::Render()
{
	if(!m_bEnable) return;

	ImGuiIO& io = ImGui::GetIO();

	// background
	ImGui::GetOverlayDrawList()->AddRectFilled(	m_Pos, ImVec2(m_Size.x, io.DisplaySize.y), 0xB0000000);

	// input string
	ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize, 
		ImVec2(m_Pos.x + m_Size.x * 0.02, m_Pos.y + m_Pos.y * 0.05), 0xFFFFFFFF, m_utf8Input);

	// dividing line
	ImGui::GetOverlayDrawList()->AddLine(
		ImVec2(m_Pos.x, m_Pos.y + m_fKeySizeY), 
		ImVec2(m_Size.x, m_Pos.y + m_fKeySizeY), 0xFF3291F5);

	float fKeySizeY = m_fKeySizeY;

	for(int i=0; i<MAX_INPUT_LINE; i++)
	{
		for( auto key : m_Rows[m_iLayout][i])
		{
			if(key.id == m_iPushedKey && key.type != KEY_SPACE)
				ImGui::GetOverlayDrawList()->AddRectFilled(
					key.pos, 
					ImVec2(key.pos.x + key.width, key.pos.y + fKeySizeY),
					0xFF3291F5);

			switch(key.type)
			{
				case KEY_DEFAULT:
					ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize, key.symPos, 0xFFFFFFFF, key.name[m_iCase]);
				break;

				case KEY_SHIFT:
					ImGui::GetOverlayDrawList()->AddTriangleFilled(
						ImVec2(key.pos.x + key.width * 0.37, key.pos.y + fKeySizeY * 0.50),
						ImVec2(key.pos.x + key.width * 0.50, key.pos.y + fKeySizeY * 0.25),
						ImVec2(key.pos.x + key.width * 0.63, key.pos.y + fKeySizeY * 0.50),
						m_iCase == LOWER_CASE ? 0xFF8A8886 : 0xFF3291F5);
					ImGui::GetOverlayDrawList()->AddRectFilled(
						ImVec2(key.pos.x + key.width * 0.44, key.pos.y + fKeySizeY * 0.5 - 1),
						ImVec2(key.pos.x + key.width * 0.56, key.pos.y + fKeySizeY * 0.68),
						m_iCase == LOWER_CASE ? 0xFF8A8886 : 0xFF3291F5);
				break;

				case KEY_BACKSPACE:
					static ImVec2 points[5];
					points[0] = ImVec2(key.pos.x + key.width * 0.35, key.pos.y + fKeySizeY * 0.5);
					points[1] = ImVec2(key.pos.x + key.width * 0.45, key.pos.y + fKeySizeY * 0.25);
					points[2] = ImVec2(key.pos.x + key.width * 0.65, key.pos.y + fKeySizeY * 0.25);
					points[3] = ImVec2(key.pos.x + key.width * 0.65, key.pos.y + fKeySizeY * 0.65);
					points[4] = ImVec2(key.pos.x + key.width * 0.45, key.pos.y + fKeySizeY * 0.65);
					ImGui::GetOverlayDrawList()->AddConvexPolyFilled(points, 5, 0xFF8A8886);
				break;

				case KEY_NUM:
					ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize, key.symPos, 0xFFFFFFFF, "num");
				break;
				
				case KEY_SWITCH:
					ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize, key.symPos, 0xFFFFFFFF, "lang");
				break;

				case KEY_SPACE:
				ImGui::GetOverlayDrawList()->AddRectFilled(
					ImVec2(key.pos.x + key.width * 0.07, key.pos.y + fKeySizeY * 0.3),
					ImVec2(key.pos.x + key.width * 0.93, key.pos.y + fKeySizeY * 0.7),
					key.id == m_iPushedKey ? 0xFF3291F5 : 0xFF8A8886);
				break;

				case KEY_SEND:
					ImGui::GetOverlayDrawList()->AddTriangleFilled(
						ImVec2(key.pos.x + key.width * 0.3, key.pos.y + fKeySizeY * 0.25),
						ImVec2(key.pos.x + key.width * 0.3, key.pos.y + fKeySizeY * 0.75), 
						ImVec2(key.pos.x + key.width * 0.7, key.pos.y + fKeySizeY * 0.50),
						0xFF8A8886);
				break;
			}
		}
	}
}

void CKeyBoard::Open(keyboard_callback* handler)
{
	if(handler == nullptr) return;

	Close();

	m_pHandler = handler;
	m_bEnable = true;
}

void CKeyBoard::Close()
{
	m_bEnable = false;

	m_sInput.clear();
	m_iInputOffset = 0;
	m_utf8Input[0] = 0;
	m_iCase = LOWER_CASE;
	m_iPushedKey = -1;
	m_pHandler = nullptr;

	return;
}

bool CKeyBoard::OnTouchEvent(int type, bool multi, int x, int y)
{
	static bool bWannaClose = false;

	if(!m_bEnable) return true;

	if(type == TOUCH_PUSH && y < m_Pos.y) bWannaClose = true;
	if(type == TOUCH_POP && y < m_Pos.y && bWannaClose)
	{
		bWannaClose = false;
		Close();
		return false;
	}

	m_iPushedKey = -1;

	kbKey* key = GetKeyFromPos(x, y);
	if(!key) return false;

	switch(type)
	{
		case TOUCH_PUSH:
		m_iPushedKey = key->id;
		break;

		case TOUCH_MOVE:
		m_iPushedKey = key->id;
		break;

		case TOUCH_POP:
			HandleInput(*key);
		break;
	}

	return false;
}

void CKeyBoard::HandleInput(kbKey &key)
{
	switch(key.type)
	{
		case KEY_DEFAULT:

		case KEY_SPACE:
			AddCharToInput(key.code[m_iCase]);
		break;

		case KEY_NUM:
			if (m_iLayout == LAYOUT_NUM) {
				m_iLayout = m_iTemp;
			}
			else {
				m_iLayout = LAYOUT_NUM;
			}
			m_iCase = LOWER_CASE;
		break;

		case KEY_SWITCH:
			if (m_iLayout == LAYOUT_ENG) {
				m_iLayout = LAYOUT_TH;
			}
			else {
				m_iLayout = LAYOUT_ENG;
			}
			m_iTemp = m_iLayout;

			m_iCase = LOWER_CASE;
		break;

		case KEY_BACKSPACE:
			DeleteCharFromInput();
		break;

		case KEY_SHIFT:
			m_iCase ^= 1;
		break;

		case KEY_SEND:
			Send();
		break;

	}
}

void CKeyBoard::AddCharToInput(char sym)
{
	if(m_sInput.length() < MAX_INPUT_LEN && sym)
	{
		m_sInput.push_back(sym);
		iso_8859_11_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);

	check:
		ImVec2 textSize = pGUI->GetFont()->CalcTextSizeA(m_fFontSize, FLT_MAX, 0.0f,  m_utf8Input, nullptr, nullptr);
		if(textSize.x >= (m_Size.x - (m_Size.x * 0.04)))
		{
			m_iInputOffset++;
			iso_8859_11_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);
			goto check;
		}
	}
}

void CKeyBoard::DeleteCharFromInput()
{
	if (!m_sInput.length()) return;

	ImVec2 textSize;
	m_sInput.pop_back();

	check:
	if(m_iInputOffset == 0) goto ret;
	iso_8859_11_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset-1]);
	textSize = pGUI->GetFont()->CalcTextSizeA(m_fFontSize, FLT_MAX, 0.0f,  m_utf8Input, nullptr, nullptr);

	if(textSize.x <= (m_Size.x - (m_Size.x * 0.04)))
	{
		m_iInputOffset--; 
		goto check;
	}
	else
	{
		ret:
		iso_8859_11_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);
		return;
	}
}

void CKeyBoard::Send()
{
	if(m_pHandler) m_pHandler(m_sInput.c_str());
	m_bEnable = false;
}

kbKey* CKeyBoard::GetKeyFromPos(int x, int y)
{
	int iRow = (y-m_Pos.y) / m_fKeySizeY;
	if(iRow <= 0) return nullptr;

	for(auto key : m_Rows[m_iLayout][iRow-1])
	{
		if( x >= key.pos.x && x <= (key.pos.x + key.width) )
			return &key;
	}

	Log("UNKNOWN KEY");
	return nullptr;
}

void CKeyBoard::InitENG()
{
	ImVec2 curPos;
	std::vector<kbKey> *row = nullptr;
	float defWidth = m_Size.x/10;

	struct kbKey key;
	key.type = KEY_DEFAULT;
	key.id = 0;

	// 1- Start InitENG
	row = &m_Rows[LAYOUT_ENG][0];
	curPos = ImVec2(0, m_Pos.y + m_fKeySizeY);

	// _
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '_';
	key.code[UPPER_CASE] = '_';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "_");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "_");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// @
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '@';
	key.code[UPPER_CASE] = '@';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "@");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "@");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// /
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '/';
	key.code[UPPER_CASE] = '/';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "/");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "/");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// -
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '-';
	key.code[UPPER_CASE] = '-';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "-");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "-");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// #
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '#';
	key.code[UPPER_CASE] = '#';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "#");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "#");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// $
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '$';
	key.code[UPPER_CASE] = '$';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "$");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "$");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// %
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '%';
	key.code[UPPER_CASE] = '%';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "%");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "%");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// *
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '*';
	key.code[UPPER_CASE] = '*';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "*");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "*");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// (
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '(';
	key.code[UPPER_CASE] = '(';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "(");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "(");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// )
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ')';
	key.code[UPPER_CASE] = ')';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], ")");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ")");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	row = &m_Rows[LAYOUT_ENG][1];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// q / Q
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'q';
	key.code[UPPER_CASE] = 'Q';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "q");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Q");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// w / W
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'w';
	key.code[UPPER_CASE] = 'W';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "w");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "W");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// e / E
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'e';
	key.code[UPPER_CASE] = 'E';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "e");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "E");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// r / R
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'r';
	key.code[UPPER_CASE] = 'R';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "r");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "R");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// t / T
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 't';
	key.code[UPPER_CASE] = 'T';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "t");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "T");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// y / Y
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'y';
	key.code[UPPER_CASE] = 'Y';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "y");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Y");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// u / U
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'u';
	key.code[UPPER_CASE] = 'U';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "u");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "U");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// i / I
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'i';
	key.code[UPPER_CASE] = 'I';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "i");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "I");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// o / O
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'o';
	key.code[UPPER_CASE] = 'O';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "o");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "O");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// p / P
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'p';
	key.code[UPPER_CASE] = 'P';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "p");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "P");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// newline
	row = &m_Rows[LAYOUT_ENG][2];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// a / A
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'a';
	key.code[UPPER_CASE] = 'A';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "a");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "A");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// s / S
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 's';
	key.code[UPPER_CASE] = 'S';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "s");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "S");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// d / D
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'd';
	key.code[UPPER_CASE] = 'D';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "d");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "D");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// f / F
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'f';
	key.code[UPPER_CASE] = 'F';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "f");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "F");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// g / G
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'g';
	key.code[UPPER_CASE] = 'G';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "g");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "G");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// h / H
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'h';
	key.code[UPPER_CASE] = 'H';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "h");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "H");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// j / J
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'j';
	key.code[UPPER_CASE] = 'J';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "j");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "J");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// k / K
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'k';
	key.code[UPPER_CASE] = 'K';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "k");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "K");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// l / L
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'l';
	key.code[UPPER_CASE] = 'L';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "l");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "L");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;


	// ; / :
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ';';
	key.code[UPPER_CASE] = ':';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], ";");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ":");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// ' / "
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '\'';
	key.code[UPPER_CASE] = '"';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "'");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "\"");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// newline
	row = &m_Rows[LAYOUT_ENG][3];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// SHIFT
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth * 1.5;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SHIFT;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	// z / Z
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'z';
	key.code[UPPER_CASE] = 'Z';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "z");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Z");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// x / X
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'x';
	key.code[UPPER_CASE] = 'X';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "x");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "X");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// c / C
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'c';
	key.code[UPPER_CASE] = 'C';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "c");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "C");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// v / V
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'v';
	key.code[UPPER_CASE] = 'V';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "v");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "V");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// b / B
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'b';
	key.code[UPPER_CASE] = 'B';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "b");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "B");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// n / N
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'n';
	key.code[UPPER_CASE] = 'N';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "n");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "N");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// m / M
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'm';
	key.code[UPPER_CASE] = 'M';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "m");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "M");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// Del
	key.pos = curPos;
	key.symPos = ImVec2(0,0);
	key.width = defWidth * 1.5;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_BACKSPACE;
	key.id++;
	row->push_back(key);

	// newline
	row = &m_Rows[LAYOUT_ENG][4];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	key.type = KEY_DEFAULT;

	// /
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '/';
	key.code[UPPER_CASE] = '/';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "/");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "/");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// Num
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_NUM;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// Switch Lang
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SWITCH;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// Space
	key.pos = curPos;
	key.symPos = ImVec2(0,0);
	key.width = defWidth * 4;
	key.code[LOWER_CASE] = ' ';
	key.code[UPPER_CASE] = ' ';
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SPACE;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	// ?
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '?';
	key.code[UPPER_CASE] = '?';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "?");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "?");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// !
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '!';
	key.code[UPPER_CASE] = '!';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "!");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "!");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	// Send
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SEND;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	return;
}

void CKeyBoard::InitTH()
{
	ImVec2 curPos;
	std::vector<kbKey> *row = nullptr;
	float defWidth = m_Size.x/14;

	struct kbKey key;
	key.type = KEY_DEFAULT;
	key.id = 0;

	// 1-?? ???
	row = &m_Rows[LAYOUT_TH][0];
	Log("row layout %d", row);
	curPos = ImVec2(0, m_Pos.y + m_fKeySizeY);

	Log("===_/%");
	key.pos = curPos;
	Log("============= 1");
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	Log("============= 2");
	key.width = defWidth;
	Log("============= 3");
	key.code[LOWER_CASE] = '_';
	Log("============= 4");
	key.code[UPPER_CASE] = '%';
	Log("============= 5");
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "_");
	Log("============= 6");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "%");
	Log("============= 7");
	key.id++;
	Log("============= 8");
	Log("key id %d, key.pos %d", key.id, key.pos);
	row->push_back(key);
	Log("============= 9");
	curPos.x += key.width;

	Log("===@/+");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '@';
	key.code[UPPER_CASE] = '+';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "@");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "+");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===/ / ñ");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '/';
	key.code[UPPER_CASE] = 'ñ';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "/");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ñ");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===- / ò");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '-';
	key.code[UPPER_CASE] = 'ò';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "-");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ò");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===À / ó");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'À';
	key.code[UPPER_CASE] = 'ó';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "À");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ó");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===¶ / ô");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¶';
	key.code[UPPER_CASE] = 'ô';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¶");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ô");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ø / Ù");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ø';
	key.code[UPPER_CASE] = 'Ù';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ø");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Ù");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ö / ß");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ö';
	key.code[UPPER_CASE] = 'ß';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ö");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ß");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===¤ / õ");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¤';
	key.code[UPPER_CASE] = 'õ';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¤");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "õ");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===µ / ö");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'µ';
	key.code[UPPER_CASE] = 'ö';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "µ");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ö");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===¨ / ÷");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¨';
	key.code[UPPER_CASE] = '÷';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¨");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "÷");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===¢ / ø");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¢';
	key.code[UPPER_CASE] = 'ø';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¢");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ø");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===ª / ù");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'ª';
	key.code[UPPER_CASE] = 'ù';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "ª");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ù");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===");
	row = &m_Rows[LAYOUT_TH][1];
	curPos.x = defWidth * 0.5;
	curPos.y += m_fKeySizeY;

	Log("===æ / ð");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'æ';
	key.code[UPPER_CASE] = 'ð';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "æ");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ð");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===ä / \" ");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'ä';
	key.code[UPPER_CASE] = '\"';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "ä");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "\"");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ó / ®");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ó';
	key.code[UPPER_CASE] = '®';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ó");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "®");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===¾ / ±");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¾';
	key.code[UPPER_CASE] = '±';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¾");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "±");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ð / ¸");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ð';
	key.code[UPPER_CASE] = '¸';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ð");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "¸");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ñ / í");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ñ';
	key.code[UPPER_CASE] = 'í';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ñ");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "í");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Õ / ê");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Õ';
	key.code[UPPER_CASE] = 'ê';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Õ");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ê");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ã / ³");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ã';
	key.code[UPPER_CASE] = '³';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ã");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "³");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===¹ / Ï");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¹';
	key.code[UPPER_CASE] = 'Ï';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¹");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Ï");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;


	Log("===Â / ­");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Â';
	key.code[UPPER_CASE] = '­';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Â");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "­");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;


	Log("===º / °");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'º';
	key.code[UPPER_CASE] = '°';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "º");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "°");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Å / ,");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Å';
	key.code[UPPER_CASE] = ',';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Å");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ",");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===£ / ¥");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '£';
	key.code[UPPER_CASE] = '¥';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "£");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "¥");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===");
	defWidth = m_Size.x/13;

	row = &m_Rows[LAYOUT_TH][2];
	curPos.x = defWidth * 0.5;
	curPos.y += m_fKeySizeY;

	Log("===¿ / Ä");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¿';
	key.code[UPPER_CASE] = 'Ä';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¿");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Ä");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ë / ¦");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ë';
	key.code[UPPER_CASE] = '¦';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ë");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "¦");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===¡ / ¯");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¡';
	key.code[UPPER_CASE] = '¯';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¡");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "¯");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===´ / â");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '´';
	key.code[UPPER_CASE] = 'â';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "´");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "â");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===à / ¬");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'à';
	key.code[UPPER_CASE] = '¬';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "à");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "¬");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===é / ç");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'é';
	key.code[UPPER_CASE] = 'ç';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "é");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ç");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===è/ ë");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'è';
	key.code[UPPER_CASE] = 'ë';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "è");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ë");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;


	Log("===Ò / É");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ò';
	key.code[UPPER_CASE] = 'É';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ò");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "É");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;


	Log("===Ê / È");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ê';
	key.code[UPPER_CASE] = 'È';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ê");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "È");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Ç / «");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ç';
	key.code[UPPER_CASE] = '«';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ç");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "«");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===§ / .");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '§';
	key.code[UPPER_CASE] = '.';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "§");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ".");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	defWidth = m_Size.x/14;

	Log("===");
	row = &m_Rows[LAYOUT_TH][3];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;
	
	Log("===Shift");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth * 1.5;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SHIFT;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	Log("===¼ / (");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '¼';
	key.code[UPPER_CASE] = '(';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "¼");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "(");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===» / )");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '»';
	key.code[UPPER_CASE] = ')';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "»");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ")");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===á / ©");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'á';
	key.code[UPPER_CASE] = '©';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "á");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "©");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Í / Î");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Í';
	key.code[UPPER_CASE] = 'Î';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Í");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Î");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("=== Ô/  Ú");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Ô';
	key.code[UPPER_CASE] = 'Ú';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Ô");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Ú");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===× / ì");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '×';
	key.code[UPPER_CASE] = 'ì';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "×");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "ì");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===· / ?");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '·';
	key.code[UPPER_CASE] = '?';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "·");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "?");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;


	Log("===Á / ²");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'Á';
	key.code[UPPER_CASE] = '²';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "Á");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "²");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;


	Log("===ã / Ì");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'ã';
	key.code[UPPER_CASE] = 'Ì';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "ã");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Ì");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===½ / Æ");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '½';
	key.code[UPPER_CASE] = 'Æ';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "½");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "Æ");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Del");
	key.pos = curPos;
	key.symPos = ImVec2(0,0);
	key.width = defWidth * 1.5;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_BACKSPACE;
	key.id++;
	row->push_back(key);

	Log("===");
	row = &m_Rows[LAYOUT_TH][4];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	key.type = KEY_DEFAULT;

	defWidth = m_Size.x/10;

	Log("===slash /");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '/';
	key.code[UPPER_CASE] = '/';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "/");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "/");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===comma ,");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_NUM;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===switch lang");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SWITCH;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===space");
	key.pos = curPos;
	key.symPos = ImVec2(0,0);
	key.width = defWidth * 4;
	key.code[LOWER_CASE] = ' ';
	key.code[UPPER_CASE] = ' ';
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SPACE;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	Log("===?");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '?';
	key.code[UPPER_CASE] = '?';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "?");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "?");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===!");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '!';
	key.code[UPPER_CASE] = '!';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "!");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "!");
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("===Send");
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SEND;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;

	Log("finished Added");
	return;
}

void CKeyBoard::InitNUM()
{
	ImVec2 curPos;
	std::vector<kbKey> *row = nullptr;
	float defWidth = m_Size.x/10;

	struct kbKey key;
	key.type = KEY_DEFAULT;
	key.id = 0;

	row = &m_Rows[LAYOUT_NUM][1];
	curPos = ImVec2(0, m_Pos.y + m_fKeySizeY + m_fKeySizeY);

	// 1
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '1';
	key.code[UPPER_CASE] = '1';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "1");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "1");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 2
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '2';
	key.code[UPPER_CASE] = '2';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "2");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "2");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 3
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '3';
	key.code[UPPER_CASE] = '3';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "3");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "3");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 4
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '4';
	key.code[UPPER_CASE] = '4';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "4");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "4");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 5
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '5';
	key.code[UPPER_CASE] = '5';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "5");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "5");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 6
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '6';
	key.code[UPPER_CASE] = '6';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "6");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "6");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 7
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '7';
	key.code[UPPER_CASE] = '7';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "7");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "7");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 8
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '8';
	key.code[UPPER_CASE] = '8';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "8");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "8");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 9
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '9';
	key.code[UPPER_CASE] = '9';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "9");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "9");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 0
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '0';
	key.code[UPPER_CASE] = '0';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "0");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "0");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 2-? ???
	row = &m_Rows[LAYOUT_NUM][2];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// @
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '@';
	key.code[UPPER_CASE] = '@';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "@");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "@");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// #
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '#';
	key.code[UPPER_CASE] = '#';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "#");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "#");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// $
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '$';
	key.code[UPPER_CASE] = '$';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "$");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "$");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// %
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '%';
	key.code[UPPER_CASE] = '%';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "%");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "%");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// "
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '"';
	key.code[UPPER_CASE] = '"';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "\"");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "\"");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// *
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '*';
	key.code[UPPER_CASE] = '*';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "*");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "*");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// (
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '(';
	key.code[UPPER_CASE] = '(';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "(");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "(");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// )
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ')';
	key.code[UPPER_CASE] = ')';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], ")");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ")");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// -
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '-';
	key.code[UPPER_CASE] = '-';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "-");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "-");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// _
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '_';
	key.code[UPPER_CASE] = '_';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "_");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "_");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// 3-? ???
	row = &m_Rows[LAYOUT_NUM][3];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// .
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '.';
	key.code[UPPER_CASE] = '.';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], ".");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ".");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// :
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ':';
	key.code[UPPER_CASE] = ':';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], ":");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ":");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// ;
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ';';
	key.code[UPPER_CASE] = ';';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], ";");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ";");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// +
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '+';
	key.code[UPPER_CASE] = '+';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "+");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "+");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// =
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '=';
	key.code[UPPER_CASE] = '=';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "=");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "=");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// <
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '<';
	key.code[UPPER_CASE] = '<';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "<");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "<");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// >
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '>';
	key.code[UPPER_CASE] = '>';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], ">");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], ">");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// [
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '[';
	key.code[UPPER_CASE] = '[';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "[");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "[");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// ]
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ']';
	key.code[UPPER_CASE] = ']';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "]");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "]");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// delete
	key.pos = curPos;
	key.symPos = ImVec2(0,0);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_BACKSPACE;
	key.id++;

	row->push_back(key);

	// 4-? ??????
	row = &m_Rows[LAYOUT_NUM][4];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	key.type = KEY_DEFAULT;

	// slash (/)
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '/';
	key.code[UPPER_CASE] = '/';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "/");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "/");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// switch numb
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_NUM;
	key.id++;
	row->push_back(key);
	curPos.x += key.width + defWidth;

	// switch language
	/*key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SWITCH;
	key.id++;
	row->push_back(key);
	curPos.x += key.width;*/

	// Space
	key.pos = curPos;
	key.symPos = ImVec2(0,0);
	key.width = defWidth * 4;
	key.code[LOWER_CASE] = ' ';
	key.code[UPPER_CASE] = ' ';
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SPACE;
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	// ?
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '?';
	key.code[UPPER_CASE] = '?';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "?");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "?");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// !
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '!';
	key.code[UPPER_CASE] = '!';
	iso_8859_11_to_utf8(key.name[LOWER_CASE], "!");
	iso_8859_11_to_utf8(key.name[UPPER_CASE], "!");
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	// Send
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SEND;
	key.id++;

	row->push_back(key);
	curPos.x += key.width;

	return;
}