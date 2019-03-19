#include "FontManager.h"
#include "Application.h"
#include "Applog.h"

#include "MathGeoLib/Math/float3.h"
#include <experimental/filesystem>

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */ 
//#include "ModuleFileSystem.h"




FontManager::FontManager()
{
	library = FT_Library();
	FT_Init_FreeType(&library);
	//LoadAllFolderFonts();
	
}


FontManager::~FontManager()
{
	for (std::vector<Font*>::reverse_iterator it = loadedFonts.rbegin(); it != loadedFonts.rend(); it++)
	{
		(*it)->CleanUp();
		delete (*it);
		(*it) = nullptr;
	}
	loadedFonts.clear();
}



Font::Font(const char * name)
{
	if(name)
		fontSrc = name;
}

void Font::GenerateCharsList()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

	float3 cursor = { 0,0,0 };

	
	for (GLubyte c = 32; c < 128; c++)
	{
		Character* nwChar = new Character();
		
		
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			app_log->AddLog("ERROR::FREETYTPE: Failed to load Glyph");
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		glGenTextures(1, &nwChar->textureID);		
		glBindTexture(GL_TEXTURE_2D, nwChar->textureID);
		glTexImage2D(GL_TEXTURE_2D,0, GL_ALPHA,face->glyph->bitmap.width,face->glyph->bitmap.rows,0, GL_ALPHA,GL_UNSIGNED_BYTE,face->glyph->bitmap.buffer);
		
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			app_log->AddLog("Error initializing OpenGL: %s!\n",gluErrorString(error));
		}
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		
		nwChar->bearing = { (float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top };
		nwChar->size = { (float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows };
		nwChar->advance = face->glyph->advance.x / 64.0f;

		charsList.insert(std::pair<char, Character*>(c, nwChar));		
	}
	
}
uint Font::GetCharacterTexture(GLchar character)
{
	for(std::map<GLchar, Character*>::iterator it = charsList.begin(); it!=charsList.end(); it++)
	{
		if ((*it).first ==(GLchar)character)
		{
			uint id = (*it).second->textureID;
			return id;
		}
	}
}

Character* Font::GetCharacter(GLchar character) 
{
	for (std::map<GLchar, Character*>::iterator it = charsList.begin(); it != charsList.end(); it++)
	{
		if ((*it).first == (GLchar)character)
		{
			return  (*it).second;
		}
	}
	return nullptr;
}

void Font::CleanUp()
{
	for (std::map<GLchar, Character*>::reverse_iterator it = charsList.rbegin(); it != charsList.rend(); it++)
	{		
		glDeleteBuffers(1, &(*it).second->textureID);
		delete (*it).second;
		(*it).second = nullptr;
	}
	charsList.clear();
	FT_Done_Face(face);
}

void FontManager::LoadAllFolderFonts()
{

	using std::experimental::filesystem::recursive_directory_iterator;
	for (auto& it : recursive_directory_iterator(FONTS_FOLDER)) {
		if (it.status().type() == std::experimental::filesystem::v1::file_type::directory) // If the path is a directory, ignore it
			continue;

		resource_deff new_res_deff;		
		std::string fontName = std::string(it.path().generic_string().c_str());
		std::string extension = std::string(it.path().generic_string().c_str());
		App->fs.getExtension(extension);
		App->fs.getFileNameFromPath(fontName);
		fontName += extension;
		LoadFont(fontName.c_str(), DEFAULT_SCALE);
	}

	
}

Font* FontManager::LoadFont(const char* name, uint scale)
{
	Font* font = new Font(name);

	std::string path = font->fontDir + font->fontSrc;
	FT_Error error = FT_New_Face(library, path.c_str(), 0, &font->face);	

	if (!error){		
		FT_Set_Pixel_Sizes(font->face, 0, scale);
		font->GenerateCharsList();
		font->scale = scale;
		loadedFonts.push_back(font);	
		bool add = true;
		for (int i = 0; i < singleFonts.size(); i++) {
			if (font->fontSrc == singleFonts[i]) {
				add = false;
				break;
			}
		}
		if (add) { singleFonts.push_back(font->fontSrc.c_str()); }
	}
	else {		
		font->CleanUp();		
		delete font;
		font = nullptr;
		FT_Done_FreeType(library);
	}
	
	return font;
}

Font* FontManager::GetFont(const char* _name)
{
	for (std::vector<Font*>::iterator it = loadedFonts.begin(); it != loadedFonts.end(); it++)
	{
		if (strcmp((*it)->fontSrc.c_str(), _name)==0)
		{
			return (*it);
		}
	}

	return nullptr;
}

void FontManager::RemoveFont(const char* name)
{
	for (std::vector<Font*>::iterator it = loadedFonts.begin(); it != loadedFonts.end(); it++)
	{
		if (strcmp((*it)->fontSrc.c_str(), name)==0)
		{
			(*it)->CleanUp();
			loadedFonts.erase(it);
			return;
		}
	}
}
