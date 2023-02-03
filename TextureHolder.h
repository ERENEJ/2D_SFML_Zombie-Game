#pragma once
#ifndef TEXTURE_HOLDER_H
#define TEXTURE_HOLDER_H

#include <SFML/Graphics.hpp>
#include<map>

using namespace sf;
using namespace std;

class TextureHolder
{
private:
	//member texture map  
	std::map<std::string, Texture> m_TextureMap;

	static TextureHolder* m_TextureHolder;

public:
	
	TextureHolder();
	static Texture& GetTexture(std::string const& filename);
};



#endif // !TEXTURE_HOLDER_H
